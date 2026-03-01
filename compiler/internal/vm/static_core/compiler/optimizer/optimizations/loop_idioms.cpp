/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/optimizations/loop_idioms.h"

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ARRAYMOVE(level) COMPILER_LOG(level, LOOP_TRANSFORM) << "[Arraymove] "

namespace ark::compiler {
bool LoopIdioms::RunImpl()
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        // Supported idioms and intrinsics emitted for them could not be encoded on Arm32.
        return false;
    }
    GetGraph()->RunPass<LoopAnalyzer>();
    RunLoopsVisitor();
    return isApplied_;
}

void LoopIdioms::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool LoopIdioms::TransformLoop(Loop *loop)
{
    if (loop->GetInnerLoops().empty()) {
        if (TryTransformArrayInitIdiom(loop) || TryTransformArrayMoveIdiom(loop)) {
            isApplied_ = true;
        }
    }
    return false;
}

StoreInst *FindStoreForArrayInit(BasicBlock *block)
{
    StoreInst *store {nullptr};
    for (auto inst : block->Insts()) {
        if (inst->GetOpcode() != Opcode::StoreArray) {
            continue;
        }
        if (store != nullptr) {
            return nullptr;
        }
        store = inst->CastToStoreArray();
    }
    // should be a loop invariant
    if (store != nullptr && store->GetStoredValue()->GetBasicBlock()->GetLoop() == block->GetLoop()) {
        return nullptr;
    }
    return store;
}

Inst *ExtractArrayInitInitialIndexValue(PhiInst *index)
{
    auto block = index->GetBasicBlock();
    BasicBlock *pred = block->GetPredsBlocks().front();
    if (pred == block) {
        pred = block->GetPredsBlocks().back();
    }
    return index->GetPhiInput(pred);
}

bool AllUsesWithinLoop(Inst *inst, const Loop *loop)
{
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetBasicBlock()->GetLoop() != loop) {
            return false;
        }
    }
    return true;
}

bool CanReplaceLoop(Loop *loop, Marker marker)
{
    ASSERT(loop->GetBlocks().size() == 1);
    auto block = loop->GetHeader();
    for (auto inst : block->AllInsts()) {
        if (inst->IsMarked(marker)) {
            continue;
        }
        auto opcode = inst->GetOpcode();
        if (opcode != Opcode::NOP && opcode != Opcode::SaveState && opcode != Opcode::SafePoint) {
            return false;
        }
    }
    return true;
}

bool IsLoopContainsArrayInitIdiom(StoreInst *store, Loop *loop, CountableLoopInfo &loopInfo)
{
    auto storeIdx = store->GetIndex();

    return loopInfo.constStep == 1UL && loopInfo.index == storeIdx && loopInfo.normalizedCc == ConditionCode::CC_LT &&
           AllUsesWithinLoop(storeIdx, loop) && AllUsesWithinLoop(loopInfo.update, loop) &&
           AllUsesWithinLoop(loopInfo.ifImm->GetInput(0).GetInst(), loop);
}

bool IsLoopContainsArrayMoveIdiom(StoreInst *store, Loop *loop, CountableLoopInfo &loopInfo)
{
    auto storeIdx = store->GetIndex();

    return loopInfo.constStep == 1UL && loopInfo.index == storeIdx && loopInfo.normalizedCc == ConditionCode::CC_LT &&
           AllUsesWithinLoop(storeIdx, loop) && AllUsesWithinLoop(loopInfo.update, loop) &&
           AllUsesWithinLoop(loopInfo.ifImm->GetInput(0).GetInst(), loop);
}

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ArrayMoveParser : private CountableLoopParser, private MarkerHolder {
public:
    explicit ArrayMoveParser(Loop *loop) : CountableLoopParser(*loop), MarkerHolder(loop->GetHeader()->GetGraph()) {}

    bool Parse()
    {
        // Check loop shape:
        if (!CountableLoopParser::Parse() || loopInfo_.constStep != 1) {
            LOG_ARRAYMOVE(DEBUG) << "Loop shape doesn't match";
            return false;
        }

        if (!FindStoreLoad() || !CheckOffsets() || !CheckLoopIsolation()) {
            LOG_ARRAYMOVE(DEBUG) << "Dataflow pattern doesn't match";
            return false;
        }

        return true;
    }

    bool FindStoreLoad()
    {
        for (auto inst : loop_.GetHeader()->Insts()) {
            if (inst->GetOpcode() != Opcode::StoreArray) {
                continue;
            }
            auto store = inst->CastToStoreArray();
            auto storedValue = store->GetStoredValue();
            if (storedValue->GetOpcode() != Opcode::LoadArray || storedValue->GetBasicBlock() != loop_.GetHeader()) {
                return false;
            }
            store_.inst = store;
            load_.inst = storedValue->CastToLoadArray();
            return DataType::GetTypeSize(store_.inst->GetType(), GetGraph()->GetArch()) ==
                   DataType::GetTypeSize(load_.inst->GetType(), GetGraph()->GetArch());
        }
        return false;
    }

    // Check and extract `Add/Sub(loopIndex, loopInvariant)` combination:
    template <typename T>
    bool ExtractInductionVariableOffset(T *desc)
    {
        auto inductionVar = desc->inst->GetIndex();
        if (inductionVar == loopInfo_.index) {
            desc->idxOffset = inductionVar->GetBasicBlock()->GetGraph()->FindOrCreateConstant(0);
            return true;
        }

        auto chkInputs = [inductionVar, this](size_t i0, size_t i1) {
            return (inductionVar->GetInput(i0).GetInst() == loopInfo_.index) &&
                   (inductionVar->GetInput(i1).GetInst()->GetBasicBlock()->GetLoop() != &loop_);
        };

        if (inductionVar->GetOpcode() == Opcode::Add) {
            for (size_t i = 0; i < 2U; i++) {
                if (chkInputs(i, 1 - i)) {
                    desc->idxOffset = inductionVar->GetInput(1 - i).GetInst();
                    return true;
                }
            }
        }

        if (inductionVar->GetOpcode() == Opcode::Sub) {
            if (chkInputs(0, 1)) {
                desc->idxOffset = inductionVar->GetInput(1).GetInst();
                desc->idxOffsetNegate = true;
                return true;
            }
        }
        return false;
    }

    bool CheckOffsets()
    {
        if (!ExtractInductionVariableOffset(&store_)) {
            return false;
        }
        if (!ExtractInductionVariableOffset(&load_)) {
            return false;
        }
        ASSERT(store_.idxOffset->GetBasicBlock()->GetLoop() != &loop_);
        ASSERT(load_.idxOffset->GetBasicBlock()->GetLoop() != &loop_);
        return true;
    }

    bool CheckLoopIsolation()
    {
        auto compare = loopInfo_.ifImm->GetInput(0).GetInst();
        ASSERT(compare->GetOpcode() == Opcode::Compare);
        std::array<Inst *, 8U> matched = {
            loopInfo_.ifImm,        compare,     loopInfo_.update,       loopInfo_.index, load_.inst,
            load_.inst->GetIndex(), store_.inst, store_.inst->GetIndex()};

        for (Inst *inst : matched) {
            ASSERT(inst->GetBasicBlock()->GetLoop() == &loop_);
            if (!AllUsesWithinLoop(inst, &loop_)) {
                LOG_ARRAYMOVE(DEBUG) << "Inst '" << inst->GetId() << ".' used outside";
                return false;
            }
            inst->SetMarker(GetMarker());
        }

        for (auto inst : loop_.GetHeader()->AllInsts()) {
            if (inst->IsMarked(GetMarker())) {
                continue;
            }
            if ((inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) ||
                inst->GetOpcode() == Opcode::ReturnInlined) {
                continue;
            }
            auto opcode = inst->GetOpcode();
            if (opcode == Opcode::NOP || opcode == Opcode::SaveState || opcode == Opcode::SafePoint) {
                continue;
            }
            LOG_ARRAYMOVE(DEBUG) << "Inst '" << inst->GetId() << ".' breaks pattern";
            return false;
        }
        return true;
    }

    template <RuntimeInterface::IntrinsicId ID = RuntimeInterface::IntrinsicId::INVALID>
    IntrinsicInst *CreateIntrinsic()
    {
        if constexpr (ID == RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY) {
            return GetGraph()->CreateInstIntrinsic(DataType::VOID, store_.inst->GetPc(), ID);
        } else {
            auto id = RuntimeInterface::IntrinsicId::INVALID;
            switch (store_.inst->GetType()) {
                case DataType::BOOL:
                case DataType::INT8:
                case DataType::UINT8:
                    id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_1_BYTE;
                    break;
                case DataType::INT16:
                case DataType::UINT16:
                    id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_2_BYTE;
                    break;
                case DataType::INT32:
                case DataType::UINT32:
                case DataType::FLOAT32:
                    id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_4_BYTE;
                    break;
                case DataType::INT64:
                case DataType::UINT64:
                case DataType::FLOAT64:
                    id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_8_BYTE;
                    break;
                default:
#include "loop_idioms_memmove_intrinsic_id_switch_case.inl"

                    if (id == RuntimeInterface::IntrinsicId::INVALID) {
                        LOG_ARRAYMOVE(DEBUG) << "Store has unsupported type " << store_.inst->GetType();
                        return nullptr;
                    }
            }
            return GetGraph()->CreateInstIntrinsic(DataType::VOID, store_.inst->GetPc(), id);
        }
        UNREACHABLE();
    }

    bool TryReplaceLoop()
    {
        auto aliasType = CheckRefAlias();
        if (aliasType == NO_ALIAS) {
            return ReplaceLoopCompletely();
        }
        // Currently we don't support overlapping regions but in fact whether overlapping regions are supported
        // depends on the certain intrinsic implementation what is provided by the corresponding plugin.
        LOG_ARRAYMOVE(DEBUG) << "Overlapping regions are not supported";
        return false;
    }

    AliasType CheckRefAlias()
    {
        auto src = load_.inst->GetDataFlowInput(0);
        auto dst = store_.inst->GetDataFlowInput(0);
        auto &aa = GetGraph()->GetValidAnalysis<AliasAnalysis>();
        auto aliasType = aa.CheckRefAlias(src, dst);
        auto checkDstNoAlias = [dst] { return dst->GetOpcode() == Opcode::NewArray; };
        auto checkSrcNoAlias = [src, dst] {
            // When both src and dst are LoadObject from the same array, AA returns MUST_ALIAS.
            // The situation where dst is stored into src and src is then being loaded, what makes the alias "in place",
            // is impossible when src dominates dst.
            auto *graph = src->GetBasicBlock()->GetGraph();
            if (!graph->IsAnalysisValid<DominatorsTree>()) {
                graph->RunPass<DominatorsTree>();
            }
            return src->GetOpcode() == Opcode::Parameter ||
                   (src->GetOpcode() == Opcode::LoadObject && src->IsDominate(dst));
        };
        if (aliasType == MAY_ALIAS && checkDstNoAlias() && checkSrcNoAlias()) {
            aliasType = NO_ALIAS;
        }
        return aliasType;
    }

    template <typename T>
    void NegateIfNeeded(T *desc, BasicBlock *b)
    {
        if (desc->idxOffsetNegate) {
            auto offset = desc->idxOffset;
            auto neg = GetGraph()->CreateInstNeg(offset->GetType(), offset->GetPc(), offset);
            ASSERT(b != nullptr);
            b->AppendInst(neg);
            desc->idxOffset = neg;
        }
    }

    void PatchRange(Inst **srcStart, Inst **srcEnd, BasicBlock *newB)
    {
        auto one = GetGraph()->FindOrCreateConstant(1);
        switch (loopInfo_.normalizedCc) {
            case CC_GE: {
                // Transform interval [e,s] to [e, s + 1)
                ASSERT(!loopInfo_.isInc);
                std::swap(*srcStart, *srcEnd);
                auto add = GetGraph()->CreateInstAdd((*srcEnd)->GetType(), (*srcEnd)->GetPc(), (*srcEnd), one);
                newB->AppendInst(add);
                *srcEnd = add;
                break;
            }
            case CC_GT: {
                //  Transform interval (e,s] to [e + 1, s + 1)
                ASSERT(!loopInfo_.isInc);
                std::swap(*srcStart, *srcEnd);
                auto addStart =
                    GetGraph()->CreateInstAdd((*srcStart)->GetType(), (*srcStart)->GetPc(), (*srcStart), one);
                auto addEnd = GetGraph()->CreateInstAdd((*srcEnd)->GetType(), (*srcEnd)->GetPc(), (*srcEnd), one);
                newB->AppendInst(addStart);
                newB->AppendInst(addEnd);
                *srcStart = addStart;
                *srcEnd = addEnd;
                break;
            }
            case CC_LE: {
                //  Transform interval [s,e] to [s, e + 1)
                ASSERT(loopInfo_.isInc);
                auto add = GetGraph()->CreateInstAdd((*srcEnd)->GetType(), (*srcEnd)->GetPc(), (*srcEnd), one);
                newB->AppendInst(add);
                *srcEnd = add;
                break;
            }
            case CC_LT: {
                // [s,e) => s' = s, e' = e;
                ASSERT(loopInfo_.isInc);
                break;
            }
            default:
                UNREACHABLE();
        }
    }

    bool ReplaceLoopCompletely()
    {
        auto mmove = CreateIntrinsic();
        if (mmove == nullptr) {
            return false;
        }
        auto newB = GetGraph()->CreateEmptyBlock(store_.inst->GetPc());
        // 0. Initially, create the new BB outside any loop.
        newB->SetLoop(GetGraph()->GetRootLoop());
        // 1. Normalize 'load_index/store_index == Sub(idx, c)' -> 'load_index/store_index == Add(idx, Neg(c))':
        NegateIfNeeded(&load_, newB);
        NegateIfNeeded(&store_, newB);
        // 2. Calculate start/end value of load_index:
        //    i.e. define [s,e) or [s,e] or [e,s] or (e,s]
        Inst *srcStart =
            GetGraph()->CreateInstAdd(DataType::INT32, load_.inst->GetPc(), loopInfo_.init, load_.idxOffset);
        newB->AppendInst(srcStart);
        Inst *srcEnd = GetGraph()->CreateInstAdd(DataType::INT32, load_.inst->GetPc(), loopInfo_.test, load_.idxOffset);
        newB->AppendInst(srcEnd);
        // 3. Handle cases of 'srcStart > srcEnd' and patch range enclosing.
        //    i.e. normalize [s,e), [s,e], [e,s], (e,s] -> [s',e').
        PatchRange(&srcStart, &srcEnd, newB);

        // 4. Calculate 'dstStart' based on 'srcStart' and idxOffsets' diff:
        auto offsDiff =
            GetGraph()->CreateInstSub(DataType::INT32, store_.inst->GetPc(), store_.idxOffset, load_.idxOffset);
        newB->AppendInst(offsDiff);
        auto dstStart = GetGraph()->CreateInstAdd(DataType::INT32, store_.inst->GetPc(), srcStart, offsDiff);
        newB->AppendInst(dstStart);

        newB->AppendInst(mmove);
        mmove->SetInputs(GetGraph()->GetAllocator(), {{load_.inst->GetArray(), DataType::REFERENCE},
                                                      {store_.inst->GetArray(), DataType::REFERENCE},
                                                      {dstStart, dstStart->GetType()},
                                                      {srcStart, srcStart->GetType()},
                                                      {srcEnd, srcEnd->GetType()}});

        // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
        switch (mmove->GetIntrinsicId()) {
#include "loop_idioms_customize_memmove_intrinsic_switch_case.inl"

            default:
                mmove->ClearFlag(inst_flags::Flags::REQUIRE_STATE);
                mmove->ClearFlag(inst_flags::Flags::RUNTIME_CALL);
                mmove->ClearFlag(inst_flags::Flags::CAN_THROW);
        }

        auto header = loop_.GetHeader();
        auto preheader = loop_.GetPreHeader();
        auto outside = header->GetSuccessor(0) != header ? header->GetSuccessor(0) : header->GetSuccessor(1);
        preheader->SetNextLoop(nullptr);
        preheader->ReplaceSucc(header, newB);
        outside->ReplacePred(header, newB);
        header->Clear();
        GetGraph()->EraseBlock(header);
        LOG_ARRAYMOVE(DEBUG) << "Replaced loop " << loop_.GetId();

        return true;
    }

    Graph *GetGraph()
    {
        return loop_.GetHeader()->GetGraph();
    }

private:
    template <typename T>
    struct ArrayAccessDescr {
        T *inst {};
        Inst *idxOffset {};
        bool idxOffsetNegate {false};
    };

    ArrayAccessDescr<LoadInst> load_;
    ArrayAccessDescr<StoreInst> store_;
};

bool LoopIdioms::TryTransformArrayMoveIdiom(Loop *loop)
{
    ASSERT(loop->GetInnerLoops().empty());
    if (loop->GetBlocks().size() != 1) {
        return false;
    }

    LOG_ARRAYMOVE(DEBUG) << "Check loop " << loop->GetId();
    ArrayMoveParser arrayMoveParser(loop);
    if (!arrayMoveParser.Parse()) {
        return false;
    }
    LOG_ARRAYMOVE(DEBUG) << "Found in loop " << loop->GetId();

    return arrayMoveParser.TryReplaceLoop();
}

bool LoopIdioms::TryTransformArrayInitIdiom(Loop *loop)
{
    if (loop->GetBlocks().size() != 1) {
        return false;
    }
    ASSERT(loop->GetInnerLoops().empty());

    auto store = FindStoreForArrayInit(loop->GetHeader());
    if (store == nullptr) {
        return false;
    }

    auto loopInfoOpt = CountableLoopParser {*loop}.Parse();
    if (!loopInfoOpt.has_value()) {
        return false;
    }
    auto loopInfo = *loopInfoOpt;
    if (!IsLoopContainsArrayInitIdiom(store, loop, loopInfo)) {
        return false;
    }
    ASSERT(loopInfo.isInc);

    MarkerHolder holder {GetGraph()};
    Marker marker = holder.GetMarker();
    store->SetMarker(marker);
    loopInfo.update->SetMarker(marker);
    loopInfo.index->SetMarker(marker);
    loopInfo.ifImm->SetMarker(marker);
    loopInfo.ifImm->GetInput(0).GetInst()->SetMarker(marker);

    if (!CanReplaceLoop(loop, marker)) {
        return false;
    }

    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Array init idiom found in loop: " << loop->GetId()
                                        << "\n\tarray: " << *store->GetArray()
                                        << "\n\tvalue: " << *store->GetStoredValue()
                                        << "\n\tinitial index: " << *loopInfo.init << "\n\ttest: " << *loopInfo.test
                                        << "\n\tupdate: " << *loopInfo.update << "\n\tstep: " << loopInfo.constStep
                                        << "\n\tindex: " << *loopInfo.index;

    bool alwaysJump = false;
    if (loopInfo.init->IsConst() && loopInfo.test->IsConst()) {
        auto iterations =
            loopInfo.test->CastToConstant()->GetIntValue() - loopInfo.init->CastToConstant()->GetIntValue();
        if (iterations <= ITERATIONS_THRESHOLD) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                << "Loop will have " << iterations << " iterations, so intrinsics will not be generated";
            return false;
        }
        alwaysJump = true;
    }

    return ReplaceArrayInitLoop(loop, &loopInfo, store, alwaysJump);
}

Inst *LoopIdioms::CreateArrayInitIntrinsic(StoreInst *store, CountableLoopInfo *info)
{
    auto type = store->GetType();
    RuntimeInterface::IntrinsicId intrinsicId;
    switch (type) {
        case DataType::BOOL:
        case DataType::INT8:
        case DataType::UINT8:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8;
            break;
        case DataType::INT16:
        case DataType::UINT16:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16;
            break;
        case DataType::INT32:
        case DataType::UINT32:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32;
            break;
        case DataType::INT64:
        case DataType::UINT64:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64;
            break;
        case DataType::FLOAT32:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F32;
            break;
        case DataType::FLOAT64:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F64;
            break;
        default:
            return nullptr;
    }

    auto fillArray = GetGraph()->CreateInstIntrinsic(DataType::VOID, store->GetPc(), intrinsicId);
    fillArray->ClearFlag(inst_flags::Flags::REQUIRE_STATE);
    fillArray->ClearFlag(inst_flags::Flags::RUNTIME_CALL);
    fillArray->ClearFlag(inst_flags::Flags::CAN_THROW);
    fillArray->SetInputs(GetGraph()->GetAllocator(), {{store->GetArray(), DataType::REFERENCE},
                                                      {store->GetStoredValue(), type},
                                                      {info->init, DataType::INT32},
                                                      {info->test, DataType::INT32}});
    return fillArray;
}

bool LoopIdioms::ReplaceArrayInitLoop(Loop *loop, CountableLoopInfo *loopInfo, StoreInst *store, bool alwaysJump)
{
    auto inst = CreateArrayInitIntrinsic(store, loopInfo);
    if (inst == nullptr) {
        return false;
    }
    auto header = loop->GetHeader();
    auto preHeader = loop->GetPreHeader();

    auto loopSucc = header->GetSuccessor(0) == header ? header->GetSuccessor(1) : header->GetSuccessor(0);
    if (alwaysJump) {
        ASSERT(loop->GetBlocks().size() == 1);
        // insert block before disconnecting header to properly handle Phi in loop_succ
        auto block = header->InsertNewBlockToSuccEdge(loopSucc);
        preHeader->ReplaceSucc(header, block, true);
        GetGraph()->DisconnectBlock(header, false, false);
        block->AppendInst(inst);

        COMPILER_LOG(INFO, LOOP_TRANSFORM) << "Replaced loop " << loop->GetId() << " with the instruction " << *inst
                                           << " inserted into the new block " << block->GetId();
    } else {
        auto guardBlock = preHeader->InsertNewBlockToSuccEdge(header);
        auto sub = GetGraph()->CreateInstSub(DataType::INT32, inst->GetPc(), loopInfo->test, loopInfo->init);
        auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, inst->GetPc(), sub,
                                                 GetGraph()->FindOrCreateConstant(ITERATIONS_THRESHOLD),
                                                 DataType::INT32, ConditionCode::CC_LE);
        auto ifImm =
            GetGraph()->CreateInstIfImm(DataType::NO_TYPE, inst->GetPc(), cmp, 0, DataType::BOOL, ConditionCode::CC_NE);
        guardBlock->AppendInst(sub);
        guardBlock->AppendInst(cmp);
        guardBlock->AppendInst(ifImm);

        auto mergeBlock = header->InsertNewBlockToSuccEdge(loopSucc);
        auto intrinsicBlock = GetGraph()->CreateEmptyBlock();

        guardBlock->AddSucc(intrinsicBlock);
        intrinsicBlock->AddSucc(mergeBlock);
        intrinsicBlock->AppendInst(inst);

        COMPILER_LOG(INFO, LOOP_TRANSFORM) << "Inserted conditional jump into intinsic " << *inst << " before  loop "
                                           << loop->GetId() << ", inserted blocks: " << intrinsicBlock->GetId() << ", "
                                           << guardBlock->GetId() << ", " << mergeBlock->GetId();
    }
    return true;
}

}  // namespace ark::compiler
