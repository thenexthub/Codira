/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "simplify_string_builder.h"

#include "compiler_logger.h"

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"

#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/string_builder_utils.h"

namespace ark::compiler {
using MethodPtr = RuntimeInterface::MethodPtr;

SimplifyStringBuilder::SimplifyStringBuilder(Graph *graph)
    : Optimization(graph),
      instructionsStack_ {graph->GetLocalAllocator()->Adapter()},
      instructionsVector_ {graph->GetLocalAllocator()->Adapter()},
      inputDescriptors_ {graph->GetLocalAllocator()->Adapter()},
      usages_ {graph->GetLocalAllocator()->Adapter()},
      matches_ {graph->GetLocalAllocator()->Adapter()},
      stringBuilderCalls_ {graph->GetLocalAllocator()->Adapter()},
      stringBuilderFirstLastCalls_ {graph->GetLocalAllocator()->Adapter()},
      sbStringLengthCalls_ {graph->GetLocalAllocator()->Adapter()}
{
}

bool HasTryCatchBlocks(Graph *graph)
{
    for (auto block : graph->GetBlocksRPO()) {
        if (block->IsTryCatch()) {
            return true;
        }
    }
    return false;
}

// CC-OFFNXT(huge_method[C++]) solid logic
bool SimplifyStringBuilder::RunImpl()
{
    isApplied_ = false;

    if (!GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        GetGraph()->RunPass<DominatorsTree>();
    }

    ASSERT(GetGraph()->GetRootLoop() != nullptr);

    // NOTE: Disable this pass if there is a NativeApi call in the graph. That is because this pass can delete
    // StringBuilder instances, which will lead to incorrect state in case Native call deoptimizes (i.e. unsupported API
    // version). Remove this workaround after proper fix!
    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : bb->Insts()) {
            if (inst->GetOpcode() != Opcode::CallStatic) {
                continue;
            }

            CallInst *callInst = inst->CastToCallStatic();
            if (callInst->GetIsNative() && !GetGraph()->GetRuntime()->IsMethodIntrinsic(callInst->GetCallMethod())) {
                return false;
            }
        }
    }

    // Loops with try-catch block and OSR mode are not supported in current implementation
    if (!HasTryCatchBlocks(GetGraph()) && !GetGraph()->IsOsrMode()) {
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            OptimizeStringConcatenation(loop);
        }
    }

    // Cleanup should be done before block optimizations, to erase instruction marked as dead
    GetGraph()->RunPass<compiler::Cleanup>();

    OptimizeStringBuilderChain();

    // Cleanup should be done before block optimizations, to erase instruction marked as dead
    GetGraph()->RunPass<compiler::Cleanup>();

    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsEmpty()) {
            continue;
        }
        OptimizeStringBuilderToString(block);
        OptimizeStringConcatenation(block);
        OptimizeStringBuilderAppendChain(block);
        OptimizeStringBuilderStringLength(block);
    }

    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Simplify StringBuilder complete";

    if (GetGraph()->IsBytecodeOptimizer()) {
        // Cleanup should be done inside pass, to satisfy GraphChecker
        GetGraph()->RunPass<compiler::Cleanup>();
        return isApplied_;
    }

    /// NOTE: (mivanov) The loop below will be removed after #30940 is implemented
    // Remove save state inserted in loop post exit block at IR builder
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->GetOpcode() != Opcode::SaveState) {
                continue;
            }
            inst->ClearFlag(inst_flags::NO_DCE);
        }
    }

    // Cleanup should be done inside pass, to satisfy GraphChecker
    GetGraph()->RunPass<compiler::Cleanup>();

    return isApplied_;
}

void SimplifyStringBuilder::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

InstIter SimplifyStringBuilder::SkipToStringBuilderConstructorWithStringArg(InstIter begin, InstIter end)
{
    return std::find_if(std::move(begin), std::move(end),
                        [](auto inst) { return IsMethodStringBuilderConstructorWithStringArg(inst); });
}

bool IsDataFlowInput(Inst *inst, Inst *input)
{
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        if (inst->GetDataFlowInput(i) == input) {
            return true;
        }
    }
    return false;
}

static bool CanRemoveFromSaveStates(Inst *inst)
{
    auto mayRequireRegMap = [instance = inst](const Inst *ssUser) {
        // For now, assume no throws or deoptimizations occur in our methods
        return SaveStateInst::InstMayRequireRegMap(ssUser) && !IsStringBuilderMethod(ssUser, instance) &&
               // CC-OFFNXT(G.FMT.02-CPP) project code style
               !IsIntrinsicStringConcat(ssUser) && !IsNullCheck(ssUser, instance) &&
               // CC-OFFNXT(G.FMT.02-CPP) project code style
               ssUser->GetOpcode() != Opcode::LoadString;
    };
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsSaveState() && !static_cast<SaveStateInst *>(userInst)->CanRemoveInputs(mayRequireRegMap)) {
            return false;
        }
    }
    return true;
}

void SimplifyStringBuilder::OptimizeStringBuilderToString(BasicBlock *block)
{
    // Removes unnecessary String Builder instances

    ASSERT(block != nullptr);
    ASSERT(block->GetGraph() == GetGraph());

    // Walk through a basic block, find every StringBuilder instance and constructor call,
    // and check it we can remove/replace them
    InstIter inst = block->Insts().begin();
    while ((inst = SkipToStringBuilderConstructorWithStringArg(inst, block->Insts().end())) != block->Insts().end()) {
        ASSERT((*inst)->IsStaticCall());
        auto ctorCall = (*inst)->CastToCallStatic();

        // void StringBuilder::<ctor> instance, arg, save_state
        ASSERT(ctorCall->GetInputsCount() == ARGS_NUM_3);
        auto instance = ctorCall->GetInput(0).GetInst();
        auto arg = ctorCall->GetInput(1).GetInst();

        // Look for StringBuilder usages within current basic block
        auto nextInst = block->Insts().end();
        bool removeInstance = true;
        for (++inst; inst != block->Insts().end(); ++inst) {
            // Skip SaveState instructions
            if ((*inst)->IsSaveState()) {
                continue;
            }

            // Skip check instructions, like NullCheck, RefTypeCheck, etc.
            if ((*inst)->IsCheck()) {
                continue;
            }

            // Continue (outer loop) with the next StringBuilder constructor,
            // in case we met one in inner loop
            if (IsMethodStringBuilderConstructorWithStringArg(*inst)) {
                nextInst = nextInst != block->Insts().end() ? nextInst : inst;
            }

            if (!IsDataFlowInput(*inst, instance)) {
                continue;
            }

            // Process usages of StringBuilder instance:
            // replace toString()-calls until we met something else
            if (IsStringBuilderToString(*inst)) {
                (*inst)->ReplaceUsers(arg);
                (*inst)->ClearFlag(compiler::inst_flags::NO_DCE);
                COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                    << "Remove StringBuilder toString()-call (id=" << (*inst)->GetId() << ")";
                isApplied_ = true;
            } else {
                removeInstance = false;
                break;
            }
        }

        // Remove StringBuilder instance unless it has usages
        if (removeInstance && CanRemoveFromSaveStates(instance) &&
            !IsUsedOutsideBasicBlock(instance, instance->GetBasicBlock())) {
            RemoveStringBuilderInstance(instance);
            isApplied_ = true;
        }

        // Proceed to the next StringBuilder constructor
        inst = nextInst != block->Insts().end() ? nextInst : inst;
    }
}

InstIter SimplifyStringBuilder::SkipToStringBuilderDefaultConstructor(InstIter begin, InstIter end)
{
    return std::find_if(std::move(begin), std::move(end),
                        [](auto inst) { return IsMethodStringBuilderDefaultConstructor(inst); });
}

InstIter SimplifyStringBuilder::SkipToStringBuilderDefaultOrStringArgConstructor(InstIter begin, InstIter end)
{
    return std::find_if(std::move(begin), std::move(end), [](auto inst) {
        return IsMethodStringBuilderDefaultConstructor(inst) || IsMethodStringBuilderConstructorWithStringArg(inst);
    });
}

IntrinsicInst *SimplifyStringBuilder::CreateConcatIntrinsic(
    const std::array<IntrinsicInst *, ARGS_NUM_4> &appendIntrinsics, size_t appendCount, DataType::Type type,
    SaveStateInst *saveState)
{
    auto concatIntrinsic =
        GetGraph()->CreateInstIntrinsic(GetGraph()->GetRuntime()->GetStringConcatStringsIntrinsicId(appendCount));
    ASSERT(concatIntrinsic->RequireState());

    concatIntrinsic->SetType(type);
    // Allocate input types (+1 input for save state)
    concatIntrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), appendCount + 1);

    for (size_t index = 0; index < appendCount; ++index) {
        auto arg = appendIntrinsics[index]->GetInput(1).GetInst();
        concatIntrinsic->AppendInput(arg);
        concatIntrinsic->AddInputType(arg->GetType());
    }

    auto saveStateClone = CopySaveState(GetGraph(), saveState);
    concatIntrinsic->AppendInput(saveStateClone);
    concatIntrinsic->AddInputType(saveStateClone->GetType());

    concatIntrinsic->SetPc(saveState->GetPc());

    return concatIntrinsic;
}

bool CheckUnsupportedCases(Inst *instance, Inst *ctorCall)
{
    if (!CanRemoveFromSaveStates(instance)) {
        return false;  // Unsupported case: this instance may be needed for deoptimization
    }

    if (IsUsedOutsideBasicBlock(instance, instance->GetBasicBlock())) {
        return false;  // Unsupported case: doesn't look like concatenation pattern
    }

    auto toStringCount = CountUsers(instance, [](auto &user) { return IsStringBuilderToString(user.GetInst()); });
    if (toStringCount != 1) {
        return false;  // Unsupported case: doesn't look like concatenation pattern
    }

    auto appendCount = CountUsers(instance, [](auto &user) { return IsStringBuilderAppend(user.GetInst()); });
    auto appendStringCount =
        CountUsers(instance, [](auto &user) { return IsIntrinsicStringBuilderAppendString(user.GetInst()); });
    if (appendCount != appendStringCount) {
        return false;  // Unsupported case: arguments must be strings
    }
    if (IsMethodStringBuilderDefaultConstructor(ctorCall)) {
        return appendCount > 1 && appendCount <= ARGS_NUM_4;
    }
    if (IsMethodStringBuilderConstructorWithStringArg(ctorCall)) {
        return appendCount > 0 && appendCount <= ARGS_NUM_3;
    }
    UNREACHABLE();
    return false;
}

bool SimplifyStringBuilder::MatchConcatenation(InstIter &begin, const InstIter &end, ConcatenationMatch &match)
{
    auto instance = match.instance;
    if (!CheckUnsupportedCases(instance, match.ctorCall)) {
        return false;
    }

    // Walk instruction range [begin, end) and fill the match structure with StringBuilder usage instructions found
    for (; begin != end; ++begin) {
        if ((*begin)->IsSaveState()) {
            continue;
        }

        // Skip instruction having nothing to do with current instance
        if (!IsDataFlowInput(*begin, instance)) {
            continue;
        }

        // Walk through NullChecks
        auto inst = *begin;
        if (inst->IsNullCheck()) {
            if (!inst->HasSingleUser()) {
                return false;  // Unsupported case: doesn't look like concatenation pattern
            }
            inst = inst->GetUsers().Front().GetInst();
            if (inst->GetBasicBlock() != instance->GetBasicBlock()) {
                return false;  // Unsupported case: doesn't look like concatenation pattern
            }
            continue;
        }

        if (IsIntrinsicStringBuilderAppendString(inst)) {
            auto intrinsic = inst->CastToIntrinsic();
            match.appendIntrinsics[match.appendCount++] = intrinsic;
        } else if (IsStringBuilderToString(inst)) {
            match.toStringCall = *begin;
        } else {
            break;
        }
    }

    if (IsMethodStringBuilderConstructorWithStringArg(match.ctorCall)) {
        if (!match.ctorCall->IsDominate(match.toStringCall)) {
            return false;
        }
    }

    for (size_t index = 0; index < match.appendCount; ++index) {
        if (!match.appendIntrinsics[index]->IsDominate(match.toStringCall)) {
            return false;
        }
    }

    // Supported case: number of toString calls is one,
    // number of append calls is between 2 and 4,
    // toString call is dominated by all append calls.
    return true;
}

void SimplifyStringBuilder::FixBrokenSaveStates(Inst *source, Inst *target)
{
    if (source->IsMovableObject()) {
        ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), source, target);
    }
}

void SimplifyStringBuilder::Check(const ConcatenationMatch &match)
{
    // NOLINTBEGIN(readability-magic-numbers)
    [[maybe_unused]] auto &appendIntrinsics = match.appendIntrinsics;
    if (IsMethodStringBuilderDefaultConstructor(match.ctorCall)) {
        ASSERT(match.appendCount >= ARGS_NUM_2 && match.appendCount <= ARGS_NUM_4);
    } else if (IsMethodStringBuilderConstructorWithStringArg(match.ctorCall)) {
        ASSERT(match.appendCount >= ARGS_NUM_1 && match.appendCount <= ARGS_NUM_3);
    }
    for (unsigned i = 0; i < match.appendCount; ++i) {
        ASSERT(appendIntrinsics[i] != nullptr);
        ASSERT(appendIntrinsics[i]->GetInputsCount() > 1);
    }
    // NOLINTEND(readability-magic-numbers)
}

void SimplifyStringBuilder::InsertIntrinsicAndFixSaveStates(
    IntrinsicInst *concatIntrinsic, const std::array<IntrinsicInst *, ARGS_NUM_4> &appendIntrinsics, size_t appendCount,
    Inst *before)
{
    InsertBeforeWithSaveState(concatIntrinsic, before);
    for (size_t index = 0; index < appendCount; ++index) {
        auto arg = appendIntrinsics[index]->GetDataFlowInput(1);
        FixBrokenSaveStates(arg, concatIntrinsic);
    }
}

void SimplifyStringBuilder::InsertNewAppendIntrinsic(ConcatenationMatch &match)
{
    auto newAppInst =
        CreateIntrinsicStringBuilderAppendString(match.instance, match.ctorCall->GetInput(1).GetInst(),
                                                 CopySaveState(GetGraph(), match.ctorCall->GetSaveState()));
    InsertAfterWithSaveState(newAppInst, match.ctorCall);
    ASSERT(match.appendCount < ARGS_NUM_4);
    for (size_t index = match.appendCount; index > 0; --index) {
        match.appendIntrinsics[index] = match.appendIntrinsics[index - 1];
    }
    match.appendIntrinsics[0] = newAppInst;
    match.appendCount += 1;
}

void SimplifyStringBuilder::ReplaceWithConcatIntrinsic(const ConcatenationMatch &match)
{
    auto &appendIntrinsics = match.appendIntrinsics;
    auto toStringCall = match.toStringCall;

    auto concatIntrinsic = CreateConcatIntrinsic(appendIntrinsics, match.appendCount, toStringCall->GetType(),
                                                 toStringCall->GetSaveState());
    InsertIntrinsicAndFixSaveStates(concatIntrinsic, appendIntrinsics, match.appendCount, toStringCall);
    toStringCall->ReplaceUsers(concatIntrinsic);

    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Replace StringBuilder (id=" << match.ctorCall->GetId()
                                     << ") append-intrinsics with concat intrinsic (id=" << concatIntrinsic->GetId()
                                     << ")";
}

void SimplifyStringBuilder::Cleanup(const ConcatenationMatch &match)
{
    RemoveStringBuilderInstance(match.instance);
}

void SimplifyStringBuilder::OptimizeStringConcatenation(BasicBlock *block)
{
    // Replace String Builder usage with string concatenation whenever optimal

    ASSERT(block != nullptr);
    ASSERT(block->GetGraph() == GetGraph());

    MarkerHolder visitedMarker {GetGraph()};
    Marker visited = visitedMarker.GetMarker();

    bool isAppliedLocal;
    do {
        isAppliedLocal = false;

        // Walk through a basic block, find every String concatenation pattern,
        // and check if we can optimize them
        InstIter inst = block->Insts().begin();
        while ((inst = SkipToStringBuilderDefaultOrStringArgConstructor(inst, block->Insts().end())) !=
               block->Insts().end()) {
            ASSERT((*inst)->IsStaticCall());
            auto ctorCall = (*inst)->CastToCallStatic();

            ASSERT(ctorCall->GetInputsCount() > 0);
            auto instance = ctorCall->GetInput(0).GetInst();

            ++inst;
            if (instance->IsMarked(visited)) {
                continue;
            }

            ConcatenationMatch match {instance, ctorCall};
            // Check if current StringBuilder instance can be optimized
            if (!MatchConcatenation(inst, block->Insts().end(), match)) {
                continue;
            }
            Check(match);

            if (IsMethodStringBuilderConstructorWithStringArg(match.ctorCall)) {
                InsertNewAppendIntrinsic(match);
            }
            ReplaceWithConcatIntrinsic(match);
            Cleanup(match);

            instance->SetMarker(visited);

            isAppliedLocal = true;
            isApplied_ = true;
        }
    } while (isAppliedLocal);
}

void SimplifyStringBuilder::ConcatenationLoopMatch::TemporaryInstructions::Clear()
{
    intermediateValue = nullptr;
    toStringCall = nullptr;
    instance = nullptr;
    ctorCall = nullptr;
    appendAccValue = nullptr;
}

bool SimplifyStringBuilder::ConcatenationLoopMatch::TemporaryInstructions::IsEmpty() const
{
    if (ctorCall != nullptr) {
        if (IsMethodStringBuilderDefaultConstructor(ctorCall)) {
            return toStringCall == nullptr || intermediateValue == nullptr || instance == nullptr ||
                   appendAccValue == nullptr;
        }
        if (IsMethodStringBuilderConstructorWithStringArg(ctorCall)) {
            return toStringCall == nullptr || intermediateValue == nullptr || instance == nullptr;
        }
        UNREACHABLE();
    }
    return false;
}

void SimplifyStringBuilder::ConcatenationLoopMatch::Clear()
{
    block = nullptr;
    accValue = nullptr;
    initialValue = nullptr;

    preheader.instance = nullptr;
    preheader.ctorCall = nullptr;
    preheader.appendAccValue = nullptr;

    loop.appendInstructions.clear();
    loop.toStringLengthChains.clear();
    temp.clear();
    exit.toStringCall = nullptr;
}

bool SimplifyStringBuilder::HasAppendOrLengthUsersOnly(Inst *inst, Inst *ctorCall) const
{
    MarkerHolder visited {inst->GetBasicBlock()->GetGraph()};
    bool found = HasUserPhiRecursively(inst, visited.GetMarker(), [inst, ctorCall](auto &user) {
        bool sameLoop = user.GetInst()->GetBasicBlock()->GetLoop() == inst->GetBasicBlock()->GetLoop();
        bool isSaveState = user.GetInst()->IsSaveState();
        bool isPhi = user.GetInst()->IsPhi();
        bool isStringLength = IsStringLengthAccessorChain(user.GetInst());
        if (IsMethodStringBuilderDefaultConstructor(ctorCall)) {
            bool isAppendInstruction = IsStringBuilderAppend(user.GetInst());
            return sameLoop && !isSaveState && !isPhi && !isAppendInstruction && !isStringLength;
        }
        if (IsMethodStringBuilderConstructorWithStringArg(ctorCall)) {
            bool isSbCtorStrInstruction = IsMethodStringBuilderConstructorWithStringArg(user.GetInst());
            return sameLoop && !isSaveState && !isPhi && !isSbCtorStrInstruction && !isStringLength;
        }
        UNREACHABLE();
        return true;
    });
    ResetUserMarkersRecursively(inst, visited.GetMarker());
    return !found;
}

bool IsCheckCastWithoutUsers(Inst *inst)
{
    return inst->GetOpcode() == Opcode::CheckCast && !inst->HasUsers();
}

bool SimplifyStringBuilder::HasPhiOrAppendOrLengthUsersOnly(Inst *inst, Inst *ctorCall,
                                                            Marker appendInstructionVisited) const
{
    MarkerHolder phiVisited {GetGraph()};
    // Release clang-tidy-14 claims match.exit.toStringCall as nullable ignorring IsInstanceHoistable checks
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    auto fnCheckUser = [loop = inst->GetBasicBlock()->GetLoop(), ctorCall, appendInstructionVisited](auto &user) {
        bool sameLoop = user.GetInst()->GetBasicBlock()->GetLoop() == loop;
        bool isSaveState = user.GetInst()->IsSaveState();
        bool isCheckCast = IsCheckCastWithoutUsers(user.GetInst());
        bool isPhi = user.GetInst()->IsPhi();
        bool isVisited = user.GetInst()->IsMarked(appendInstructionVisited);
        bool isStringLength = IsStringLengthAccessorChain(user.GetInst());
        if (IsMethodStringBuilderDefaultConstructor(ctorCall)) {
            bool isAppendInstruction = IsStringBuilderAppend(user.GetInst());
            return sameLoop && !isSaveState && !isCheckCast && !isStringLength && !isPhi &&
                   !(isAppendInstruction && isVisited);
        }
        if (IsMethodStringBuilderConstructorWithStringArg(ctorCall)) {
            bool isSbCtorStrInstruction = IsMethodStringBuilderConstructorWithStringArg(user.GetInst());
            return sameLoop && !isSaveState && !isCheckCast && !isStringLength && !isPhi &&
                   !(isSbCtorStrInstruction && isVisited);
        }
        UNREACHABLE();
        return true;
    };
    bool found = HasUserPhiRecursively(inst, phiVisited.GetMarker(), fnCheckUser);
    return !found;
}

bool SimplifyStringBuilder::ConcatenationLoopMatch::IsInstanceHoistable() const
{
    if (block == nullptr || accValue == nullptr || initialValue == nullptr) {
        return false;
    }

    if (preheader.instance == nullptr || preheader.ctorCall == nullptr || preheader.appendAccValue == nullptr) {
        return false;
    }

    if (block != preheader.instance->GetBasicBlock() || block != preheader.ctorCall->GetBasicBlock() ||
        block != preheader.appendAccValue->GetBasicBlock()) {
        return false;
    }

    if ((block->IsTry() || block->IsCatch()) && block->GetTryId() != block->GetLoop()->GetPreHeader()->GetTryId()) {
        return false;
    }

    if (loop.appendInstructions.empty()) {
        return false;
    }

    if (exit.toStringCall == nullptr) {
        return false;
    }

    return true;
}

bool SimplifyStringBuilder::IsInstanceHoistable(const ConcatenationLoopMatch &match) const
{
    return match.IsInstanceHoistable() && HasAppendOrLengthUsersOnly(match.accValue, match.preheader.ctorCall);
}

bool SimplifyStringBuilder::IsToStringHoistable(const ConcatenationLoopMatch &match,
                                                Marker appendInstructionVisited) const
{
    ASSERT(match.exit.toStringCall != nullptr);
    return HasPhiOrAppendOrLengthUsersOnly(match.exit.toStringCall, match.preheader.ctorCall, appendInstructionVisited);
}

SaveStateInst *FindPreHeaderSaveState(Loop *loop)
{
    for (const auto &inst : loop->GetPreHeader()->InstsReverse()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            return inst->CastToSaveState();
        }
    }
    return nullptr;
}

size_t CountOuterLoopSuccs(BasicBlock *block)
{
    return std::count_if(block->GetSuccsBlocks().begin(), block->GetSuccsBlocks().end(),
                         [block](auto succ) { return block->GetLoop()->IsInside(succ->GetLoop()); });
}

BasicBlock *GetOuterLoopSucc(BasicBlock *block)
{
    auto found = std::find_if(block->GetSuccsBlocks().begin(), block->GetSuccsBlocks().end(),
                              [block](auto succ) { return block->GetLoop()->IsInside(succ->GetLoop()); });
    return found != block->GetSuccsBlocks().end() ? *found : nullptr;
}

BasicBlock *GetLoopPostExit(Loop *loop)
{
    // Find a block immediately following after loop
    // Supported case:
    //  1. Preheader block exists
    //  2. Loop has exactly one block with exactly one successor outside a loop (post exit block)
    //  3. Preheader dominates post exit block found
    // Loop structures different from the one described above are not supported in current implementation

    BasicBlock *postExit = nullptr;
    for (auto block : loop->GetBlocks()) {
        size_t count = CountOuterLoopSuccs(block);
        if (count == 0) {
            continue;
        }
        if (count == 1 && postExit == nullptr) {
            postExit = GetOuterLoopSucc(block);
            continue;
        }
        // Unsupported case
        return nullptr;
    }

    // Supported case
    if (postExit != nullptr && postExit->GetPredsBlocks().size() == 1 && loop->GetPreHeader() != nullptr &&
        loop->GetPreHeader()->IsDominate(postExit)) {
        return postExit;
    }

    // Unsupported case
    return nullptr;
}

CallInst *SimplifyStringBuilder::CreateStringBuilderAppendString(Inst *instance, Inst *arg, SaveStateInst *saveState)
{
    auto graph = GetGraph();
    auto runtime = graph->GetRuntime();
    auto methodPtr = runtime->GetMethodStringBuilderAppendString(runtime->GetStringBuilderClass());
    ASSERT(methodPtr != nullptr);
    auto methodId = runtime->GetMethodId(methodPtr);
    auto newAppend =
        graph->CreateInstCallStatic(runtime->GetMethodReturnType(methodPtr), instance->GetPc(), methodId, methodPtr);
    ASSERT(newAppend != nullptr);
    newAppend->SetInputs(graph->GetAllocator(),
                         {{instance, instance->GetType()}, {arg, arg->GetType()}, {saveState, saveState->GetType()}});
    return newAppend;
}

IntrinsicInst *CreateIntrinsic(Graph *graph, RuntimeInterface::IntrinsicId intrinsicId, DataType::Type type,
                               const std::initializer_list<std::pair<Inst *, DataType::Type>> &args)
{
    auto intrinsic = graph->CreateInstIntrinsic(intrinsicId);

    intrinsic->SetType(type);
    intrinsic->SetInputs(graph->GetAllocator(), args);

    return intrinsic;
}

IntrinsicInst *SimplifyStringBuilder::CreateIntrinsicStringBuilderAppendString(Inst *instance, Inst *arg,
                                                                               SaveStateInst *saveState)
{
    return CreateIntrinsic(GetGraph(), GetGraph()->GetRuntime()->GetStringBuilderAppendStringsIntrinsicId(1U),
                           DataType::REFERENCE,
                           {{instance, instance->GetType()}, {arg, arg->GetType()}, {saveState, saveState->GetType()}});
}

Inst *SimplifyStringBuilder::NormalizeStringBuilderAppendInstructionUsers(Inst *instance, SaveStateInst *saveState)
{
    Inst *newAppendInst = nullptr;
    [[maybe_unused]] Inst *ctorCall = nullptr;

    for (auto &user : instance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        // Make additional append-call out of constructor argument (if present)
        if (IsMethodStringBuilderConstructorWithStringArg(userInst)) {
            ASSERT(ctorCall == nullptr);
            ctorCall = userInst;
            auto ctorArg = ctorCall->GetInput(1).GetInst();
            if (!GetGraph()->IsBytecodeOptimizer()) {
                newAppendInst =
                    CreateIntrinsicStringBuilderAppendString(instance, ctorArg, CopySaveState(GetGraph(), saveState));
            } else {
                newAppendInst =
                    CreateStringBuilderAppendString(instance, ctorArg, CopySaveState(GetGraph(), saveState));
            }
            InsertAfterWithSaveState(newAppendInst, ctorCall);
        } else if (IsMethodStringBuilderDefaultConstructor(userInst)) {
            ASSERT(ctorCall == nullptr);
            ctorCall = userInst;
        } else if (IsStringBuilderAppend(userInst)) {
            // StringBuilder append-call returns 'this' (instance)
            // Replace all users of append-call by instance for simplicity
            userInst->ReplaceUsers(instance);
        }
    }
    ASSERT(ctorCall != nullptr);
    return newAppendInst;
}

ArenaVector<Inst *> SimplifyStringBuilder::FindStringBuilderAppendInstructions(Inst *instance)
{
    ArenaVector<Inst *> appendInstructions {GetGraph()->GetAllocator()->Adapter()};
    for (auto &user : instance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (IsStringBuilderAppend(userInst)) {
            appendInstructions.push_back(userInst);
        }
    }

    return appendInstructions;
}

void SimplifyStringBuilder::RemoveFromSaveStateInputs(Inst *inst, bool doMark)
{
    inputDescriptors_.clear();

    for (auto &user : inst->GetUsers()) {
        if (!user.GetInst()->IsSaveState()) {
            continue;
        }
        inputDescriptors_.emplace_back(user.GetInst(), user.GetIndex());
    }

    RemoveFromInstructionInputs(inputDescriptors_, doMark);
}

void SimplifyStringBuilder::RemoveFromAllExceptPhiInputs(Inst *inst)
{
    inputDescriptors_.clear();

    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->IsPhi()) {
            continue;
        }
        inputDescriptors_.emplace_back(user.GetInst(), user.GetIndex());
    }

    RemoveFromInstructionInputs(inputDescriptors_);
}

void SimplifyStringBuilder::RemoveStringBuilderInstance(Inst *instance)
{
    ASSERT(!HasUser(instance, [](auto &user) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        auto hasUsers = userInst->HasUsers();
        auto isSaveState = userInst->IsSaveState();
        auto isCtorCall = IsMethodStringBuilderDefaultConstructor(userInst) ||
                          IsMethodStringBuilderConstructorWithStringArg(userInst) ||
                          IsMethodStringBuilderConstructorWithCharArrayArg(userInst);
        auto isAppendInstruction = IsStringBuilderAppend(userInst);
        auto isToStringCall = IsStringBuilderToString(userInst);
        return !(isSaveState || isCtorCall || ((isAppendInstruction || isToStringCall) && !hasUsers));
    }));
    ASSERT(CanRemoveFromSaveStates(instance));

    RemoveFromSaveStateInputs(instance, true);

    for (auto &user : instance->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsCheck()) {
            auto checkUserInst = SkipSingleUserCheckInstruction(userInst);
            checkUserInst->GetBasicBlock()->RemoveInst(checkUserInst);
            COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                << "Remove StringBuilder user instruction (id=" << checkUserInst->GetId() << ")";
        }

        userInst->GetBasicBlock()->RemoveInst(userInst);
        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Remove StringBuilder user instruction (id=" << userInst->GetId() << ")";
    }

    ASSERT(!instance->HasUsers());
    instance->GetBasicBlock()->RemoveInst(instance);
    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Remove StringBuilder instance (id=" << instance->GetId() << ")";
}

void SimplifyStringBuilder::ReconnectStringBuilderCascade(Inst *instance, Inst *inputInst, Inst *appendInstruction,
                                                          SaveStateInst *saveState)
{
    // Reconnect all append-calls of input_instance to instance instruction

    // Check if input of append-call is toString-call
    if (inputInst->GetBasicBlock() != appendInstruction->GetBasicBlock() || !IsStringBuilderToString(inputInst)) {
        return;
    }

    // Get cascading instance of input toString-call
    auto inputToStringCall = inputInst;
    ASSERT(inputToStringCall->GetInputsCount() > 0);
    auto inputInstance = inputToStringCall->GetDataFlowInput(0);
    if (inputInstance == instance) {
        return;
    }

    instructionsStack_.push(inputInstance);

    auto newAppendInst = NormalizeStringBuilderAppendInstructionUsers(inputInstance, saveState);
    for (auto inputAppendInstruction : FindStringBuilderAppendInstructions(inputInstance)) {
        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Retarget append instrisic (id=" << inputAppendInstruction->GetId()
                                         << ") to instance (id=" << instance->GetId() << ")";

        if (newAppendInst != nullptr && newAppendInst == inputAppendInstruction) {
            inputAppendInstruction->SetInput(0, instance);
        } else {
            inputAppendInstruction->SetInput(0, instance);
            inputAppendInstruction->SetSaveState(saveState);

            if (inputAppendInstruction->GetBasicBlock() != nullptr) {
                inputAppendInstruction->GetBasicBlock()->EraseInst(inputAppendInstruction, true);
            }
            appendInstruction->InsertAfter(inputAppendInstruction);
        }

        for (auto &input : inputAppendInstruction->GetInputs()) {
            if (input.GetInst()->IsSaveState()) {
                continue;
            }
            FixBrokenSaveStates(input.GetInst(), inputAppendInstruction);
        }
    }

    // Erase input_instance constructor from block
    for (auto &user : inputInstance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (IsMethodStringBuilderConstructorWithStringArg(userInst) ||
            IsMethodStringBuilderDefaultConstructor(userInst)) {
            userInst->ClearFlag(compiler::inst_flags::NO_DCE);
        }
    }

    // Cleanup save states
    RemoveFromSaveStateInputs(inputInstance);
    // Erase input_instance itself
    inputInstance->ClearFlag(compiler::inst_flags::NO_DCE);

    // Cleanup instructions we don't need anymore
    appendInstruction->GetBasicBlock()->RemoveInst(appendInstruction);
    RemoveFromSaveStateInputs(inputToStringCall);
    inputToStringCall->ClearFlag(compiler::inst_flags::NO_DCE);
}

void SimplifyStringBuilder::ReconnectStringBuilderCascades(const ConcatenationLoopMatch &match)
{
    // Consider the following code:
    //      str += a + b + c + ...
    // StringBuilder equivalent view:
    //      sb_abc.append(a)
    //      sb_abc.append(b)
    //      sb_abc.append(c)
    //      sb_str.append(sb_abc.toString())
    //
    // We call StringBuilder cascading to be calls like sb_str.append(sb_abc.toString()), i.e output of one SB used as
    // input of another SB
    //
    // The code below transforms this into:
    //      sb_str.append(a)
    //      sb_str.append(b)
    //      sb_str.append(c)
    // i.e appending args directly to string 'str', w/o the use of intermediate SB

    ASSERT(instructionsStack_.empty());
    instructionsStack_.push(match.preheader.instance);

    while (!instructionsStack_.empty()) {
        auto instance = instructionsStack_.top();
        instructionsStack_.pop();

        // For each append-call of current StringBuilder instance
        for (auto appendInstruction : FindStringBuilderAppendInstructions(instance)) {
            for (auto &input : appendInstruction->GetInputs()) {
                auto inputInst = input.GetInst();

                // Reconnect append-calls of cascading instance
                ReconnectStringBuilderCascade(instance, inputInst, appendInstruction,
                                              appendInstruction->GetSaveState());
            }
        }
    }
}

RuntimeInterface::FieldPtr SimplifyStringBuilder::GetGetterStringBuilderStringLength()
{
    if (UNLIKELY(sbStringLengthField_ == nullptr)) {
        auto runtime = GetGraph()->GetRuntime();
        sbStringLengthField_ = runtime->GetGetterStringBuilderStringLength();
        ASSERT(sbStringLengthField_ != nullptr);
    }
    return sbStringLengthField_;
}

void SimplifyStringBuilder::ReconnectToStringLengthChains(const ConcatenationLoopMatch &match)
{
    if (match.loop.toStringLengthChains.empty()) {
        return;
    }

    auto *runtime = GetGraph()->GetRuntime();
    auto sbClass = runtime->GetStringBuilderClass();
    auto lengthField = runtime->GetFieldStringBuilderLength(sbClass);
    auto loop = match.block->GetLoop();

    MarkerHolder chainsVisited {GetGraph()};
    for (auto &chain : match.loop.toStringLengthChains) {
        ASSERT(chain.toStringCallOrPhi == match.accValue);

        auto nullCheckOrLenArray = chain.nullCheckCall != nullptr ? chain.nullCheckCall : chain.lenArrayCall;
        if (nullCheckOrLenArray->SetMarker(chainsVisited.GetMarker())) {
            continue;
        }

        auto checkLoop = nullCheckOrLenArray->GetBasicBlock()->GetLoop();
        if (checkLoop != loop && !checkLoop->IsInside(loop)) {
            continue;
        }

        if (chain.nullCheckCall != nullptr) {
            chain.nullCheckCall->ReplaceInput(match.accValue, match.preheader.instance);
        }
        Inst *lengthInst = nullptr;
        if (lengthField != nullptr) {
            ASSERT(chain.nullCheckCall != nullptr);
            lengthInst = GetGraph()->CreateInstLoadObject(
                DataType::INT32, chain.lenArrayCall->GetPc(), chain.nullCheckCall,
                TypeIdMixin {runtime->GetFieldId(lengthField), GetGraph()->GetMethod()}, lengthField,
                runtime->IsFieldVolatile(lengthField));
            chain.nullCheckCall->InsertAfter(lengthInst);
        } else {
            auto methodId = runtime->GetMethodId(GetGetterStringBuilderStringLength());

            auto lengthCallInst = GetGraph()->CreateInstCallStatic(DataType::INT32, chain.lenArrayCall->GetPc(),
                                                                   methodId, GetGetterStringBuilderStringLength());

            auto saveState = match.loop.appendInstructions.front()->CastToCallStatic()->GetSaveState();
            auto saveStateClone = CopySaveState(GetGraph(), saveState);
            lengthCallInst->SetInputs(GetGraph()->GetAllocator(),
                                      {{match.preheader.instance, match.preheader.instance->GetType()},
                                       {saveStateClone, saveStateClone->GetType()}});

            lengthInst = lengthCallInst;
            InsertAfterWithSaveState(lengthInst, chain.lenArrayCall);
        }

        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "[.length] Replace string.length call (id=" << chain.lenArrayCall->GetId()
                                         << ") to sb.length (id=" << lengthInst->GetId() << ")";

        Inst *chainTail = (chain.shrLengthShift != nullptr) ? chain.shrLengthShift : chain.lenArrayCall;
        chainTail->ReplaceUsers(lengthInst);
        chain.lenArrayCall->ClearFlag(inst_flags::NO_DCE);
        chainTail->ClearFlag(inst_flags::NO_DCE);
    }
}

void SimplifyStringBuilder::ReconnectInstructions(const ConcatenationLoopMatch &match)
{
    // Make StringBuilder append-call hoisted point to an instance hoisted and initialize it with string initial value
    match.preheader.appendAccValue->SetInput(0, match.preheader.instance);
    match.preheader.appendAccValue->SetInput(1, match.initialValue);

    // Make temporary instance users point to an instance hoisted
    for (auto &temp : match.temp) {
        instructionsVector_.clear();
        for (auto &user : temp.instance->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsSaveState()) {
                continue;
            }
            instructionsVector_.push_back(userInst);
        }
        for (auto userInst : instructionsVector_) {
            userInst->ReplaceInput(temp.instance, match.preheader.instance);

            COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                << "Replace input of instruction "
                << "id=" << userInst->GetId() << " (" << GetOpcodeString(userInst->GetOpcode())
                << ") old id=" << temp.instance->GetId() << " (" << GetOpcodeString(temp.instance->GetOpcode())
                << ") new id=" << match.preheader.instance->GetId() << " ("
                << GetOpcodeString(match.preheader.instance->GetOpcode()) << ")";
        }
    }

    ReconnectToStringLengthChains(match);

    // Replace users of accumulated value outside the loop by toString-call hoisted to post exit block
    instructionsVector_.clear();
    for (auto &user : match.accValue->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsSaveState()) {
            continue;
        }
        if (userInst->GetBasicBlock() != match.block) {
            instructionsVector_.push_back(userInst);
        }
    }

    for (auto userInst : instructionsVector_) {
        userInst->ReplaceInput(match.accValue, match.exit.toStringCall);

        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Replace input of instruction "
                                         << "id=" << userInst->GetId() << " (" << GetOpcodeString(userInst->GetOpcode())
                                         << ") old id=" << match.accValue->GetId() << " ("
                                         << GetOpcodeString(match.accValue->GetOpcode())
                                         << ") new id=" << match.exit.toStringCall->GetId() << " ("
                                         << GetOpcodeString(match.exit.toStringCall->GetOpcode()) << ")";
    }

    ReconnectStringBuilderCascades(match);
}

bool AllInstructionInputsDominate(Inst *inst, Inst *other)
{
    return !HasInput(inst, [other](auto &input) { return !input.GetInst()->IsDominate(other); });
}

Inst *SimplifyStringBuilder::HoistInstructionToPreHeader(BasicBlock *preHeader, Inst *lastInst, Inst *inst,
                                                         SaveStateInst *saveState)
{
    // Based on similar code in LICM-pass

    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Hoist instruction id=" << inst->GetId() << " ("
                                     << GetOpcodeString(inst->GetOpcode()) << ")"
                                     << " from loop block id=" << inst->GetBasicBlock()->GetId()
                                     << " to preheader block id=" << preHeader->GetId();

    Inst *target = nullptr;
    if (inst->IsMovableObject()) {
        target = inst->GetPrev();
        if (target == nullptr) {
            target = inst->GetNext();
        }
        if (target == nullptr) {
            target = GetGraph()->CreateInstNOP();
            inst->InsertAfter(target);
        }
    }
    inst->GetBasicBlock()->EraseInst(inst, true);
    if (lastInst == nullptr || lastInst->IsPhi()) {
        preHeader->AppendInst(inst);
        lastInst = inst;
    } else {
        ASSERT(lastInst->GetBasicBlock() == preHeader);
        Inst *saveStateDeoptimize = preHeader->FindSaveStateDeoptimize();
        if (saveStateDeoptimize != nullptr && AllInstructionInputsDominate(inst, saveStateDeoptimize)) {
            saveStateDeoptimize->InsertBefore(inst);
        } else {
            lastInst->InsertAfter(inst);
            lastInst = inst;
        }
    }

    if (inst->RequireState()) {
        ASSERT(saveState != nullptr);
        auto saveStateClone = CopySaveState(GetGraph(), saveState);
        if (saveStateClone->GetBasicBlock() == nullptr) {
            inst->InsertBefore(saveStateClone);
        }
        inst->SetSaveState(saveStateClone);
        inst->SetPc(saveStateClone->GetPc());
    }

    FixBrokenSaveStates(inst, target);

    return lastInst;
}

Inst *SimplifyStringBuilder::HoistInstructionToPreHeaderRecursively(BasicBlock *preHeader, Inst *lastInst, Inst *inst,
                                                                    SaveStateInst *saveState)
{
    // Hoist all non-SaveState instruction inputs first
    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->IsSaveState()) {
            continue;
        }

        if (inputInst->GetBasicBlock() == preHeader) {
            continue;
        }
        // Only instructions from first input are moved to preheader in this case
        if (IsMethodStringBuilderConstructorWithStringArg(inst) && inputInst != inst->GetInput(0).GetInst()) {
            continue;
        }
        lastInst = HoistInstructionToPreHeaderRecursively(preHeader, lastInst, inputInst, saveState);
        if (lastInst->RequireState()) {
            saveState = lastInst->GetSaveState();
        }
    }

    // Hoist instruction itself
    return HoistInstructionToPreHeader(preHeader, lastInst, inst, saveState);
}

void SimplifyStringBuilder::HoistInstructionsToPreHeader(const ConcatenationLoopMatch &match,
                                                         SaveStateInst *initSaveState)
{
    // Move StringBuilder construction and initialization from inside loop to preheader block

    auto loop = match.block->GetLoop();
    auto preHeader = loop->GetPreHeader();

    HoistInstructionToPreHeaderRecursively(preHeader, preHeader->GetLastInst(), match.preheader.ctorCall,
                                           initSaveState);

    ASSERT(match.preheader.ctorCall->RequireState());
    if (IsMethodStringBuilderDefaultConstructor(match.preheader.ctorCall)) {
        HoistInstructionToPreHeader(preHeader, match.preheader.ctorCall, match.preheader.appendAccValue,
                                    match.preheader.ctorCall->GetSaveState());
    }
    for (auto &input : match.preheader.appendAccValue->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->IsSaveState()) {
            continue;
        }

        FixBrokenSaveStates(inputInst, match.preheader.appendAccValue);
    }
}

void HoistCheckInstructionInputs(Inst *inst, BasicBlock *loopBlock, BasicBlock *postExit)
{
    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->GetBasicBlock() == loopBlock && inputInst->IsCheck()) {
            inputInst->GetBasicBlock()->EraseInst(inputInst, true);
            if (inputInst->RequireState()) {
                inputInst->SetSaveState(CopySaveState(loopBlock->GetGraph(), inst->GetSaveState()));
            }
            InsertBeforeWithSaveState(inputInst, inst);

            COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                << "Hoist instruction id=" << inputInst->GetId() << " (" << GetOpcodeString(inputInst->GetOpcode())
                << ") from loop block id=" << loopBlock->GetId() << " to post exit block id=" << postExit->GetId();
        }
    }
}

void SimplifyStringBuilder::HoistCheckCastInstructionUsers(Inst *inst, BasicBlock *loopBlock, BasicBlock *postExit)
{
    // Hoist CheckCast instruction to post exit block

    // Start with CheckCast instruction itself
    ASSERT(instructionsStack_.empty());
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetBasicBlock() == loopBlock && IsCheckCastWithoutUsers(userInst)) {
            instructionsStack_.push(userInst);
        }
    }

    // Collect all the inputs of CheckCast instruction as well
    instructionsVector_.clear();
    while (!instructionsStack_.empty()) {
        auto userInst = instructionsStack_.top();
        instructionsStack_.pop();
        for (auto &input : userInst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (inputInst->IsSaveState()) {
                continue;
            }
            if (inputInst->GetBasicBlock() != loopBlock) {
                continue;
            }

            instructionsStack_.push(inputInst);
        }
        instructionsVector_.push_back(userInst);
    }

    // Hoist collected instructions
    for (auto userInst : instructionsVector_) {
        userInst->GetBasicBlock()->EraseInst(userInst, true);
        if (userInst->RequireState()) {
            userInst->SetSaveState(CopySaveState(GetGraph(), inst->GetSaveState()));
        }
        InsertAfterWithSaveState(userInst, inst);

        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Hoist instruction id=" << userInst->GetId() << " ("
                                         << GetOpcodeString(userInst->GetOpcode())
                                         << ") from loop block id=" << loopBlock->GetId()
                                         << " to post exit block id=" << postExit->GetId()
                                         << " after inst id=" << inst->GetId();
    }
}

void SimplifyStringBuilder::HoistInstructionsToPostExit(const ConcatenationLoopMatch &match, SaveStateInst *saveState)
{
    // Move toString()-call and its inputs/users (null-checks, etc) out of the loop
    // and place them in the loop exit successor block

    auto postExit = GetLoopPostExit(match.block->GetLoop());
    ASSERT(postExit != nullptr);
    ASSERT(!postExit->IsEmpty());
    ASSERT(saveState->GetBasicBlock() == postExit);
    ASSERT(match.exit.toStringCall->RequireState());

    auto loopBlock = match.exit.toStringCall->GetBasicBlock();
    loopBlock->EraseInst(match.exit.toStringCall, true);

    ASSERT(saveState != nullptr);
    if (!saveState->HasUsers()) {
        // First use of save state, insert toString-call after it
        match.exit.toStringCall->SetSaveState(saveState);
        saveState->InsertAfter(match.exit.toStringCall);
    } else {
        // Duplicate save state and prepend both instructions
        match.exit.toStringCall->SetSaveState(CopySaveState(GetGraph(), saveState));
        InsertBeforeWithSaveState(match.exit.toStringCall, postExit->GetFirstInst());
    }

    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Hoist toString()-call instruction id=" << match.exit.toStringCall->GetId()
                                     << " from loop block id=" << loopBlock->GetId()
                                     << " to post exit block id=" << postExit->GetId();

    // Hoist all the toString-call Check instructions inputs
    HoistCheckInstructionInputs(match.exit.toStringCall, loopBlock, postExit);

    // Hoist toString-call instructions users
    HoistCheckCastInstructionUsers(match.exit.toStringCall, loopBlock, postExit);
}

void SimplifyStringBuilder::Cleanup(const ConcatenationLoopMatch &match)
{
    // Remove temporary instructions

    for (auto &temp : match.temp) {
        temp.toStringCall->ClearFlag(compiler::inst_flags::NO_DCE);
        for (auto &user : temp.toStringCall->GetUsers()) {
            auto userInst = user.GetInst();
            if (IsCheckCastWithoutUsers(userInst)) {
                userInst->ClearFlag(compiler::inst_flags::NO_DCE);
            }
        }
        if (temp.intermediateValue != match.exit.toStringCall) {
            temp.intermediateValue->ClearFlag(compiler::inst_flags::NO_DCE);
        }
        temp.instance->ReplaceUsers(match.preheader.instance);
        temp.instance->ClearFlag(compiler::inst_flags::NO_DCE);
        temp.ctorCall->ClearFlag(compiler::inst_flags::NO_DCE);
        if (temp.appendAccValue != nullptr) {
            temp.appendAccValue->ClearFlag(compiler::inst_flags::NO_DCE);
        }
    }
}

bool SimplifyStringBuilder::NeedRemoveInputFromSaveStateInstruction(Inst *inputInst)
{
    for (auto &match : matches_) {
        // If input is hoisted toString-call or accumulated phi instruction mark it for removal
        if (inputInst == match.exit.toStringCall || inputInst == match.accValue) {
            return true;
        }

        // If input is removed toString-call (temporary instruction) mark it for removal
        bool isIntermediateValue = std::find_if(match.temp.begin(), match.temp.end(), [inputInst](auto &temp) {
                                       return inputInst == temp.toStringCall;
                                   }) != match.temp.end();
        if (isIntermediateValue) {
            return true;
        }
    }
    return false;
}

void SimplifyStringBuilder::CollectSaveStateInputsForRemoval(Inst *inst)
{
    ASSERT(inst->IsSaveState());

    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        auto inputInst = inst->GetInput(i).GetInst();
        if (NeedRemoveInputFromSaveStateInstruction(inputInst)) {
            inputDescriptors_.emplace_back(inst, i);
        }
    }
}

void SimplifyStringBuilder::CleanupSaveStateInstructionInputs(Loop *loop)
{
    // StringBuilder toString-call is either hoisted to post exit block or erased from loop, accumulated value is erased
    // from loop, so this instructions need to be removed from the inputs of save state instructions within current loop

    inputDescriptors_.clear();
    for (auto block : loop->GetBlocks()) {
        for (auto inst : block->Insts()) {
            if (!inst->IsSaveState()) {
                continue;
            }
            CollectSaveStateInputsForRemoval(inst);
        }
    }
    RemoveFromInstructionInputs(inputDescriptors_);
}

Inst *SimplifyStringBuilder::FindHoistedToStringCallInput(Inst *phi)
{
    for (auto &match : matches_) {
        auto toStringCall = match.exit.toStringCall;
        if (HasInput(phi, [toStringCall](auto &input) { return input.GetInst() == toStringCall; })) {
            return toStringCall;
        }
    }

    return nullptr;
}

void SimplifyStringBuilder::CleanupPhiInstructionInputs(Inst *phi)
{
    ASSERT(phi->IsPhi());

    auto toStringCall = FindHoistedToStringCallInput(phi);
    if (toStringCall != nullptr) {
        RemoveFromSaveStateInputs(phi);
        phi->ReplaceUsers(toStringCall);
        phi->GetBasicBlock()->RemoveInst(phi);
    }
}

void SimplifyStringBuilder::CleanupPhiInstructionInputs(Loop *loop)
{
    // Remove toString()-call from accumulated value phi-instruction inputs,
    // replace accumulated value users with toString()-call, and remove accumulated value itself
    for (auto block : loop->GetBlocks()) {
        for (auto phi : block->PhiInstsSafe()) {
            CleanupPhiInstructionInputs(phi);
        }
    }
}

bool SimplifyStringBuilder::HasNotHoistedUser(PhiInst *phi)
{
    return HasUser(phi, [phi](auto &user) {
        auto userInst = user.GetInst();
        bool isSelf = userInst == phi;
        bool isSaveState = userInst->IsSaveState();
        bool isRemovedAppendInstruction =
            IsStringBuilderAppend(userInst) && !userInst->GetFlag(compiler::inst_flags::NO_DCE);
        return !isSelf && !isSaveState && !isRemovedAppendInstruction;
    });
}

void SimplifyStringBuilder::RemoveUnusedPhiInstructions(Loop *loop)
{
    // Remove instructions having no users, or instruction with all users hoisted out of the loop
    for (auto block : loop->GetBlocks()) {
        for (auto phi : block->PhiInstsSafe()) {
            if (HasNotHoistedUser(phi->CastToPhi())) {
                continue;
            }

            RemoveFromAllExceptPhiInputs(phi);
            block->RemoveInst(phi);

            COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                << "Remove unused instruction id=" << phi->GetId() << " (" << GetOpcodeString(phi->GetOpcode())
                << ") from header block id=" << block->GetId();
        }
    }
}

void SimplifyStringBuilder::FixBrokenSaveStates(Loop *loop)
{
    ssb_.FixSaveStatesInBB(loop->GetPreHeader());
    ssb_.FixSaveStatesInBB(GetLoopPostExit(loop));
}

void SimplifyStringBuilder::Cleanup(Loop *loop)
{
    if (!isApplied_) {
        return;
    }

    FixBrokenSaveStates(loop);
    CleanupSaveStateInstructionInputs(loop);
    CleanupPhiInstructionInputs(loop);
    RemoveUnusedPhiInstructions(loop);
}

void SimplifyStringBuilder::MatchStringBuilderUsage(Inst *instance, StringBuilderUsage &usage)
{
    // Find all the usages of a given StringBuilder instance, and pack them into StringBuilderUsage structure:
    // i.e instance itself, constructor-call, all append-calls, all toString-calls

    usage.instance = instance;
    auto loop = instance->GetBasicBlock()->GetLoop();

    for (auto &user : instance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (IsMethodStringBuilderConstructorWithStringArg(userInst) ||
            IsMethodStringBuilderDefaultConstructor(userInst)) {
            usage.ctorCall = userInst;
        } else if (IsStringBuilderAppend(userInst)) {
            // StringBuilder append-call returns 'this' (instance)
            // Replace all users of append-call by instance for simplicity
            userInst->ReplaceUsers(instance);
            usage.appendInstructions.push_back(userInst);
        } else if (IsStringBuilderToString(userInst)) {
            usage.toStringCalls.push_back(userInst);
        }
    }

    ASSERT(usage.ctorCall != nullptr);

    if (IsMethodStringBuilderConstructorWithStringArg(usage.ctorCall)) {
        usage.appendInstructions.push_back(usage.ctorCall);
    }

    for (auto &toStringCall : usage.toStringCalls) {
        MarkerHolder phiVisited {GetGraph()};
        MarkerHolder lenArrayVisited {GetGraph()};
        auto fnVisitStringCallPhiUser = [&usage, &loop, &lenArrayVisited](auto &user) {
            Inst *userInst = user.GetInst();
            if (!IsStringLengthAccessorChain(userInst)) {
                return false;
            }

            auto userLoop = userInst->GetBasicBlock()->GetLoop();
            if (userLoop != loop && !userLoop->IsInside(loop)) {
                return false;
            }

            ToStringLengthChain chain;
            chain.toStringCallOrPhi = user.GetInput();
            chain.nullCheckCall = userInst->IsNullCheck() ? userInst : nullptr;
            chain.lenArrayCall = SkipSingleUserCheckInstruction(userInst);
            chain.shrLengthShift = GetStringLengthCompressedShr(chain.lenArrayCall);

            if (chain.lenArrayCall->SetMarker(lenArrayVisited.GetMarker())) {
                return false;
            }

            usage.toStringLengthChains.push_back(chain);
            COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "[.length] Chain has been added"
                                             << " lenArrayCall=v" << chain.lenArrayCall->GetId();
            return false;
        };

        HasUserPhiRecursively(toStringCall, phiVisited.GetMarker(), fnVisitStringCallPhiUser);
    }
}

bool SimplifyStringBuilder::HasInputFromPreHeader(PhiInst *phi) const
{
    auto preHeader = phi->GetBasicBlock()->GetLoop()->GetPreHeader();
    for (size_t i = 0; i < phi->GetInputsCount(); ++i) {
        if (phi->GetPhiInputBb(i) == preHeader) {
            return true;
        }
    }
    return false;
}

bool SimplifyStringBuilder::HasToStringCallInput(PhiInst *phi) const
{
    // Returns true if
    //  (1) 'phi' has StringBuilder.toString call as one of its inputs, and
    //  (2) StringBuilder.toString call has no effective usages except:
    //      (2.a) this 'phi' itself
    //      (2.b) access to .length property (NullCheck->LenArray chain)
    //     Users being save states, check casts, and not used users are skipped.

    MarkerHolder visited {GetGraph()};

    // (1)
    bool hasToStringCallInput = HasInputPhiRecursively(
        phi, visited.GetMarker(), [](auto &input) { return IsStringBuilderToString(input.GetInst()); });

    // (2)
    bool toStringCallInputUsedAnywhereExceptPhiOrLength = HasInput(phi, [phi](auto &input) {
        return IsStringBuilderToString(input.GetInst()) && HasUser(input.GetInst(), [phi](auto &user) {
                   auto userInst = user.GetInst();
                   bool isPhi = userInst == phi;
                   bool isSaveState = userInst->IsSaveState();
                   bool isCheckCast = IsCheckCastWithoutUsers(userInst);
                   bool isStringLength = IsStringLengthAccessorChain(userInst);
                   bool hasUsers = userInst->HasUsers();
                   return !isPhi && !isSaveState && !isCheckCast && !isStringLength && hasUsers;
               });
    });
    ResetInputMarkersRecursively(phi, visited.GetMarker());
    return hasToStringCallInput && !toStringCallInputUsedAnywhereExceptPhiOrLength;
}

static bool UsedByPhiInstInSameBB(const PhiInst *phi)
{
    auto header = phi->GetBasicBlock();
    for (auto &user : phi->GetUsers()) {
        auto inst = user.GetInst();
        if (inst->IsPhi() && inst->GetBasicBlock() == header) {
            return true;
        }
    }
    return false;
}

bool SimplifyStringBuilder::HasInputInst(Inst *inputInst, Inst *inst) const
{
    MarkerHolder visited {GetGraph()};
    bool found = HasInputPhiRecursively(inst, visited.GetMarker(),
                                        [inputInst](auto &input) { return inputInst == input.GetInst(); });
    ResetInputMarkersRecursively(inst, visited.GetMarker());
    return found;
}

bool SimplifyStringBuilder::HasSbCtorStrInstructionUser(Inst *inst) const
{
    Loop *instLoop = inst->GetBasicBlock()->GetLoop();
    for (auto &user : inst->GetUsers()) {
        auto ctorInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (!IsMethodStringBuilderConstructorWithStringArg(ctorInst)) {
            continue;
        }
        if (ctorInst->GetDataFlowInput(1) != inst) {
            continue;
        }
        if (ctorInst->GetBasicBlock()->GetLoop() != instLoop) {
            continue;
        }
        return true;
    }
    return false;
}

bool SimplifyStringBuilder::HasAppendInstructionUser(Inst *inst) const
{
    Loop *instLoop = inst->GetBasicBlock()->GetLoop();
    for (auto &user : inst->GetUsers()) {
        auto appendInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (!IsStringBuilderAppend(appendInst) || appendInst->GetDataFlowInput(1) != inst ||
            appendInst->GetBasicBlock()->GetLoop() != instLoop) {
            continue;
        }

        auto instance = appendInst->GetDataFlowInput(0);
        if (!IsStringBuilderInstance(instance) || instance->GetBasicBlock() != appendInst->GetBasicBlock()) {
            continue;
        }

        // Cycle through instructions from appendInst to sb (backwards) and make sure no other appends in between
        auto currentInst = appendInst->GetPrev();
        while (currentInst != nullptr && currentInst != instance) {
            if (IsStringBuilderAppend(currentInst) && currentInst->GetDataFlowInput(0) == instance) {
                return false;
            }
            currentInst = currentInst->GetPrev();
        }
        return true;
    }

    return false;
}

bool SimplifyStringBuilder::IsPhiAccumulatedValue(PhiInst *phi) const
{
    // Phi-instruction is accumulated value, if it is used in the following way:
    //  bb_preheader:
    //      0 LoadString ...
    //      ...
    //  bb_header:
    //      10p Phi v0(bb_preheader), v20(bb_back_edge)
    //      ...
    //  bb_back_edge:
    //      ...
    //      12.ref NullCheck v10p, save_state
    //      13.i32 LenArray v12
    //      ...
    //      15 Intrinsic.StdCoreSbAppendString sb, v10p, ss
    //      ...
    //      20 CallStatic std.core.StringBuilder::toString sb, ss
    //      ...

    return HasInputFromPreHeader(phi) && HasToStringCallInput(phi) && !UsedByPhiInstInSameBB(phi) &&
           (HasAppendInstructionUser(phi) || HasSbCtorStrInstructionUser(phi));
}

ArenaVector<Inst *> SimplifyStringBuilder::GetPhiAccumulatedValues(Loop *loop)
{
    // Search loop for all accumulated values
    // Phi accumulated value is an instruction used to store string concatenation result between loop iterations

    instructionsVector_.clear();

    for (auto inst : loop->GetHeader()->PhiInsts()) {
        if (IsPhiAccumulatedValue(inst->CastToPhi())) {
            instructionsVector_.push_back(inst);
        }
    }

    for (auto backEdge : loop->GetBackEdges()) {
        if (backEdge == loop->GetHeader()) {
            // Already processed above
            continue;
        }

        for (auto inst : backEdge->PhiInsts()) {
            if (IsPhiAccumulatedValue(inst->CastToPhi())) {
                instructionsVector_.push_back(inst);
            }
        }
    }

    return instructionsVector_;
}

void SimplifyStringBuilder::StringBuilderUsagesDFS(Inst *inst, Loop *loop, Marker visited)
{
    // Recursively traverse current instruction users, and collect all the StringBuilder usages met

    ASSERT(inst != nullptr);
    if (inst->GetBasicBlock()->GetLoop() != loop || inst->IsMarked(visited)) {
        return;
    }

    inst->SetMarker(visited);

    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsPhi() && !userInst->IsMarked(visited)) {
            StringBuilderUsagesDFS(userInst, loop, visited);
        }

        if (!IsStringBuilderAppend(userInst) && !IsMethodStringBuilderConstructorWithStringArg(userInst)) {
            continue;
        }

        auto appendInstruction = userInst;
        ASSERT(appendInstruction->GetInputsCount() > 1);
        auto instance = appendInstruction->GetDataFlowInput(0);
        if (instance->GetBasicBlock()->GetLoop() == loop) {
            StringBuilderUsage usage {nullptr, nullptr, GetGraph()->GetAllocator()};
            MatchStringBuilderUsage(instance, usage);

            if (usage.toStringCalls.size() != 1) {
                continue;
            }

            if (!usage.toStringCalls[0]->IsMarked(visited)) {
                StringBuilderUsagesDFS(usage.toStringCalls[0], loop, visited);
                usages_.push_back(usage);
            }
        }
    }
}

const ArenaVector<SimplifyStringBuilder::StringBuilderUsage> &SimplifyStringBuilder::GetStringBuilderUsagesPO(
    Inst *accValue)
{
    // Get all the StringBuilder usages under the phi accumulated value instruction data flow graph in post order (PO)

    usages_.clear();

    MarkerHolder usageMarker {GetGraph()};
    Marker visited = usageMarker.GetMarker();

    StringBuilderUsagesDFS(accValue, accValue->GetBasicBlock()->GetLoop(), visited);

    return usages_;
}

bool SimplifyStringBuilder::AllUsersAreVisitedAppendInstructions(Inst *inst, Marker appendInstructionVisited)
{
    bool allUsersVisited = true;
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsSaveState()) {
            continue;
        }
        if (IsCheckCastWithoutUsers(userInst)) {
            continue;
        }
        allUsersVisited &= IsStringBuilderAppend(userInst);
        allUsersVisited &= userInst->IsMarked(appendInstructionVisited);
    }
    return allUsersVisited;
}

Inst *SimplifyStringBuilder::UpdateIntermediateValue(const StringBuilderUsage &usage, Inst *intermediateValue,
                                                     Marker appendInstructionVisited)
{
    // Update intermediate value with toString-call of the current StringBuilder usage, if all its append-call users
    // were already visited

    size_t usersCount = 0;
    bool allUsersVisited = true;

    for (auto &user : intermediateValue->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsSaveState()) {
            continue;
        }

        if (userInst->IsPhi() && !userInst->HasUsers()) {
            continue;
        }

        if (userInst->IsPhi() && !userInst->HasUsers()) {
            continue;
        }

        if (userInst->IsPhi()) {
            ++usersCount;
            allUsersVisited &= AllUsersAreVisitedAppendInstructions(userInst, appendInstructionVisited);
        } else if (IsStringBuilderAppend(userInst)) {
            ++usersCount;
            allUsersVisited &= userInst->IsMarked(appendInstructionVisited);
        } else {
            break;
        }
    }

    ASSERT(usage.toStringCalls.size() == 1);
    if (usersCount == 1) {
        intermediateValue = usage.toStringCalls[0];
    } else if (usersCount > 1 && allUsersVisited) {
        intermediateValue = usage.toStringCalls[0];
        for (auto &user : usage.toStringCalls[0]->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsPhi() && userInst->HasUsers()) {
                intermediateValue = userInst;
                break;
            }
        }
    }

    return intermediateValue;
}

bool SimplifyStringBuilder::MatchTemporaryInstruction(ConcatenationLoopMatch &match,
                                                      ConcatenationLoopMatch::TemporaryInstructions &temp,
                                                      Inst *appendInstruction, const Inst *accValue,
                                                      Marker appendInstructionVisited)
{
    // Arrange single append instruction to one of the two groups (substructures of ConcatenationLoopMatch):
    //  'temp' group - temporary instructions to be erased from loop
    //  'loop' group - append-call instructions to be kept inside loop

    ASSERT(appendInstruction->GetInputsCount() > 1);
    auto appendArg = appendInstruction->GetDataFlowInput(1);
    if ((appendArg->IsPhi() && IsDataFlowInput(appendArg, temp.intermediateValue)) ||
        appendArg == temp.intermediateValue || appendArg == accValue) {
        // Append-call needs to be removed, if its argument is either accumulated value, or intermediate value;
        // or intermediate value is data flow input of argument

        if (temp.appendAccValue == nullptr) {
            temp.appendAccValue = appendInstruction;
        } else {
            // Does not look like string concatenation pattern
            temp.Clear();
            return false;
        }
        appendInstruction->SetMarker(appendInstructionVisited);
    } else {
        if (temp.appendAccValue != nullptr) {
            // First appendee should be acc value
            temp.Clear();
            return false;
        }
        // Keep append-call inside loop otherwise
        match.loop.appendInstructions.push_back(appendInstruction);
    }
    return true;
}

bool SimplifyStringBuilder::ValidateTemporaryStringLengthPhis(const ConcatenationLoopMatch &match,
                                                              const StringBuilderUsage &usage)
{
    /// This avoids the pattern
    //    s += s1 + s2
    //    print(s.length) // from temporary
    //    s += s3 + s4

    for (const auto &chain : usage.toStringLengthChains) {
        if (chain.toStringCallOrPhi != match.accValue) {
            COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "[.length][reject] Hostment to non-acc is not supported";
            return false;
        }
    }
    return true;
}

void SimplifyStringBuilder::MatchTemporaryInstructions(const StringBuilderUsage &usage, ConcatenationLoopMatch &match,
                                                       Inst *accValue, Inst *intermediateValue,
                                                       Marker appendInstructionVisited)
{
    // Split all the instructions of a given StringBuilder usage into groups (substructures of ConcatenationLoopMatch):
    //  'temp' group - temporary instructions to be erased from loop
    //  'loop' group - append-call instructions to be kept inside loop

    ConcatenationLoopMatch::TemporaryInstructions temp {};
    temp.intermediateValue = intermediateValue;
    temp.toStringCall = usage.toStringCalls[0];
    temp.instance = usage.instance;
    temp.ctorCall = usage.ctorCall;

    if (!usage.toStringLengthChains.empty()) {
        if (!ValidateTemporaryStringLengthPhis(match, usage)) {
            match.Clear();
            return;
        }
        std::copy(usage.toStringLengthChains.begin(), usage.toStringLengthChains.end(),
                  std::back_inserter(match.loop.toStringLengthChains));
    }

    for (auto appendInstruction : usage.appendInstructions) {
        bool isMatchStillValid =
            MatchTemporaryInstruction(match, temp, appendInstruction, accValue, appendInstructionVisited);
        if (!isMatchStillValid) {
            // Doesn't look like concatenation pattern. Clean entire match
            match.Clear();
            return;
        }
    }

    if (!temp.IsEmpty()) {
        match.temp.push_back(temp);
    }
}

// CC-OFFNXT(huge_depth) false positive
Inst *SimplifyStringBuilder::MatchHoistableInstructions(const StringBuilderUsage &usage, ConcatenationLoopMatch &match,
                                                        Marker appendInstructionVisited)
{
    // Split all the instructions of a given StringBuilder usage into groups (substructures of ConcatenationLoopMatch):
    //  'preheader' group - instructions to be hoisted to preheader block
    //  'loop' group - append-call instructions to be kept inside loop
    //  'exit' group - instructions to be hoisted to post exit block

    match.block = usage.instance->GetBasicBlock();

    ASSERT(usage.instance->GetInputsCount() > 0);
    match.preheader.instance = usage.instance;
    match.preheader.ctorCall = usage.ctorCall;
    ASSERT(usage.toStringCalls.size() == 1);
    match.exit.toStringCall = usage.toStringCalls[0];
    ASSERT(match.loop.toStringLengthChains.empty());
    match.loop.toStringLengthChains = usage.toStringLengthChains;

    for (auto &user : usage.instance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (userInst->IsSaveState()) {
            continue;
        }

        if (!IsStringBuilderAppend(userInst) && !IsMethodStringBuilderConstructorWithStringArg(userInst)) {
            continue;
        }

        // Check if append-call needs to be hoisted or kept inside loop
        auto appendInstruction = userInst;
        ASSERT(appendInstruction->GetInputsCount() > 1);
        auto appendArg = appendInstruction->GetDataFlowInput(1);
        if (appendArg->IsPhi() && IsPhiAccumulatedValue(appendArg->CastToPhi())) {
            // Append-call needs to be hoisted, if its argument is accumulated value
            auto phiAppendArg = appendArg->CastToPhi();
            auto initialValue =
                phiAppendArg->GetPhiDataflowInput(usage.instance->GetBasicBlock()->GetLoop()->GetPreHeader());
            if (match.initialValue != nullptr || match.accValue != nullptr ||
                match.preheader.appendAccValue != nullptr) {
                // Does not look like string concatenation pattern
                match.Clear();
                break;
            }

            match.initialValue = initialValue;
            match.accValue = phiAppendArg;
            match.preheader.appendAccValue = appendInstruction;

            appendInstruction->SetMarker(appendInstructionVisited);
        } else {
            // Keep append-call inside loop otherwise
            match.loop.appendInstructions.push_back(appendInstruction);
        }
    }

    // Initialize intermediate value with toString-call to be hoisted to post exit block
    return match.exit.toStringCall;
}

const ArenaVector<SimplifyStringBuilder::ConcatenationLoopMatch> &SimplifyStringBuilder::MatchLoopConcatenation(
    Loop *loop)
{
    // Search loop for string concatenation patterns like the following:
    //   >  let str = initial_value: String
    //   >  for (...) {
    //   >      str += a0 + b0 + ...
    //   >      str += a1 + b2 + ...
    //   >      str += str.length
    //   >      ...
    //   >  }
    // And fill ConcatenationLoopMatch structure with instructions from the pattern found

    matches_.clear();

    MarkerHolder appendInstructionMarker {GetGraph()};
    Marker appendInstructionVisited = appendInstructionMarker.GetMarker();

    // Accumulated value (acc_value) is a phi-instruction holding concatenation result between loop iterations, and
    // final result after loop completes
    for (auto accValue : GetPhiAccumulatedValues(loop)) {
        // Intermediate value is an instruction holding concatenation result during loop iteration execution.
        // It is initialized with acc_value, and updated with either toString-calls, or other phi-instructions
        // (depending on the loop control flow and data flow)
        Inst *intermediateValue = accValue;
        ConcatenationLoopMatch match {GetGraph()->GetAllocator()};

        // Get all the StringBuilders used to calculate current accumulated value (in PO)
        auto &usages = GetStringBuilderUsagesPO(accValue);
        // RPO traversal: walk through PO usages backwards
        for (auto usage = usages.rbegin(); usage != usages.rend(); ++usage) {
            if (usage->toStringCalls.size() != 1) {
                continue;  // Unsupported: doesn't look like string concatenation pattern
            }

            if (match.preheader.instance == nullptr) {
                // First StringBuilder instance are set to be hoisted
                intermediateValue = MatchHoistableInstructions(*usage, match, appendInstructionVisited);
            } else {
                // All other StringBuilder instances are set to be removed as temporary
                MatchTemporaryInstructions(*usage, match, accValue, intermediateValue, appendInstructionVisited);
                intermediateValue = UpdateIntermediateValue(*usage, intermediateValue, appendInstructionVisited);
            }

            // MVP<.length>: ignore non-exit induces phis
            if (!ValidateTemporaryStringLengthPhis(match, *usage)) {
                match.Clear();
                break;
            }
        }

        if (IsInstanceHoistable(match) && IsToStringHoistable(match, appendInstructionVisited)) {
            matches_.push_back(match);
        }

        // Reset markers
        if (match.preheader.appendAccValue != nullptr) {
            match.preheader.appendAccValue->ResetMarker(appendInstructionVisited);
        }
        for (auto &temp : match.temp) {
            if (temp.appendAccValue != nullptr) {
                temp.appendAccValue->ResetMarker(appendInstructionVisited);
            }
        }
    }

    return matches_;
}

void SimplifyStringBuilder::OptimizeStringConcatenation(Loop *loop)
{
    // Optimize String Builder concatenation loops

    // Process inner loops first
    for (auto innerLoop : loop->GetInnerLoops()) {
        OptimizeStringConcatenation(innerLoop);
    }

    // Check if basic block for instructions to hoist exist
    // Alternative way maybe to create one
    if (loop->GetPreHeader() == nullptr) {
        return;
    }

    auto preHeaderSaveState = FindPreHeaderSaveState(loop);
    if (preHeaderSaveState == nullptr) {
        return;
    }

    // Check if basic block for instructions to hoist exist
    // Alternative way maybe to create one
    auto postExit = GetLoopPostExit(loop);
    if (postExit == nullptr) {
        return;
    }

    auto postExitSaveState = FindFirstSaveState(postExit);
    ASSERT(postExitSaveState);  // IR Builder must create empty save states for loop exits

    for (auto &match : MatchLoopConcatenation(loop)) {
        ReconnectInstructions(match);
        HoistInstructionsToPreHeader(match, preHeaderSaveState);
        HoistInstructionsToPostExit(match, postExitSaveState);
        Cleanup(match);

        isApplied_ = true;
    }
    Cleanup(loop);
}

IntrinsicInst *SimplifyStringBuilder::CreateIntrinsicStringBuilderAppendStrings(const ConcatenationMatch &match,
                                                                                SaveStateInst *saveState)
{
    auto arg0 = match.appendIntrinsics[0U]->GetInput(1U).GetInst();
    auto arg1 = match.appendIntrinsics[1U]->GetInput(1U).GetInst();

    switch (match.appendCount) {
        case 2U: {
            return CreateIntrinsic(
                GetGraph(), GetGraph()->GetRuntime()->GetStringBuilderAppendStringsIntrinsicId(match.appendCount),
                DataType::REFERENCE,
                {{match.instance, match.instance->GetType()},
                 {arg0, arg0->GetType()},
                 {arg1, arg1->GetType()},
                 {saveState, saveState->GetType()}});
        }
        case 3U: {
            auto arg2 = match.appendIntrinsics[2U]->GetInput(1U).GetInst();

            return CreateIntrinsic(
                GetGraph(), GetGraph()->GetRuntime()->GetStringBuilderAppendStringsIntrinsicId(match.appendCount),
                DataType::REFERENCE,
                {{match.instance, match.instance->GetType()},
                 {arg0, arg0->GetType()},
                 {arg1, arg1->GetType()},
                 {arg2, arg2->GetType()},
                 {saveState, saveState->GetType()}});
        }
        case 4U: {
            auto arg2 = match.appendIntrinsics[2U]->GetInput(1U).GetInst();
            auto arg3 = match.appendIntrinsics[3U]->GetInput(1U).GetInst();

            return CreateIntrinsic(
                GetGraph(), GetGraph()->GetRuntime()->GetStringBuilderAppendStringsIntrinsicId(match.appendCount),
                DataType::REFERENCE,
                {{match.instance, match.instance->GetType()},
                 {arg0, arg0->GetType()},
                 {arg1, arg1->GetType()},
                 {arg2, arg2->GetType()},
                 {arg3, arg3->GetType()},
                 {saveState, saveState->GetType()}});
        }
        default:
            UNREACHABLE();
    }
}

void SimplifyStringBuilder::ReplaceWithAppendIntrinsic(const ConcatenationMatch &match)
{
    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << match.appendCount << " consecutive append string intrinsics found IDs: ";
    for (size_t index = 0; index < match.appendCount; ++index) {
        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "\t" << match.appendIntrinsics[index]->GetId();
    }
    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "for instance id=" << match.instance->GetId() << " ("
                                     << GetOpcodeString(match.instance->GetOpcode()) << "), applying";

    ASSERT(match.appendCount > 0);
    auto lastAppendIntrinsic = match.appendIntrinsics[match.appendCount - 1U];
    auto appendNIntrinsic = CreateIntrinsicStringBuilderAppendStrings(
        match, CopySaveState(GetGraph(), lastAppendIntrinsic->GetSaveState()));

    InsertBeforeWithSaveState(appendNIntrinsic, lastAppendIntrinsic);

    for (size_t index = 0; index < match.appendCount; ++index) {
        auto appendIntrinsic = match.appendIntrinsics[index];
        FixBrokenSaveStates(appendIntrinsic->GetDataFlowInput(1), appendNIntrinsic);
        appendIntrinsic->ClearFlag(inst_flags::NO_DCE);
    }

    COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "Replaced with append" << match.appendCount
                                     << " intrinsic id=" << appendNIntrinsic->GetId();

    isApplied_ = true;
}

bool HasPhiOfStringBuilders(BasicBlock *block)
{
    for (auto phi : block->PhiInsts()) {
        MarkerHolder visited {block->GetGraph()};
        if (HasInputPhiRecursively(phi, visited.GetMarker(),
                                   [](auto &input) { return IsStringBuilderInstance(input.GetInst()); })) {
            return true;
        }
    }
    return false;
}

// CC-OFFNXT(huge_depth) false positive
const SimplifyStringBuilder::StringBuilderCallsMap &SimplifyStringBuilder::CollectStringBuilderCalls(BasicBlock *block)
{
    auto &calls = stringBuilderCalls_;
    calls.clear();

    // Skip blocks with Phi of String Builders
    if (HasPhiOfStringBuilders(block)) {
        return calls;
    }

    auto current = calls.end();
    for (auto inst : block->Insts()) {
        if (inst->IsSaveState() || inst->IsCheck()) {
            continue;
        }

        for (size_t index = 0; index < inst->GetInputsCount(); ++index) {
            auto instance = inst->GetDataFlowInput(index);
            if (!IsStringBuilderInstance(instance)) {
                continue;
            }

            if (current != calls.end() && current->first != instance) {
                current = calls.find(instance);
            }
            if (current == calls.end()) {
                current = calls.emplace(instance, InstVector {GetGraph()->GetLocalAllocator()->Adapter()}).first;
            }
            current->second.push_back(inst);
        }
    }
    return calls;
}

size_t CountStringBuilderAppendString(const InstVector &calls, size_t from)
{
    auto to = std::min(from + SB_APPEND_STRING_MAX_ARGS, calls.size());
    for (auto index = from; index < to; ++index) {
        if (IsIntrinsicStringBuilderAppendString(calls[index])) {
            continue;
        }
        return index;
    }
    return to;
}

void SimplifyStringBuilder::ReplaceWithAppendIntrinsic(Inst *instance, const InstVector &calls, size_t from, size_t to)
{
    auto appendCount = to - from;
    ASSERT(0 < appendCount && appendCount <= SB_APPEND_STRING_MAX_ARGS);
    if (appendCount == 1) {
        COMPILER_LOG(DEBUG, SIMPLIFY_SB) << "One consecutive append string intrinsic found id=" << calls[from]->GetId()
                                         << " for instance id=" << instance->GetId() << " ("
                                         << GetOpcodeString(instance->GetOpcode()) << "), skipping";
    } else {
        ConcatenationMatch match {instance, nullptr, nullptr, appendCount};
        for (size_t index = 0; index < appendCount; ++index) {
            match.appendIntrinsics[index] = calls[from + index]->CastToIntrinsic();
        }
        ReplaceWithAppendIntrinsic(match);
    }
}

void SimplifyStringBuilder::OptimizeStringBuilderAppendChain(BasicBlock *block)
{
    // StringBuilder::append(s0, s1, ...) implemented with Irtoc functions with number of arguments more than 4, and
    // requires pre/post-write barriers to be supported. AARCH32 does not support neither barriers, nor so much
    // arguments in Irtoc functions.
    if (!GetGraph()->SupportsIrtocBarriers()) {
        return;
    }

    isApplied_ |= BreakStringBuilderAppendChains(block);

    for (auto &[instance, calls] : CollectStringBuilderCalls(block)) {
        for (size_t from = 0; from < calls.size();) {
            if (!IsIntrinsicStringBuilderAppendString(calls[from])) {
                ++from;
                continue;
            }

            size_t to = CountStringBuilderAppendString(calls, from);
            ReplaceWithAppendIntrinsic(instance, calls, from, to);

            ASSERT(from < to);
            from = to;
        }
    }
}

// CC-OFFNXT(huge_depth) false positive
void SimplifyStringBuilder::CollectStringBuilderFirstCalls(BasicBlock *block)
{
    Inst *instance = nullptr;
    auto calls = stringBuilderFirstLastCalls_.end();
    for (auto inst : block->Insts()) {
        if (IsStringBuilderInstance(inst)) {
            instance = inst;
            calls = stringBuilderFirstLastCalls_.find(instance);
            if (calls == stringBuilderFirstLastCalls_.end()) {
                calls = stringBuilderFirstLastCalls_.emplace(instance, InstPair {}).first;
            }
        }

        if (calls != stringBuilderFirstLastCalls_.end() && IsStringBuilderAppend(inst) &&
            inst->GetDataFlowInput(0) == instance) {
            auto &firstCall = calls->second.first;
            if (firstCall == nullptr) {
                firstCall = inst;
            }
        }
        if (calls != stringBuilderFirstLastCalls_.end() && IsMethodStringBuilderConstructorWithStringArg(inst) &&
            inst->GetDataFlowInput(0) == instance) {
            auto &firstCall = calls->second.first;
            if (firstCall == nullptr) {
                firstCall = inst;
            }
        }
    }
}

// CC-OFFNXT(huge_depth) false positive
void SimplifyStringBuilder::CollectStringBuilderLastCalls(BasicBlock *block)
{
    for (auto inst : block->InstsReverse()) {
        if (inst->IsSaveState() || inst->IsCheck()) {
            continue;
        }

        if (!IsStringBuilderToString(inst)) {
            continue;
        }

        auto inputInst = inst->GetDataFlowInput(0);
        if (inputInst->IsSaveState()) {
            continue;
        }

        auto instance = inputInst;
        if (!IsStringBuilderInstance(instance)) {
            continue;
        }

        auto calls = stringBuilderFirstLastCalls_.find(instance);
        if (calls == stringBuilderFirstLastCalls_.end()) {
            calls = stringBuilderFirstLastCalls_.emplace(instance, InstPair {}).first;
        }
        auto &lastCall = calls->second.second;
        if (lastCall == nullptr) {
            lastCall = inst;
        }
    }
}

void SimplifyStringBuilder::CollectStringBuilderFirstLastCalls()
{
    // For each SB in graph, find its first (append) call and last (toString) call
    stringBuilderFirstLastCalls_.clear();

    // Traverse CFG in RPO
    for (auto block : GetGraph()->GetBlocksRPO()) {
        isApplied_ |= BreakStringBuilderAppendChains(block);
        CollectStringBuilderFirstCalls(block);
        CollectStringBuilderLastCalls(block);
    }
}

bool SimplifyStringBuilder::CanMergeStringBuilders(Inst *instance, const InstPair &instanceCalls, Inst *inputInstance)
{
    if (instance == inputInstance) {
        return false;  // Skip instance.append(instance.toString()) case
    }

    auto inputInstanceCalls = stringBuilderFirstLastCalls_.find(inputInstance);
    if (inputInstanceCalls == stringBuilderFirstLastCalls_.end()) {
        return false;
    }

    auto &[inputInstanceFirstAppendCall, inputInstanceLastToStringCall] = inputInstanceCalls->second;
    if (inputInstanceFirstAppendCall == nullptr || inputInstanceLastToStringCall == nullptr) {
        return false;  // Unsupported case: doesn't look like concatenation pattern
    }

    for (auto &user : instance->GetUsers()) {
        auto userInst = user.GetInst();
        if (IsMethodStringBuilderConstructorWithCharArrayArg(userInst)) {
            // Does not look like string concatenation pattern
            return false;
        }
    }

    auto instanceFirstCall = instanceCalls.first;

    if (instance->GetBasicBlock() != instanceFirstCall->GetBasicBlock()) {
        // Does not look like string concatenation pattern
        return false;
    }

    // Check if 'inputInstance' toString call comes after any 'inputInstance' append call
    for (auto &user : inputInstance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (!IsStringBuilderAppend(userInst) && !IsMethodStringBuilderConstructorWithStringArg(userInst)) {
            continue;
        }

        if (inputInstanceLastToStringCall->IsDominate(userInst)) {
            return false;
        }
    }

    // Check if 'inputInstance' toString call has single append call user
    if (CountUsers(inputInstanceLastToStringCall, [instanceFirstCall](auto &user) {
            auto userInst = user.GetInst();
            auto isAppend = IsStringBuilderAppend(userInst);
            auto isCtorStr = IsMethodStringBuilderConstructorWithStringArg(userInst);
            return (isAppend || isCtorStr) && userInst != instanceFirstCall;
        }) > 0) {
        return false;
    }

    // Check if all 'inputInstance' calls comes before any 'instance' call
    return inputInstanceLastToStringCall->IsDominate(instanceFirstCall);
}

void SimplifyStringBuilder::Cleanup(Inst *instance, Inst *instanceFirstAppendCall, Inst *inputInstanceToStringCall)
{
    // Mark 'instance' constructor as dead
    for (auto &user : instance->GetUsers()) {
        auto userInst = user.GetInst();
        if (!IsMethodStringBuilderDefaultConstructor(userInst)) {
            continue;
        }
        userInst->ClearFlag(inst_flags::NO_DCE);
    }

    // Mark 'instance' append call as dead
    instanceFirstAppendCall->ClearFlag(inst_flags::NO_DCE);

    if (!GetGraph()->IsBytecodeOptimizer()) {
        // Mark 'inputInstance' toString call as dead with it's users
        CleanupInstruction(inputInstanceToStringCall);
    }

    // Mark 'instance' itself as dead
    instance->ClearFlag(inst_flags::NO_DCE);

    // Remove instructions marked dead from save states
    RemoveFromSaveStateInputs(instance);
    RemoveFromSaveStateInputs(instanceFirstAppendCall);

    // Remove inputInstance.toString() call only if it is not used anywhere else
    if (!HasUser(inputInstanceToStringCall, [instanceFirstAppendCall](auto &user) {
            auto userInst = user.GetInst();
            auto isSaveState = userInst->IsSaveState();
            auto isAppend = userInst == instanceFirstAppendCall;
            auto isCheckCast = IsCheckCastWithoutUsers(userInst);
            return !isSaveState && !isAppend && !isCheckCast;
        })) {
        if (GetGraph()->IsBytecodeOptimizer()) {
            // Mark 'inputInstance' toString call as dead with it's users
            CleanupInstruction(inputInstanceToStringCall);
        }
        RemoveFromSaveStateInputs(inputInstanceToStringCall);
    }
}

void SimplifyStringBuilder::CleanupInstruction(Inst *inst)
{
    inst->ClearFlag(inst_flags::NO_DCE);
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (!IsCheckCastWithoutUsers(userInst)) {
            continue;
        }
        userInst->ClearFlag(inst_flags::NO_DCE);
    }
}

void SimplifyStringBuilder::FixBrokenSaveStatesForStringBuilderCalls(Inst *instance)
{
    for (auto &user : instance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (userInst->IsSaveState()) {
            continue;
        }

        if (IsStringBuilderAppend(userInst)) {
            ASSERT(userInst->GetInputsCount() > 1U);
            auto appendArg = userInst->GetDataFlowInput(1U);
            FixBrokenSaveStates(appendArg, userInst);
        }

        FixBrokenSaveStates(instance, userInst);
    }
}

void SimplifyStringBuilder::CollectStringBuilderChainCalls(Inst *instance, Inst *inputInstance)
{
    ASSERT(stringBuilderCalls_.find(inputInstance) == stringBuilderCalls_.end());

    // Check if 'instance' already in calls map
    auto calls = stringBuilderCalls_.find(instance);
    if (calls == stringBuilderCalls_.end()) {
        // If not, add 'inputInstance' with empty instructions vector to a map
        calls = stringBuilderCalls_
                    .insert(std::make_pair(inputInstance, InstVector {GetGraph()->GetAllocator()->Adapter()}))
                    .first;
    } else {
        // If yes, do the following:
        // 1. Add 'inputInstance' with instructions vector taken from 'instance'
        // 2. Remove 'instance' from a map
        calls = stringBuilderCalls_.insert(std::make_pair(inputInstance, std::move(calls->second))).first;
        stringBuilderCalls_.erase(instance);
    }

    // Map 'instance' users to 'inputInstance'
    for (auto &user : instance->GetUsers()) {
        auto userInst = user.GetInst();
        // Skip save states
        if (userInst->IsSaveState()) {
            continue;
        }

        // Skip StringBuilder constructors
        if (IsMethodStringBuilderDefaultConstructor(userInst) ||
            IsMethodStringBuilderConstructorWithStringArg(userInst) ||
            IsMethodStringBuilderConstructorWithCharArrayArg(userInst)) {
            continue;
        }

        calls->second.push_back(userInst);
    }
}

SimplifyStringBuilder::StringBuilderCallsMap &SimplifyStringBuilder::CollectStringBuilderChainCalls()
{
    CollectStringBuilderFirstLastCalls();
    stringBuilderCalls_.clear();

    // Traverse CFG in PO
    auto &blocksRPO = GetGraph()->GetBlocksRPO();
    for (auto block = blocksRPO.rbegin(); block != blocksRPO.rend(); ++block) {
        // Traverse 'block' backwards
        for (auto instance : (*block)->InstsReverse()) {
            if (!IsStringBuilderInstance(instance)) {
                continue;
            }

            auto instanceCalls = stringBuilderFirstLastCalls_.find(instance);
            if (instanceCalls == stringBuilderFirstLastCalls_.end()) {
                continue;
            }

            auto &instanceFirstLastCalls = instanceCalls->second;
            auto &[instanceFirstAppendCall, instanceLastToStringCall] = instanceFirstLastCalls;
            if (instanceFirstAppendCall == nullptr || instanceLastToStringCall == nullptr) {
                continue;  // Unsupported case: doesn't look like concatenation pattern
            }

            if (!IsStringBuilderAppend(instanceFirstAppendCall) &&
                !IsMethodStringBuilderConstructorWithStringArg(instanceFirstAppendCall)) {
                continue;
            }

            auto inputInstanceToStringCall = instanceFirstAppendCall->GetDataFlowInput(1);
            if (!IsStringBuilderToString(inputInstanceToStringCall)) {
                continue;
            }

            if (instanceFirstAppendCall->GetDataFlowInput(1) != inputInstanceToStringCall) {
                continue;  // Allow only cases like: instance.append(inputInstace.toString())
            }

            ASSERT(inputInstanceToStringCall->GetInputsCount() > 1);
            auto inputInstance = inputInstanceToStringCall->GetDataFlowInput(0);
            if (!IsStringBuilderInstance(inputInstance)) {
                continue;
            }

            if (!CanMergeStringBuilders(instance, instanceFirstLastCalls, inputInstance)) {
                continue;
            }

            Cleanup(instance, instanceFirstAppendCall, inputInstanceToStringCall);
            CollectStringBuilderChainCalls(instance, inputInstance);
        }
    }

    return stringBuilderCalls_;
}

void SimplifyStringBuilder::OptimizeStringBuilderChain()
{
    /*
        Merges consecutive String Builders into one String Builder if possible
        Example of string concatenation (TS):
            let a: String = ...
            a += ...;
            a += ...;
            let result = a;

        Frontend generates the following SB-equivalent code for the example above:
            let inputInstance = new StringBuilder()
            inputInstance.append(...)                   // append calls for 'inputInstance'
            ...
            let instance = new StringBuilder()
            instance.append(inputInstance.toString())   // first call to append for 'instance'
                                                        // 'inputInstance' not used beyond this point
            instance.append(...)                        // append calls for 'instance'
            ...
            let result = instance.toString()

        The algorithm transforms it into:
            let inputInstance = new StringBuilder()
            inputInstance.append(...)                   // append calls for 'inputInstance'
            ...
            inputInstance.append(...)                   // append calls for 'instance' replaced by 'inputInstance'
            ...
            let result = inputInstance.toString()

        Note: arbirtary length of String Builder chain supported

        The algorithm works on SBs, but not on strings themselves, thus the following limitations assumed:
            1) all SBs must be created via default constructor,
            2) call to append for 'instance' SB with the input of inputInstance.toString() must be the first,
            3) 'inputInstance' must not be used after instance.append(inputInstance.toString()) call.
        Those limitations fully match SB patterns of string concatenation.
    */

    for (auto &instanceCalls : CollectStringBuilderChainCalls()) {
        auto inputInstance = instanceCalls.first;
        auto &calls = instanceCalls.second;

        for (auto call : calls) {
            // Switch call to new instance only if it is not marked for removal
            if (call->GetFlag(inst_flags::NO_DCE)) {
                call->SetInput(0U, inputInstance);
            }
        }

        FixBrokenSaveStatesForStringBuilderCalls(inputInstance);

        isApplied_ = true;
    }
}

void SimplifyStringBuilder::OptimizeStringBuilderStringLength(BasicBlock *block)
{
    /*
        Removes extra calls for get-stringLength
        Example of get-stringLength calls:
            let sb = new StringBuilder();
            let len0 = sb.stringLength;
            ...
            let len1 = sb.stringLength;

        If stringLength calls refer to the same object and if the object was not modified between stringLength calls,
        we remove all unnecessary stringLength calls.

        Calls are not removed in the following cases:
            let sb = new StringBuilder();
            let len0 = sb.stringLength;
            ...
            sb.append(...) // or any call modifying the StringBuilder object
            ...
            let len1 = sb.stringLength;
    */

    sbStringLengthCalls_.clear();

    for (Inst *curInst : block->Insts()) {
        if (IsMethodStringBuilderGetStringLength(curInst)) {
            auto currStringLength = curInst;
            auto instance = currStringLength->GetDataFlowInput(0);
            auto prevStringLength = sbStringLengthCalls_.find(instance);
            if (prevStringLength != sbStringLengthCalls_.end()) {
                ASSERT(prevStringLength->second->IsDominate(currStringLength));
                currStringLength->ReplaceUsers(prevStringLength->second);
                currStringLength->ClearFlag(inst_flags::NO_DCE);

                isApplied_ = true;

                COMPILER_LOG(DEBUG, SIMPLIFY_SB)
                    << "Remove StringBuilder stringLength()-call (id=" << curInst->GetId() << ")";
            } else {
                sbStringLengthCalls_.emplace(instance, currStringLength);
            }
            continue;
        }

        curInst = SkipSingleUserCheckInstruction(curInst);
        if (IsStringBuilderToString(curInst) || curInst->IsSaveState() ||
            IsMethodStringBuilderGetStringLength(curInst)) {
            continue;
        }

        // Remove stringLength call from the sbStringLengthCalls_ Map if another instruction using the same SB instance
        // is found after the stringLength call (except for the toString call)
        for (size_t index = 0; index < curInst->GetInputsCount(); ++index) {
            auto input = curInst->GetDataFlowInput(index);

            auto prevStringLength = sbStringLengthCalls_.find(input);
            if (prevStringLength != sbStringLengthCalls_.end()) {
                sbStringLengthCalls_.erase(prevStringLength);
            }
        }
    }
}

}  // namespace ark::compiler
