/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"
#include "licm_conditions.h"

namespace ark::compiler {
bool LicmConditions::RunImpl()
{
    conditionChainManager_.Reset();
    MarkerHolder hoistableInstMh(GetGraph());
    MarkerHolder processedBlocksMh(GetGraph());
    hoistableInstMarker_ = hoistableInstMh.GetMarker();
    processedBlocksMarker_ = processedBlocksMh.GetMarker();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    GetGraph()->RunPass<DominatorsTree>();
    MarkHoistableInst();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return isApplied_;
}

void LicmConditions::MarkHoistableInst()
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!bb->IsLoopValid()) {
            continue;
        }
        auto loop = bb->GetLoop();
        if (loop->IsRoot() || loop->IsIrreducible()) {
            continue;
        }
        for (auto inst : bb->PhiInsts()) {
            if (AllInputsDominate(inst, loop)) {
                inst->SetMarker(hoistableInstMarker_);
            }
        }
        if (bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM) {
            auto lastInst = bb->GetLastInst();
            if (AllInputsDominate(lastInst, loop)) {
                lastInst->SetMarker(hoistableInstMarker_);
            }
        }
    }
}

bool LicmConditions::TransformLoop(Loop *loop)
{
    samePhiInput_.clear();
    conditionChainsCache_.Clear();
    FindHoistableConditionChains(loop);
    HoistConditionChains(loop);
    return true;
}

void LicmConditions::FindHoistableConditionChains(Loop *loop)
{
    conditionChainsCtx_.clear();
    for (auto block : loop->GetBlocks()) {
        auto chain = conditionChainManager_.FindConditionChain(block);
        if (chain == nullptr) {
            continue;
        }
        auto multiplePredsSucc = chain->GetMultiplePredecessorsSuccessor();
        auto singlePredSucc = chain->GetSinglePredecessorSuccessor();
        COMPILER_LOG(DEBUG, LICM_COND_OPT)
            << "Found conditions chain " << chain->GetFirstBlock()->GetId() << "->" << chain->GetLastBlock()->GetId()
            << ", succs: " << multiplePredsSucc->GetId() << ", " << singlePredSucc->GetId();
        if (!IsHoistable(chain)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Skip not hoistable chain";
            continue;
        }
        auto hoistPhiAvailable = IsHoistPhiAvailable(chain, multiplePredsSucc->GetPredsBlocks(), singlePredSucc);
        if (!AllPhiAllowConditionChainHoisting(chain, multiplePredsSucc, hoistPhiAvailable)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Skip not all phi are suitable";
            continue;
        }
        conditionChainsCtx_.emplace_back(chain, multiplePredsSucc, singlePredSucc, hoistPhiAvailable);
    }
    std::sort(conditionChainsCtx_.begin(), conditionChainsCtx_.end(),
              [](auto a, auto b) { return a.GetChain()->GetSize() > b.GetChain()->GetSize(); });
}

bool LicmConditions::IsHoistable(const ConditionChain *chain)
{
    auto last = chain->GetEnd();
    for (auto bbIt = chain->GetBegin(); bbIt != last; ++bbIt) {
        auto bb = *bbIt;
        auto lastInst = bb->GetLastInst();
        if (lastInst->GetOpcode() != Opcode::IfImm) {
            return false;
        }
        if (!lastInst->IsMarked(hoistableInstMarker_)) {
            return false;
        }
    }
    return true;
}

// After LICM pass all hoistable inputs should be moved out from loop
bool LicmConditions::AllInputsDominate(const Inst *inst, const Loop *loop)
{
    auto preheader = loop->GetPreHeader();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        if (!inst->GetInput(i).GetInst()->GetBasicBlock()->IsDominate(preheader)) {
            return false;
        }
    }
    return true;
}

bool LicmConditions::IsHoistPhiAvailable(const ConditionChain *chain, ArenaVector<BasicBlock *> &preds,
                                         const BasicBlock *singlePredSucc)
{
    for (auto pred : preds) {
        if (!chain->Contains(pred) && pred != singlePredSucc) {
            return false;
        }
    }
    return true;
}

bool LicmConditions::AllPhiAllowConditionChainHoisting(const ConditionChain *chain, const BasicBlock *multiplePredsSucc,
                                                       bool hoistPhiAvailable)
{
    for (auto phi : multiplePredsSucc->PhiInsts()) {
        if (hoistPhiAvailable && phi->IsMarked(hoistableInstMarker_)) {
            continue;
        }
        auto sameInput = SamePhiInputFromChain(phi, chain);
        if (sameInput == nullptr) {
            return false;
        }
        samePhiInput_.insert({std::make_pair(chain, phi), sameInput});
    }
    return true;
}

Inst *LicmConditions::SamePhiInputFromChain(Inst *phi, const ConditionChain *chain)
{
    Inst *savedInput = nullptr;
    auto bb = phi->GetBasicBlock();
    for (auto pred : bb->GetPredsBlocks()) {
        if (!chain->Contains(pred)) {
            continue;
        }
        auto predIndex = bb->GetPredBlockIndex(pred);
        auto input = phi->GetInput(predIndex).GetInst();
        if (savedInput == nullptr) {
            savedInput = input;
        } else if (savedInput != input) {
            return nullptr;
        } else {
            continue;
        }
    }
    return savedInput;
}

/*
 * Move condition chains which look like
 *
 *          | |
 *          v v
 *          [A]------\
 *           |       |
 *           |       v
 *           |<-----[B]
 *           |       |
 *           v       v
 *       -->[S0]    [S1]<--
 *           |       |
 *           v       v
 *
 * out of the loop if possible.
 * After whole chain is moved out it looks like
 *
 *           |
 *           v
 *          [A]------\
 *           |       |
 *           |       v
 *           |<-----[B]
 *           |       |
 *           |       v
 *           |    [empty_block]
 *           |       |
 *           v       v
 *          [phi_block]
 *               |
 *               v
 *          [loop preheader]
 *
 * phi_block contains either new phi instructions which is result of condition chain
 * (single_condition_block uses it) or phi which was hoisted from `multiple_predecessors successor.
 * Condition chain is replaced in loop with single_condition_block
 *
 *              |
 *              v
 *     [single_condition_block]
 *           |       |
 *           v       v
 *       -->[S0]    [S1]<--
 *           |       |
 *           v       v
 */
void LicmConditions::HoistConditionChains(Loop *loop)
{
    auto appendBb = loop->GetPreHeader();
    for (auto chainCtx : conditionChainsCtx_) {
        auto chain = chainCtx.GetChain();
        auto chainFirstBlock = chain->GetFirstBlock();
        if (chainFirstBlock->IsMarked(processedBlocksMarker_)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT)
                << "Skip chain with first block #" << chainFirstBlock->GetId() << ", longer chain was processed";
            continue;
        }

        auto multiplePredsSucc = chain->GetMultiplePredecessorsSuccessor();
        auto singlePredSucc = chain->GetSinglePredecessorSuccessor();

        COMPILER_LOG(DEBUG, LICM_COND_OPT)
            << "Process conditions chain " << chainFirstBlock->GetId() << "->" << chain->GetLastBlock()->GetId()
            << ", succs: " << multiplePredsSucc->GetId() << ", " << singlePredSucc->GetId();

        SaveProcessedBlocks(chain);
        SplitChainFirstBasicBlock(chain);

        // update chain successors because they can be changed during previous transformations
        chainCtx.SetMultiplePredecessorsSuccessor(multiplePredsSucc);
        chainCtx.SetSingleSPredecessorSuccessor(singlePredSucc);

        appendBb = ReplaceChainWithSingleBlock(appendBb, chainCtx);

        isApplied_ = true;
    }
}

void LicmConditions::SaveProcessedBlocks(const ConditionChain *chain)
{
    std::for_each(chain->GetBegin(), chain->GetEnd(),
                  [this](BasicBlock *bb) { bb->SetMarker(processedBlocksMarker_); });
}

void LicmConditions::SaveMulitplePredecessorsSuccessorPreds(const BasicBlock *bb)
{
    multiplePredecessorsSuccessorPreds_.clear();
    std::copy(bb->GetPredsBlocks().begin(), bb->GetPredsBlocks().end(),
              std::back_inserter(multiplePredecessorsSuccessorPreds_));
}

void LicmConditions::SplitChainFirstBasicBlock(ConditionChain *chain)
{
    auto chainFirstBb = chain->GetFirstBlock();
    auto prelastInst = chainFirstBb->GetLastInst()->GetPrev();
    if (prelastInst == nullptr) {
        return;
    }
    auto newFirstBb = chainFirstBb->SplitBlockAfterInstruction(prelastInst, true);
    chain->SetFirstBlock(newFirstBb);
    COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Split first chain block " << chainFirstBb->GetId() << "->"
                                       << newFirstBb->GetId();
}

BasicBlock *LicmConditions::ReplaceChainWithSingleBlock(BasicBlock *appendBb, const ConditionChainContext &chainCtx)
{
    auto chain = chainCtx.GetChain();
    // try to find condition chain which is looking like this
    auto cachedPhi = conditionChainsCache_.FindPhi(chain);
    auto multiplePredsSucc = chainCtx.GetMultiplePredecessorsSuccessor();
    auto singlePredSucc = chainCtx.GetSinglePredecessorSuccessor();
    SaveMulitplePredecessorsSuccessorPreds(multiplePredsSucc);

    auto singleConditionBlock = GetGraph()->CreateEmptyBlock();
    AdjustPredecessorEdges(chain->GetFirstBlock(), singleConditionBlock);

    singleConditionBlock->AddSucc(multiplePredsSucc);
    // NOTE: multiple_preds_succ preds are not fixed
    auto chainLastBlock = chain->GetLastBlock();
    singlePredSucc->ReplacePred(chainLastBlock, singleConditionBlock);

    auto phiBlock = GetGraph()->CreateEmptyBlock();
    auto appendBbSucc = appendBb->GetSuccessor(0);
    appendBb->ReplaceSucc(appendBbSucc, chain->GetFirstBlock());
    appendBbSucc->ReplacePred(appendBb, phiBlock);

    auto emptyBlock = GetGraph()->CreateEmptyBlock();
    chainLastBlock->ReplaceSucc(singlePredSucc, emptyBlock, true);

    UpdateMultiplePredecessorsSuccessorsPreds(chainCtx, phiBlock, emptyBlock);
    UpdatePhis(chain, multiplePredsSucc, phiBlock, chainCtx.IsHoistPhiAvailable());

    Inst *phiInst;
    if (cachedPhi != nullptr) {
        phiInst = cachedPhi;
    } else {
        phiInst = AddPhiInst(phiBlock, chain);
        conditionChainsCache_.Insert(chain, phiInst);
    }
    AddSingleIfImmInst(singleConditionBlock, chain, phiInst);
    return phiBlock;
}

PhiInst *LicmConditions::AddPhiInst(BasicBlock *bb, const ConditionChain *chain)
{
    auto graph = bb->GetGraph();
    auto oneCnst = graph->FindOrCreateConstant(1);
    auto zeroCnst = graph->FindOrCreateConstant(0);
    auto phiInst = graph->CreateInstPhi(DataType::BOOL, INVALID_PC);
    bb->AppendPhi(phiInst);
    for (auto pred : bb->GetPredsBlocks()) {
        phiInst->AppendInput(chain->Contains(pred) ? oneCnst : zeroCnst);
    }
    return phiInst;
}

void LicmConditions::AddSingleIfImmInst(BasicBlock *bb, const ConditionChain *chain, Inst *input)
{
    auto origIfInst = chain->GetLastBlock()->GetLastInst()->CastToIfImm();
    auto ifInst = bb->GetGraph()->CreateInstIfImm(DataType::NO_TYPE, 0, input, 0, DataType::BOOL, ConditionCode::CC_NE,
                                                  origIfInst->GetMethod());
    bb->AppendInst(ifInst);
}

void LicmConditions::AdjustPredecessorEdges(BasicBlock *chainFirstBb, BasicBlock *bb)
{
    for (auto pred : chainFirstBb->GetPredsBlocks()) {
        pred->ReplaceSucc(chainFirstBb, bb);
    }
    chainFirstBb->GetPredsBlocks().clear();
}

void LicmConditions::UpdateMultiplePredecessorsSuccessorsPreds(const ConditionChainContext &chainCtx,
                                                               BasicBlock *phiBlock, BasicBlock *emptyBlock)
{
    auto chain = chainCtx.GetChain();
    auto multiplePredsSucc = chainCtx.GetMultiplePredecessorsSuccessor();
    auto singlePredSucc = chainCtx.GetSinglePredecessorSuccessor();
    multiplePredecessorsSuccessorRemovedPredIndices_.clear();
    // keep predecessors order in phi_block
    for (auto bb : multiplePredecessorsSuccessorPreds_) {
        if (chain->Contains(bb)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT)
                << "Update chain block " << bb->GetId() << " successor: " << multiplePredsSucc->GetId() << "->"
                << phiBlock->GetId();
            bb->ReplaceSucc(multiplePredsSucc, phiBlock, true);
            auto predIndex = multiplePredsSucc->GetPredBlockIndex(bb);
            multiplePredsSucc->RemovePred(predIndex);
            multiplePredecessorsSuccessorRemovedPredIndices_.push_back(predIndex);
        } else if (bb == singlePredSucc) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT)
                << "Add new edge to phi_block corresponding to edge between chain successors";
            emptyBlock->AddSucc(phiBlock, true);
        } else {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Skip predecessor " << bb->GetId();
        }
    }
    if (emptyBlock->GetSuccsBlocks().empty()) {
        COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Add last edge to phi_block";
        emptyBlock->AddSucc(phiBlock, true);
    }
}

void LicmConditions::UpdatePhis(const ConditionChain *chain, BasicBlock *multiplePredsSucc, BasicBlock *phiBlock,
                                bool hoistPhiAvailable)
{
    for (auto phi : multiplePredsSucc->PhiInstsSafe()) {
        if (hoistPhiAvailable && phi->IsMarked(hoistableInstMarker_)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Hoist phi " << phi->GetId();
            multiplePredsSucc->EraseInst(phi);
            // preds order was preserved
            phiBlock->AppendPhi(phi);
            if (phi->GetInputsCount() >= phiBlock->GetPredsBlocks().size()) {
                ASSERT(phi->GetInputsCount() == phiBlock->GetPredsBlocks().size());
                continue;
            }
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Add dummy input";
            if (DataType::IsReference(phi->GetType())) {
                phi->AppendInput(GetGraph()->GetOrCreateNullPtr());
            } else {
                phi->AppendInput(GetGraph()->FindOrCreateConstant(0));
            }
            ASSERT(phi->GetInputsCount() == phiBlock->GetPredsBlocks().size());
        } else {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Update inputs for phi " << phi->GetId();
            auto key = std::make_pair(chain, phi);
            ASSERT(samePhiInput_.count(key) != 0);
            phi->AppendInput(samePhiInput_[key]);
            for (size_t i : multiplePredecessorsSuccessorRemovedPredIndices_) {
                phi->RemoveInput(i);
            }
        }
    }
}

void LicmConditions::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace ark::compiler
