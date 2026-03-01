/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "interop_intrinsic_optimization.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "plugins/ets/compiler/generated/interop_intrinsic_kinds.h"

namespace ark::compiler {

static bool IsForbiddenInst(Inst *inst)
{
    return inst->IsCall() && !static_cast<CallInst *>(inst)->IsInlined();
}

static bool IsScopeStart(Inst *inst)
{
    return inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                      RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE;
}

static bool IsScopeEnd(Inst *inst)
{
    return inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                      RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_DESTROY_LOCAL_SCOPE;
}

static bool IsWrapIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    return GetInteropIntrinsicKind(id) == InteropIntrinsicKind::WRAP;
}

static bool IsUnwrapIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    return GetInteropIntrinsicKind(id) == InteropIntrinsicKind::UNWRAP;
}

static RuntimeInterface::IntrinsicId GetUnwrapIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    switch (id) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_DOUBLE:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_F64;
        case RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_BOOLEAN:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U1;
        case RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_STRING:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_STRING;
        default:
            return RuntimeInterface::IntrinsicId::INVALID;
    }
}

static bool IsConvertIntrinsic(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto id = inst->CastToIntrinsic()->GetIntrinsicId();
    return IsWrapIntrinsicId(id) || IsUnwrapIntrinsicId(id);
}

static bool IsInteropIntrinsic(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto id = inst->CastToIntrinsic()->GetIntrinsicId();
    return GetInteropIntrinsicKind(id) != InteropIntrinsicKind::NONE;
}

static bool CanCreateNewScopeObject(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto kind = GetInteropIntrinsicKind(inst->CastToIntrinsic()->GetIntrinsicId());
    return kind == InteropIntrinsicKind::WRAP || kind == InteropIntrinsicKind::CREATES_LOCAL;
}

InteropIntrinsicOptimization::BlockInfo &InteropIntrinsicOptimization::GetInfo(BasicBlock *block)
{
    return blockInfo_[block->GetId()];
}

void InteropIntrinsicOptimization::MergeScopesInsideBlock(BasicBlock *block)
{
    Inst *lastStart = nullptr;  // Start of the current scope or nullptr if no scope is open
    Inst *lastEnd = nullptr;    // End of the last closed scope
    // Number of object creations in the last closed scope (valid value if we are in the next scope now)
    uint32_t lastObjectCount = 0;
    uint32_t currentObjectCount = 0;  // Number of object creations in the current scope or 0
    for (auto *inst : block->InstsSafe()) {
        if (IsScopeStart(inst)) {
            ASSERT(lastStart == nullptr);
            ASSERT(currentObjectCount == 0);
            hasScopes_ = true;
            lastStart = inst;
        } else if (IsScopeEnd(inst)) {
            ASSERT(lastStart != nullptr);
            if (lastEnd != nullptr && lastObjectCount + currentObjectCount <= scopeObjectLimit_) {
                ASSERT(lastEnd->IsPrecedingInSameBlock(lastStart));
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Remove scope end " << *lastEnd << "\nand scope start "
                                                           << *lastStart << "\nfrom BB " << block->GetId();
                block->RemoveInst(lastEnd);
                block->RemoveInst(lastStart);
                lastObjectCount += currentObjectCount;
                isApplied_ = true;
            } else {
                objectLimitHit_ |= (lastEnd != nullptr);
                lastObjectCount = currentObjectCount;
            }
            currentObjectCount = 0;

            lastEnd = inst;
            lastStart = nullptr;
        } else if (IsForbiddenInst(inst)) {
            ASSERT(lastStart == nullptr);
            lastEnd = nullptr;
            lastObjectCount = 0;
            currentObjectCount = 0;
        } else if (CanCreateNewScopeObject(inst)) {
            ASSERT(lastStart != nullptr);
            ++currentObjectCount;
        }
    }
}

bool InteropIntrinsicOptimization::TryCreateSingleScope(BasicBlock *bb)
{
    bool isStart = bb == GetGraph()->GetStartBlock()->GetSuccessor(0);
    bool hasStart = false;
    auto lastInst = bb->GetLastInst();
    // We do not need to close scope in compiler before deoptimize or throw inst
    bool isEnd = lastInst != nullptr && lastInst->IsReturn();
    SaveStateInst *ss = nullptr;
    Inst *lastEnd = nullptr;
    for (auto *inst : bb->InstsSafe()) {
        if (isStart && !hasStart && IsScopeStart(inst)) {
            hasStart = true;
        } else if (isEnd && IsScopeEnd(inst)) {
            if (lastEnd != nullptr) {
                bb->RemoveInst(lastEnd);
            }
            lastEnd = inst;
        } else if (IsScopeStart(inst) || IsScopeEnd(inst)) {
            bb->RemoveInst(inst);
        } else if (isEnd && inst->GetOpcode() == Opcode::SaveState && ss == nullptr) {
            ss = inst->CastToSaveState();
        }
    }
    if (!isEnd || lastEnd != nullptr) {
        return hasStart;
    }
    if (ss == nullptr) {
        ss = GetGraph()->CreateInstSaveState(DataType::NO_TYPE, lastInst->GetPc());
        lastInst->InsertBefore(ss);
        if (lastInst->GetOpcode() == Opcode::Return && lastInst->GetInput(0).GetInst()->IsMovableObject()) {
            ss->AppendBridge(lastInst->GetInput(0).GetInst());
        }
    }
    auto scopeEnd = GetGraph()->CreateInstIntrinsic(
        DataType::VOID, ss->GetPc(), RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_DESTROY_LOCAL_SCOPE);
    scopeEnd->SetInputs(GetGraph()->GetAllocator(), {{ss, DataType::NO_TYPE}});
    ss->InsertAfter(scopeEnd);
    return hasStart;
}

bool InteropIntrinsicOptimization::TryCreateSingleScope()
{
    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : bb->Insts()) {
            if (trySingleScope_ && IsForbiddenInst(inst)) {
                trySingleScope_ = false;
            }
            if (IsScopeStart(inst)) {
                hasScopes_ = true;
            }
        }
    }
    if (!hasScopes_ || !trySingleScope_) {
        return false;
    }
    bool hasStart = false;
    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        hasStart |= TryCreateSingleScope(bb);
    }
    if (hasStart) {
        return true;
    }
    auto *startBlock = GetGraph()->GetStartBlock()->GetSuccessor(0);
    SaveStateInst *ss = nullptr;
    for (auto *inst : startBlock->InstsReverse()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            ss = inst->CastToSaveState();
            break;
        }
    }
    if (ss == nullptr) {
        ss = GetGraph()->CreateInstSaveState(DataType::NO_TYPE, startBlock->GetFirstInst()->GetPc());
        startBlock->PrependInst(ss);
        for (auto *inst : GetGraph()->GetStartBlock()->Insts()) {
            if (inst->IsMovableObject()) {
                ss->AppendBridge(inst);
            }
        }
    }
    auto scopeStart = GetGraph()->CreateInstIntrinsic(
        DataType::VOID, ss->GetPc(), RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE);
    scopeStart->SetInputs(GetGraph()->GetAllocator(), {{ss, DataType::NO_TYPE}});
    ss->InsertAfter(scopeStart);
    return true;
}

// Returns estimated object creations in loop if it can be moved into scope or std::nullopt otherwise
std::optional<uint64_t> InteropIntrinsicOptimization::FindForbiddenLoops(Loop *loop)
{
    bool forbidden = false;
    uint64_t objects = 0;
    for (auto *innerLoop : loop->GetInnerLoops()) {
        auto innerObjects = FindForbiddenLoops(innerLoop);
        if (innerObjects.has_value()) {
            objects += *innerObjects;
        } else {
            forbidden = true;
        }
    }
    if (forbidden || loop->IsIrreducible()) {
        forbiddenLoops_.insert(loop);
        return std::nullopt;
    }
    for (auto *block : loop->GetBlocks()) {
        for (auto *inst : block->Insts()) {
            if (IsForbiddenInst(inst)) {
                forbiddenLoops_.insert(loop);
                return std::nullopt;
            }
            if (CanCreateNewScopeObject(inst)) {
                ++objects;
            }
        }
    }
    if (objects == 0) {
        return 0;
    }
    if (objects > scopeObjectLimit_) {
        forbiddenLoops_.insert(loop);
        return std::nullopt;
    }
    if (auto loopInfo = CountableLoopParser(*loop).Parse()) {
        auto optIterations = CountableLoopParser::GetLoopIterations(*loopInfo);
        if (optIterations.has_value() &&
            *optIterations <= scopeObjectLimit_ / objects) {  // compare using division to avoid overflow
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Found small countable loop, id = " << loop->GetId() << ", iterations = " << *optIterations;
            return objects * *optIterations;
        }
    }
    forbiddenLoops_.insert(loop);
    return std::nullopt;
}

bool InteropIntrinsicOptimization::IsForbiddenLoopEntry(BasicBlock *block)
{
    return block->GetLoop()->IsIrreducible() || (block->IsLoopHeader() && forbiddenLoops_.count(block->GetLoop()) > 0);
}

template <bool REVERSE>
static auto GetInstsIter(BasicBlock *block)
{
    if constexpr (REVERSE) {
        return block->InstsReverse();
    } else {
        return block->Insts();
    }
}

// Implementation of disjoint set union with path compression
int32_t InteropIntrinsicOptimization::GetParentComponent(int32_t component)
{
    auto &parent = components_[component].parent;
    if (parent != component) {
        parent = GetParentComponent(parent);
    }
    ASSERT(parent != DFS_NOT_VISITED);
    return parent;
}

void InteropIntrinsicOptimization::MergeComponents(int32_t first, int32_t second)
{
    first = GetParentComponent(first);
    second = GetParentComponent(second);
    if (first != second) {
        components_[first].objectCount += components_[second].objectCount;
        components_[second].parent = first;
    }
}

void InteropIntrinsicOptimization::UpdateStatsForMerging(Inst *inst, int32_t otherEndComponent, uint32_t *scopeSwitches,
                                                         uint32_t *objectsInBlockAfterMerge)
{
    if (IsScopeStart(inst) || IsScopeEnd(inst)) {
        ++*scopeSwitches;
    } else if (CanCreateNewScopeObject(inst)) {
        if (*scopeSwitches == 1) {
            ++*objectsInBlockAfterMerge;
        } else if (*scopeSwitches == 0 && otherEndComponent != currentComponent_) {
            // If both ends of the block belong to the same component, count instructions only from
            // the first visited end
            ++components_[currentComponent_].objectCount;
        }
    } else if (IsForbiddenInst(inst)) {
        if (*scopeSwitches == 1) {
            canMerge_ = false;
        } else if (*scopeSwitches == 0) {
            COMPILER_LOG_IF(!components_[currentComponent_].isForbidden, DEBUG, INTEROP_INTRINSIC_OPT)
                << "Connected component " << currentComponent_
                << " cannot be moved into scope because it contains forbidden inst " << *inst;
            components_[currentComponent_].isForbidden = true;
        }
    }
}

template <bool IS_END>
void InteropIntrinsicOptimization::IterateBlockFromBoundary(BasicBlock *block)
{
    auto otherEndComponent = GetInfo(block).blockComponent[static_cast<int32_t>(!IS_END)];
    if (otherEndComponent != currentComponent_) {
        currentComponentBlocks_.push_back(block);
    }
    uint32_t scopeSwitches = 0;
    uint32_t objectsInBlockAfterMerge = 0;
    for (auto *inst : GetInstsIter<IS_END>(block)) {
        UpdateStatsForMerging(inst, otherEndComponent, &scopeSwitches, &objectsInBlockAfterMerge);
    }
    if (scopeSwitches == 0) {
        if (block->IsStartBlock() || block->IsEndBlock()) {
            COMPILER_LOG_IF(!components_[currentComponent_].isForbidden, DEBUG, INTEROP_INTRINSIC_OPT)
                << "Connected component " << currentComponent_ << " cannot be moved into scope because it contains "
                << (block->IsStartBlock() ? "start" : "end") << " block " << block->GetId();
            components_[currentComponent_].isForbidden = true;
        }
        BlockBoundaryDfs<!IS_END>(block);
    } else if (scopeSwitches == 1) {
        // Other end of the block was already moved into scope (otherwise scope switch count in block would be even)
        ASSERT(otherEndComponent != DFS_NOT_VISITED && otherEndComponent != currentComponent_);
        otherEndComponent = GetParentComponent(otherEndComponent);
        if (components_[otherEndComponent].isForbidden) {
            canMerge_ = false;
        } else {
            objectsInScopeAfterMerge_ += GetObjectCountIfUnused(components_[otherEndComponent], currentComponent_);
        }
    } else if (scopeSwitches > 2U || otherEndComponent != currentComponent_) {
        objectsInScopeAfterMerge_ += objectsInBlockAfterMerge;
    }
}

template <bool IS_END>
void InteropIntrinsicOptimization::BlockBoundaryDfs(BasicBlock *block)
{
    auto index = static_cast<int32_t>(IS_END);
    if (GetInfo(block).blockComponent[index] != DFS_NOT_VISITED) {
        return;
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Added " << (IS_END ? "end" : "start ") << " of block "
                                               << block->GetId() << " to connected component " << currentComponent_;
    GetInfo(block).blockComponent[index] = currentComponent_;
    if (!IS_END && IsForbiddenLoopEntry(block)) {
        COMPILER_LOG_IF(!components_[currentComponent_].isForbidden, DEBUG, INTEROP_INTRINSIC_OPT)
            << "Connected component " << currentComponent_
            << " cannot be moved into scope because it contains entry to forbidden loop " << block->GetLoop()->GetId()
            << " via BB " << block->GetId();
        components_[currentComponent_].isForbidden = true;
    }
    IterateBlockFromBoundary<IS_END>(block);

    if constexpr (IS_END) {
        auto &neighbours = block->GetSuccsBlocks();
        for (auto *neighbour : neighbours) {
            BlockBoundaryDfs<!IS_END>(neighbour);
        }
    } else {
        auto &neighbours = block->GetPredsBlocks();
        for (auto *neighbour : neighbours) {
            BlockBoundaryDfs<!IS_END>(neighbour);
        }
    }
}

static bool MoveBlockStartIntoScope(BasicBlock *block)
{
    bool removed = false;
    for (auto *inst : block->InstsSafe()) {
        if (IsScopeStart(inst)) {
            ASSERT(!removed);
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Remove first scope start " << *inst << "\nfrom BB " << block->GetId();
            block->RemoveInst(inst);
            removed = true;
        }
        if (IsScopeEnd(inst)) {
            ASSERT(removed);
            return false;
        }
    }
    return removed;
}

static bool MoveBlockEndIntoScope(BasicBlock *block)
{
    bool removed = false;
    for (auto *inst : block->InstsSafeReverse()) {
        if (IsScopeEnd(inst)) {
            ASSERT(!removed);
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Remove last scope end " << *inst << "\nfrom BB " << block->GetId();
            block->RemoveInst(inst);
            removed = true;
        }
        if (IsScopeStart(inst)) {
            ASSERT(removed);
            return false;
        }
    }
    return removed;
}

void InteropIntrinsicOptimization::MergeCurrentComponentWithNeighbours()
{
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "Merging connected component " << currentComponent_ << " with its neighbours";
    for (auto *block : currentComponentBlocks_) {
        if (GetInfo(block).blockComponent[0] == currentComponent_ && MoveBlockStartIntoScope(block)) {
            MergeComponents(currentComponent_, GetInfo(block).blockComponent[1]);
        }
        if (GetInfo(block).blockComponent[1] == currentComponent_ && MoveBlockEndIntoScope(block)) {
            MergeComponents(currentComponent_, GetInfo(block).blockComponent[0]);
        }
    }
    isApplied_ = true;
}

template <bool IS_END>
void InteropIntrinsicOptimization::FindComponentAndTryMerge(BasicBlock *block)
{
    auto index = static_cast<int32_t>(IS_END);
    if (GetInfo(block).blockComponent[index] != DFS_NOT_VISITED) {
        return;
    }
    components_.push_back({currentComponent_, 0, 0, false});
    currentComponentBlocks_.clear();
    objectsInScopeAfterMerge_ = 0;
    canMerge_ = true;
    BlockBoundaryDfs<IS_END>(block);
    if (components_[currentComponent_].isForbidden) {
        canMerge_ = false;
    }
    if (canMerge_) {
        objectsInScopeAfterMerge_ += components_[currentComponent_].objectCount;
        if (objectsInScopeAfterMerge_ > scopeObjectLimit_) {
            objectLimitHit_ = true;
        } else {
            MergeCurrentComponentWithNeighbours();
        }
    }
    ++currentComponent_;
}

void InteropIntrinsicOptimization::MergeInterScopeRegions()
{
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        FindComponentAndTryMerge<false>(block);
        FindComponentAndTryMerge<true>(block);
    }
}

// Numbering is similar to pre-order, but we stop in blocks with scope starts
void InteropIntrinsicOptimization::DfsNumbering(BasicBlock *block)
{
    if (GetInfo(block).dfsNumIn != DFS_NOT_VISITED) {
        return;
    }
    GetInfo(block).dfsNumIn = dfsNum_++;
    for (auto inst : block->Insts()) {
        ASSERT(!IsScopeEnd(inst));
        if (IsScopeStart(inst)) {
            GetInfo(block).dfsNumOut = dfsNum_;
            return;
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        DfsNumbering(succ);
    }
    GetInfo(block).dfsNumOut = dfsNum_;
}

// Calculate minimal and maximal `dfs_num_in` for blocks that can be reached by walking some edges forward
// and, after that, maybe one edge backward
// Visit order will be the same as in DfsNumbering
// We walk the graph again because during the first DFS some numbers for predecessors may be invalid yet
void InteropIntrinsicOptimization::CalculateReachabilityRec(BasicBlock *block)
{
    if (block->SetMarker(visited_)) {
        return;
    }
    auto &info = GetInfo(block);
    info.subgraphMinNum = info.subgraphMaxNum = info.dfsNumIn;

    bool isScopeStart = false;
    for (auto inst : block->Insts()) {
        ASSERT(!IsScopeEnd(inst));
        if (IsForbiddenInst(inst)) {
            info.subgraphMinNum = DFS_NOT_VISITED;
        }
        if (IsScopeStart(inst)) {
            isScopeStart = true;
            break;
        }
    }

    if (!isScopeStart) {
        for (auto *succ : block->GetSuccsBlocks()) {
            CalculateReachabilityRec(succ);
            info.subgraphMinNum = std::min(info.subgraphMinNum, GetInfo(succ).subgraphMinNum);
            info.subgraphMaxNum = std::max(info.subgraphMaxNum, GetInfo(succ).subgraphMaxNum);
            if (IsForbiddenLoopEntry(succ)) {
                info.subgraphMinNum = DFS_NOT_VISITED;
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << "BB " << block->GetId() << " cannot be moved into scope because succ " << succ->GetId()
                    << " is entry to forbidden loop " << succ->GetLoop()->GetId();
                break;
            }
        }
        if (info.dfsNumIn <= info.subgraphMinNum && info.subgraphMaxNum < info.dfsNumOut) {
            block->SetMarker(canHoistTo_);
        }
    }
    for (auto *pred : block->GetPredsBlocks()) {
        if (pred->IsMarked(startDfs_)) {
            info.subgraphMinNum = DFS_NOT_VISITED;
            break;
        }
        info.subgraphMinNum = std::min(info.subgraphMinNum, GetInfo(pred).dfsNumIn);
        info.subgraphMaxNum = std::max(info.subgraphMaxNum, GetInfo(pred).dfsNumIn);
    }
}

template <void (InteropIntrinsicOptimization::*DFS)(BasicBlock *)>
void InteropIntrinsicOptimization::DoDfs()
{
    // We launch DFS in a graph where vertices correspond to block starts not contained in scopes, and
    // vertices for bb1 and bb2 are connected by a (directed) edge if bb1 and bb2 are connected in CFG and
    // there are no scopes in bb1

    for (auto *block : GetGraph()->GetBlocksRPO()) {
        // We mark block with `start_dfs_` marker if its **end** is contained in a new inter-scope region
        // (i. e. block is the start block or last scope switch in block is scope end)
        // And since our graph contains **starts** of blocks, we launch DFS from succs, not from the block itself
        if (block->IsMarked(startDfs_)) {
            for (auto *succ : block->GetSuccsBlocks()) {
                (this->*DFS)(succ);
            }
        }
    }
}

bool InteropIntrinsicOptimization::CreateScopeStartInBlock(BasicBlock *block)
{
    for (auto *inst : block->InstsSafeReverse()) {
        ASSERT(!IsForbiddenInst(inst) && !IsScopeStart(inst) && !IsScopeEnd(inst));
        if (inst->GetOpcode() == Opcode::SaveState) {
            auto scopeStart = GetGraph()->CreateInstIntrinsic(
                DataType::VOID, inst->GetPc(), RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE);
            scopeStart->SetInputs(GetGraph()->GetAllocator(), {{inst, DataType::NO_TYPE}});
            inst->InsertAfter(scopeStart);
            isApplied_ = true;
            return true;
        }
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Failed to create new scope start in BB " << block->GetId();
    return false;
}

void InteropIntrinsicOptimization::RemoveReachableScopeStarts(BasicBlock *block, BasicBlock *newStartBlock)
{
    ASSERT(!block->IsEndBlock());
    if (block->SetMarker(visited_)) {
        return;
    }
    block->ResetMarker(canHoistTo_);
    ASSERT(newStartBlock->IsDominate(block));
    if (block != newStartBlock) {
        ASSERT(!IsForbiddenLoopEntry(block));
        for (auto *inst : block->Insts()) {
            ASSERT(!IsForbiddenInst(inst) && !IsScopeEnd(inst));
            if (IsScopeStart(inst)) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << "Removed scope start " << *inst << "from BB " << block->GetId() << ", new scope start in "
                    << newStartBlock->GetId();
                block->RemoveInst(inst);
                return;
            }
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        RemoveReachableScopeStarts(succ, newStartBlock);
    }
}

void InteropIntrinsicOptimization::HoistScopeStarts()
{
    auto startDfsHolder = MarkerHolder(GetGraph());
    startDfs_ = startDfsHolder.GetMarker();
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        bool endInScope = false;
        for (auto inst : block->InstsReverse()) {
            if (IsScopeEnd(inst)) {
                block->SetMarker(startDfs_);
                break;
            }
            if (IsScopeStart(inst)) {
                endInScope = true;
                break;
            }
        }
        if (block->IsStartBlock() && !endInScope) {
            block->SetMarker(startDfs_);
        }
    }
    DoDfs<&InteropIntrinsicOptimization::DfsNumbering>();
    auto canHoistToHolder = MarkerHolder(GetGraph());
    canHoistTo_ = canHoistToHolder.GetMarker();
    {
        auto visitedHolder = MarkerHolder(GetGraph());
        visited_ = visitedHolder.GetMarker();
        DoDfs<&InteropIntrinsicOptimization::CalculateReachabilityRec>();
    }

    for (auto *block : GetGraph()->GetBlocksRPO()) {
        if (block->IsMarked(canHoistTo_) && CreateScopeStartInBlock(block)) {
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Hoisted scope start to BB " << block->GetId() << ", removing scope starts reachable from it";
            auto visitedHolder = MarkerHolder(GetGraph());
            visited_ = visitedHolder.GetMarker();
            RemoveReachableScopeStarts(block, block);
        }
    }
}

void InteropIntrinsicOptimization::InvalidateScopesInSubgraph(BasicBlock *block)
{
    ASSERT(!block->IsEndBlock());
    if (block->SetMarker(scopeStartInvalidated_)) {
        return;
    }
    for (auto *inst : block->Insts()) {
        ASSERT(!IsScopeStart(inst));
        if (IsScopeEnd(inst)) {
            return;
        }
        if (IsInteropIntrinsic(inst)) {
            scopeForInst_[inst] = nullptr;
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        InvalidateScopesInSubgraph(succ);
    }
}

void InteropIntrinsicOptimization::CheckGraphRec(BasicBlock *block, Inst *scopeStart)
{
    if (block->SetMarker(visited_)) {
        if (GetInfo(block).lastScopeStart != scopeStart) {
            // It's impossible to have scope start in one path to block and to have no scope in another path
            ASSERT(GetInfo(block).lastScopeStart != nullptr);
            ASSERT(scopeStart != nullptr);
            // Different scope starts for different execution paths are possible
            // NOTE(aefremov): find insts with always equal scopes somehow
            InvalidateScopesInSubgraph(block);
        }
        return;
    }
    GetInfo(block).lastScopeStart = scopeStart;
    for (auto *inst : block->Insts()) {
        if (IsScopeEnd(inst)) {
            ASSERT(scopeStart != nullptr);
            scopeStart = nullptr;
        } else if (IsScopeStart(inst)) {
            ASSERT(scopeStart == nullptr);
            scopeStart = inst;
        } else if (IsForbiddenInst(inst)) {
            ASSERT(scopeStart == nullptr);
        } else if (IsInteropIntrinsic(inst)) {
            ASSERT(scopeStart != nullptr);
            scopeForInst_[inst] = scopeStart;
        }
    }
    if (block->IsEndBlock()) {
        ASSERT(scopeStart == nullptr);
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        CheckGraphRec(succ, scopeStart);
    }
}

void InteropIntrinsicOptimization::CheckGraphAndFindScopes()
{
    auto visitedHolder = MarkerHolder(GetGraph());
    visited_ = visitedHolder.GetMarker();
    auto invalidatedHolder = MarkerHolder(GetGraph());
    scopeStartInvalidated_ = invalidatedHolder.GetMarker();
    CheckGraphRec(GetGraph()->GetStartBlock(), nullptr);
}

void InteropIntrinsicOptimization::MarkRequireRegMap(Inst *inst)
{
    SaveStateInst *saveState = nullptr;
    if (inst->IsSaveState()) {
        saveState = static_cast<SaveStateInst *>(inst);
    } else if (inst->RequireState()) {
        saveState = inst->GetSaveState();
    }
    while (saveState != nullptr && saveState->SetMarker(requireRegMap_) && saveState->GetCallerInst() != nullptr) {
        saveState = saveState->GetCallerInst()->GetSaveState();
    }
}

void InteropIntrinsicOptimization::TryRemoveUnwrapAndWrap(Inst *inst, Inst *input)
{
    if (scopeForInst_.at(inst) == nullptr || scopeForInst_.at(inst) != scopeForInst_.at(input)) {
        COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
            << "Scopes don't match, skip: wrap " << *inst << "\nwith unwrap input " << *input;
        return;
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Remove wrap " << *inst << "\nwith unwrap input " << *input;
    auto *newInput = input->GetInput(0).GetInst();
    // We don't extend scopes out of basic blocks in OSR mode
    ASSERT(!GetGraph()->IsOsrMode() || inst->GetBasicBlock() == newInput->GetBasicBlock());
    ASSERT(newInput->GetType() == DataType::POINTER);
    ASSERT(inst->GetType() == DataType::POINTER);
    inst->ReplaceUsers(newInput);
    inst->GetBasicBlock()->RemoveInst(inst);
    // If `input` (unwrap from local to JSValue or ets primitve) has SaveState users which require regmap,
    // we will not delete the unwrap intrinsic
    // NOTE(aefremov): support unwrap (local => JSValue/primitive) during deoptimization
    isApplied_ = true;
}

void InteropIntrinsicOptimization::TryRemoveUnwrapToJSValue(Inst *inst)
{
    auto commonId = RuntimeInterface::IntrinsicId::INVALID;
    DataType::Type userType = DataType::Type::NO_TYPE;
    for (auto &user : inst->GetUsers()) {
        auto *userInst = user.GetInst();
        if (userInst->IsSaveState()) {
            // see the previous note
            if (userInst->IsMarked(requireRegMap_)) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << "User " << *userInst << "\nof inst " << *inst << " requires regmap, skip";
                return;
            }
            continue;
        }
        if (!userInst->IsIntrinsic()) {
            return;
        }
        if (HasOsrEntryBetween(inst, userInst)) {
            return;
        }
        auto currentId = userInst->CastToIntrinsic()->GetIntrinsicId();
        if (commonId == RuntimeInterface::IntrinsicId::INVALID) {
            userType = userInst->GetType();
            commonId = currentId;
        } else if (currentId != commonId) {
            return;
        }
    }
    auto newId = GetUnwrapIntrinsicId(commonId);
    if (newId == RuntimeInterface::IntrinsicId::INVALID) {
        // includes the case when common_id is INVALID
        return;
    }
    inst->CastToIntrinsic()->SetIntrinsicId(newId);
    inst->SetType(userType);
    for (auto userIt = inst->GetUsers().begin(); userIt != inst->GetUsers().end();) {
        auto userInst = userIt->GetInst();
        if (userInst->IsIntrinsic() && userInst->CastToIntrinsic()->GetIntrinsicId() == commonId) {
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Replace cast from JSValue " << *userInst << "\nwith direct unwrap " << *inst;
            userInst->ReplaceUsers(inst);
            userInst->GetBasicBlock()->RemoveInst(userInst);
            userIt = inst->GetUsers().begin();
        } else {
            ++userIt;
        }
    }
    isApplied_ = true;
}

void InteropIntrinsicOptimization::TryRemoveIntrinsic(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return;
    }
    auto *input = inst->GetInput(0).GetInst();
    auto intrinsicId = inst->CastToIntrinsic()->GetIntrinsicId();
    if (input->IsIntrinsic() && IsWrapIntrinsicId(intrinsicId) &&
        IsUnwrapIntrinsicId(input->CastToIntrinsic()->GetIntrinsicId())) {
        TryRemoveUnwrapAndWrap(inst, input);
    } else if (intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_JS_VALUE) {
        TryRemoveUnwrapToJSValue(inst);
    } else if (intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_CALL_FUNCTION && !inst->HasUsers()) {
        // avoid creation of handle for return value in local scope if it is unused
        inst->SetType(DataType::VOID);
        inst->CastToIntrinsic()->SetIntrinsicId(
            RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_CALL_VOID_FUNCTION);
        isApplied_ = true;
    }
}

void InteropIntrinsicOptimization::EliminateCastPairs()
{
    auto requireRegMapHolder = MarkerHolder(GetGraph());
    requireRegMap_ = requireRegMapHolder.GetMarker();
    auto &blocksRpo = GetGraph()->GetBlocksRPO();
    for (auto it = blocksRpo.rbegin(); it != blocksRpo.rend(); ++it) {
        auto *block = *it;
        for (auto inst : block->InstsSafeReverse()) {
            if (inst->RequireRegMap()) {
                MarkRequireRegMap(inst);
            }
            TryRemoveIntrinsic(inst);
        }
    }
}

void InteropIntrinsicOptimization::DomTreeDfs(BasicBlock *block)
{
    // bb1->IsDominate(bb2) iff bb1.dom_tree_in <= bb2.dom_tree_in < bb1.dom_tree_out
    GetInfo(block).domTreeIn = domTreeNum_++;
    for (auto *dom : block->GetDominatedBlocks()) {
        DomTreeDfs(dom);
    }
    GetInfo(block).domTreeOut = domTreeNum_;
}

void InteropIntrinsicOptimization::MarkPartiallyAnticipated(BasicBlock *block, BasicBlock *stopBlock)
{
    if (block->SetMarker(instAnticipated_)) {
        return;
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "Mark block " << block->GetId() << " where current inst is partially anticipated";
    GetInfo(block).subgraphMinNum = DFS_NOT_VISITED;
    GetInfo(block).maxChain = 0;
    GetInfo(block).maxDepth = -1L;
    if (block == stopBlock) {
        return;
    }
    ASSERT(!block->IsStartBlock());
    for (auto *pred : block->GetPredsBlocks()) {
        MarkPartiallyAnticipated(pred, stopBlock);
    }
}

void InteropIntrinsicOptimization::CalculateDownSafe(BasicBlock *block)
{
    auto &info = GetInfo(block);
    if (!block->IsMarked(instAnticipated_)) {
        info.maxChain = 0;
        info.maxDepth = -1L;
        info.subgraphMinNum = DFS_NOT_VISITED;
        return;
    }
    if (info.subgraphMinNum != DFS_NOT_VISITED) {
        return;
    }
    ASSERT(info.domTreeIn >= 0);
    info.subgraphMinNum = info.subgraphMaxNum = info.domTreeIn;
    int32_t succMaxChain = 0;
    for (auto *succ : block->GetSuccsBlocks()) {
        CalculateDownSafe(succ);
        succMaxChain = std::max(succMaxChain, GetInfo(succ).maxChain);
        if (!block->IsMarked(eliminationCandidate_)) {
            info.subgraphMinNum = std::min(info.subgraphMinNum, GetInfo(succ).subgraphMinNum);
            info.subgraphMaxNum = std::max(info.subgraphMaxNum, GetInfo(succ).subgraphMaxNum);
        }
    }
    for (auto *dom : block->GetSuccsBlocks()) {
        if (dom->IsMarked(instAnticipated_)) {
            info.maxDepth = std::max(info.maxDepth, GetInfo(dom).maxDepth);
        }
    }
    info.maxChain += succMaxChain;
}

void InteropIntrinsicOptimization::ReplaceInst(Inst *inst, Inst **newInst, Inst *insertAfter)
{
    ASSERT(inst != nullptr);
    ASSERT(*newInst != nullptr);
    ASSERT((*newInst)->IsDominate(inst));
    if ((*newInst)->GetOpcode() == Opcode::SaveState) {
        ASSERT(insertAfter != nullptr);
        auto oldNextInst = inst->GetNext();
        auto *block = inst->GetBasicBlock();
        block->EraseInst(inst);
        insertAfter->InsertAfter(inst);
        inst->SetSaveState(*newInst);
        *newInst = inst;
        if (inst->IsReferenceOrAny()) {
            // SSB is needed for conversion from local to JSValue or other ref type
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), *newInst, oldNextInst, nullptr, block);
        }
    } else {
        ASSERT(inst->GetOpcode() == (*newInst)->GetOpcode());
        inst->ReplaceUsers(*newInst);
        if (inst->IsReferenceOrAny()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), *newInst, inst);
        }
    }
}

static bool HasSaveState(BasicBlock *block)
{
    for (auto inst : block->Insts()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            return true;
        }
    }
    return false;
}

bool InteropIntrinsicOptimization::CanHoistTo(BasicBlock *block)
{
    ASSERT(!block->IsMarked(eliminationCandidate_));
    auto &info = GetInfo(block);
    bool dominatesSubgraph = info.domTreeIn <= info.subgraphMinNum && info.subgraphMaxNum < info.domTreeOut;
    auto depth = block->GetLoop()->GetDepth();
    // Heuristics to estimate if hoisting to this blocks is profitable, and it is not better to hoist to some dominated
    // blocks instead
    if (dominatesSubgraph && info.maxChain > 1) {
        for (auto *dom : block->GetDominatedBlocks()) {
            auto domDepth = dom->GetLoop()->GetDepth();
            if (GetInfo(dom).maxChain > 0 &&
                (GetInfo(dom).maxChain < info.maxChain || domDepth > depth || !HasSaveState(dom))) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << " Block " << block->GetId() << " is candidate for hoisting";
                return true;
            }
        }
    }
    if (info.maxDepth > static_cast<int32_t>(depth)) {
        for (auto *dom : block->GetDominatedBlocks()) {
            auto domDepth = dom->GetLoop()->GetDepth();
            if (GetInfo(dom).maxDepth >= 0 && (domDepth > depth || !HasSaveState(dom))) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << " Block " << block->GetId() << " is candidate for hoisting";
                return true;
            }
        }
    }
    return false;
}

// DFS blocks dominated by start block, and save blocks from its dominance frontier to process them later in
// HoistAndEliminate. If *new_inst is SaveState, we move the first dominated inst we want to replace after insert_after
// and set *new_inst to it. Otherwise just replace dominated inst with *new_inst
void InteropIntrinsicOptimization::HoistAndEliminateRec(BasicBlock *block, const BlockInfo &startInfo, Inst **newInst,
                                                        Inst *insertAfter)
{
    if (block->ResetMarker(eliminationCandidate_)) {
        for (auto *inst : block->InstsSafe()) {
            if (inst->IsMarked(eliminationCandidate_) && inst != *newInst) {
                ReplaceInst(inst, newInst, insertAfter);
                isApplied_ = true;
            }
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        if (!succ->ResetMarker(instAnticipated_)) {
            continue;
        }
        // Fast IsDominate check
        if (startInfo.domTreeIn <= GetInfo(succ).domTreeIn && GetInfo(succ).domTreeIn < startInfo.domTreeOut) {
            HoistAndEliminateRec(succ, startInfo, newInst, insertAfter);
        } else {
            blocksToProcess_.push_back(succ);
        }
    }
}

// Returns SaveState and inst to insert after
static std::pair<Inst *, Inst *> GetHoistPosition(BasicBlock *block, Inst *boundaryInst)
{
    for (auto inst : block->InstsReverse()) {
        if (inst == boundaryInst) {
            auto prev = inst->GetPrev();
            if (prev != nullptr && prev->GetOpcode() == Opcode::SaveState && !inst->IsMovableObject()) {
                return std::make_pair(prev, inst);
            }
            return std::make_pair(nullptr, nullptr);
        }
        if (inst->GetOpcode() == Opcode::SaveState) {
            return std::make_pair(inst, inst);
        }
    }
    return std::make_pair(nullptr, nullptr);
}

Inst *InteropIntrinsicOptimization::FindEliminationCandidate(BasicBlock *block)
{
    for (auto inst : block->Insts()) {
        if (inst->IsMarked(eliminationCandidate_)) {
            return inst;
        }
    }
    UNREACHABLE();
}

void InteropIntrinsicOptimization::HoistAndEliminate(BasicBlock *startBlock, Inst *boundaryInst)
{
    ASSERT(boundaryInst->GetBasicBlock() == startBlock);
    blocksToProcess_.clear();
    blocksToProcess_.push_back(startBlock);
    ASSERT(startBlock->ResetMarker(instAnticipated_));
    while (!blocksToProcess_.empty()) {
        auto *block = blocksToProcess_.back();
        blocksToProcess_.pop_back();
        // We reset inst_anticipated_ marker while we traverse the subgraph where inst is anticipated
        ASSERT(!block->IsMarked(instAnticipated_));
        auto &info = GetInfo(block);
        ASSERT(info.subgraphMinNum != DFS_INVALIDATED);
        Inst *newInst = nullptr;
        Inst *insertAfter = nullptr;
        if (block->IsMarked(eliminationCandidate_)) {
            newInst = FindEliminationCandidate(block);
        } else if (CanHoistTo(block)) {
            std::tie(newInst, insertAfter) = GetHoistPosition(block, boundaryInst);
            if (newInst == nullptr) {
                info.subgraphMinNum = DFS_INVALIDATED;
                continue;
            }
        }
        info.subgraphMinNum = DFS_INVALIDATED;
        if (newInst != nullptr) {
            HoistAndEliminateRec(block, GetInfo(block), &newInst, insertAfter);
            continue;
        }
        for (auto *succ : block->GetSuccsBlocks()) {
            if (succ->ResetMarker(instAnticipated_)) {
                blocksToProcess_.push_back(succ);
            }
        }
    }
}

void InteropIntrinsicOptimization::DoRedundancyElimination(Inst *scopeStart, InstVector &insts)
{
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "Process group of intrinsics with identical inputs and scope start: " << *scopeStart;
    ASSERT(!insts.empty());
#ifndef NDEBUG
    for (auto *inst : insts) {
        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            auto inputInst = inst->GetInput(i).GetInst();
            ASSERT(inputInst->IsSaveState() || inputInst == insts[0]->GetInput(i).GetInst());
        }
    }
#endif
    auto *boundaryInst = scopeStart;
    // find highest common dominated inst of `scopeStart` and `insts` inputs
    // i. e. lowest (latest) inst among them
    for (auto &input : insts.front()->GetInputs()) {
        auto *inputInst = input.GetInst();
        if (!inputInst->IsSaveState() && boundaryInst->IsDominate(inputInst)) {
            boundaryInst = inputInst;
        }
    }
    ASSERT(scopeStart->IsDominate(boundaryInst));
    auto *boundary = boundaryInst->GetBasicBlock();
    auto instAnticipatedHolder = MarkerHolder(GetGraph());
    instAnticipated_ = instAnticipatedHolder.GetMarker();
    auto eliminationCandidateHolder = MarkerHolder(GetGraph());
    eliminationCandidate_ = eliminationCandidateHolder.GetMarker();
    // Marking blocks where inst is partially anticipated is needed only to reduce number of processed blocks
    for (auto *inst : insts) {
        COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << " Inst in this group: " << *inst;
        inst->SetMarker(eliminationCandidate_);
        auto *block = inst->GetBasicBlock();
        block->SetMarker(eliminationCandidate_);
        MarkPartiallyAnticipated(block, boundary);
        GetInfo(block).maxChain++;
        GetInfo(block).maxDepth = static_cast<int32_t>(block->GetLoop()->GetDepth());
    }
    CalculateDownSafe(boundary);
    HoistAndEliminate(boundary, boundaryInst);
}

void InteropIntrinsicOptimization::SaveSiblingForElimination(Inst *sibling, ArenaMap<Inst *, InstVector> &currentInsts,
                                                             RuntimeInterface::IntrinsicId id, Marker processed)
{
    if (!sibling->IsIntrinsic() || sibling->CastToIntrinsic()->GetIntrinsicId() != id) {
        return;
    }
    sibling->SetMarker(processed);
    auto scopeStart = scopeForInst_.at(sibling);
    if (scopeStart != nullptr) {
        auto it = currentInsts.try_emplace(scopeStart, GetGraph()->GetLocalAllocator()->Adapter()).first;
        it->second.push_back(sibling);
    }
}

void InteropIntrinsicOptimization::RedundancyElimination()
{
    DomTreeDfs(GetGraph()->GetStartBlock());
    auto processedHolder = MarkerHolder(GetGraph());
    auto processed = processedHolder.GetMarker();
    ArenaMap<Inst *, InstVector> currentInsts(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : block->InstsSafe()) {
            if (inst->IsMarked(processed) || !IsConvertIntrinsic(inst)) {
                continue;
            }
            auto id = inst->CastToIntrinsic()->GetIntrinsicId();
            currentInsts.clear();
            auto *input = inst->GetInput(0).GetInst();
            for (auto &user : input->GetUsers()) {
                SaveSiblingForElimination(user.GetInst(), currentInsts, id, processed);
            }
            for (auto &[scope, insts] : currentInsts) {
                DoRedundancyElimination(scope, insts);
            }
        }
    }
    {
        currentInsts.clear();
        auto id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_LOAD_JS_CONSTANT_POOL;
        for (auto *block : GetGraph()->GetBlocksRPO()) {
            for (auto *inst : block->Insts()) {
                if (inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() == id) {
                    SaveSiblingForElimination(inst, currentInsts, id, processed);
                }
            }
        }
        for (auto &[scope, insts] : currentInsts) {
            DoRedundancyElimination(scope, insts);
        }
    }
}

bool InteropIntrinsicOptimization::RunImpl()
{
    ASSERT(!GetGraph()->IsBytecodeOptimizer());
    bool oneScope = TryCreateSingleScope();
    if (!hasScopes_) {
        return false;
    }
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<DominatorsTree>();
    if (!oneScope) {
        COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase I: merge scopes inside basic blocks";
        for (auto *block : GetGraph()->GetBlocksRPO()) {
            MergeScopesInsideBlock(block);
        }
        if (!GetGraph()->IsOsrMode()) {
            for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
                FindForbiddenLoops(loop);
            }
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "  Phase II: remove inter-scope regions to merge neighbouring scopes";
            MergeInterScopeRegions();
            if (!objectLimitHit_) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase III: hoist scope starts";
                HoistScopeStarts();
            }
        }
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "  Phase IV: Check graph and find scope starts for interop intrinsics";
    // NB: we assume that each scope is residing inside one block before the pass, and that there are no nested scopes
    CheckGraphAndFindScopes();
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase V: Peepholes for interop intrinsics";
    EliminateCastPairs();
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase VI: Redundancy elimination for wrap intrinsics";
    RedundancyElimination();
    return IsApplied();
}

}  // namespace ark::compiler
