/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/loop_peeling.h"

namespace ark::compiler {
/**
 * Loop-peeling optimization works with a loops with the following requirements:
 * - loop is not irreducible;
 * - there is only 1 back-edge;
 * - loop-header is a single loop-exit point;
 *
 *          [pre-header]
 *              |
 *              v
 *     /---->[header]--------\
 *     |        |            |
 *     |        v            v
 *     \----[back-edge]   [outer]
 *
 * There are two stages of the algorithm
 * 1 stage - insert pre-loop which is an if-block:
 *
 *         [pre-header]
 *              |
 *              v
 *          [pre-loop]--------\
 *              |             |
 *              v             v
 *     /---->[header]-------->|
 *     |        |             |
 *     |        v             v
 *     \----[back-edge]   [resolver]
 *                            |
 *                            v
 *                         [outer]
 *
 * 2 stage - move exit-edge form the header to the back-edge block:
 *
 *         [pre-header]
 *              |
 *              v
 *          [pre-loop]--------\
 *              |             |
 *              v             v
 *     /---->[header]         |
 *     |        |             |
 *     |        v             v
 *     \----[back-edge]-->[resolver]
 *                            |
 *                            v
 *                         [outer]
 *
 */
bool LoopPeeling::RunImpl()
{
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return isAppied_;
}

void LoopPeeling::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
}

static inline void CleanDeadPhis(BasicBlock *block)
{
    for (auto phi : block->PhiInstsSafe()) {
        if (!phi->HasUsers()) {
            block->RemoveInst(phi);
        }
    }
}

static bool HeaderHasInlinedCalls(const BasicBlock *header)
{
    auto checkInlinedCall = [](auto inst) {
        return inst->IsCall() && static_cast<const CallInst *>(inst)->IsInlined();
    };
    auto insts = header->AllInsts();
    return std::find_if(insts.begin(), insts.end(), checkInlinedCall) != insts.end();
}

bool LoopPeeling::TransformLoop(Loop *loop)
{
    ASSERT(loop->GetBackEdges().size() == 1);
    ASSERT(loop->GetHeader()->GetLastInst()->GetOpcode() == Opcode::IfImm ||
           loop->GetHeader()->GetLastInst()->GetOpcode() == Opcode::If);
    if (!loop->GetInnerLoops().empty()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop wasn't peeled since it contains loops. Loop id = " << loop->GetId();
        return false;
    }
    auto header = loop->GetHeader();
    // If header contains inlined call this call will be cloned and stay unpaired without `Return.inlined` instruction
    if (HeaderHasInlinedCalls(header)) {
        return false;
    }
    auto backEdge = loop->GetBackEdges()[0];
    InsertPreLoop(loop);
    auto movedInstCount = MoveLoopExitToBackEdge(header, backEdge);
    CleanDeadPhis(header);
    isAppied_ = true;
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Loop was peeled, id = " << loop->GetId();
    GetGraph()->GetEventWriter().EventLoopPeeling(loop->GetId(), header->GetGuestPc(), backEdge->GetGuestPc(),
                                                  movedInstCount);
    return true;
}

/// Clone the loop-header and insert before it
void LoopPeeling::InsertPreLoop(Loop *loop)
{
    auto header = loop->GetHeader();
    auto preHeader = loop->GetPreHeader();
    auto graphCloner =
        GraphCloner(header->GetGraph(), header->GetGraph()->GetAllocator(), header->GetGraph()->GetLocalAllocator());
    graphCloner.CloneLoopHeader(header, GetLoopOuterBlock(header), preHeader);
}

/*
 *  Make back-edge loop-exit point
 */
size_t LoopPeeling::MoveLoopExitToBackEdge(BasicBlock *header, BasicBlock *backEdge)
{
    size_t movedInstCount = 0;
    auto outerBlock = GetLoopOuterBlock(header);
    size_t outerIdx = header->GetSuccBlockIndex(outerBlock);

    // Create exit block
    BasicBlock *exitBlock = nullptr;
    if (header != backEdge) {
        ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
        exitBlock = backEdge->InsertNewBlockToSuccEdge(header);
        outerBlock->ReplacePred(header, exitBlock);
        header->RemoveSucc(outerBlock);
        exitBlock->SetTry(header->IsTry());

        auto loop = header->GetLoop();
        loop->AppendBlock(exitBlock);
        loop->ReplaceBackEdge(backEdge, exitBlock);

        // Check the order of true-false successors
        if (exitBlock->GetSuccBlockIndex(outerBlock) != outerIdx) {
            exitBlock->SwapTrueFalseSuccessors();
        }

        // Fix Dominators info
        auto &domTree = GetGraph()->GetAnalysis<DominatorsTree>();
        domTree.SetValid(true);
        domTree.SetDomPair(backEdge, exitBlock);
        backEdge = exitBlock;
    }

    // Use reverse order to keep domination relation between instructions in the header-block
    for (auto inst : header->InstsSafeReverse()) {
        if (exitBlock != nullptr) {
            header->EraseInst(inst);
            exitBlock->PrependInst(inst);
            movedInstCount++;
        }
        UpdateClonedInstInputs(inst, header, backEdge);
    }

    // Update outer phis
    for (auto phi : outerBlock->PhiInsts()) {
        size_t headerIdx = phi->CastToPhi()->GetPredBlockIndex(backEdge);
        auto headerInst = phi->GetInput(headerIdx).GetInst();
        if (headerInst->IsPhi()) {
            phi->SetInput(headerIdx, headerInst->CastToPhi()->GetPhiInput(backEdge));
        }
    }

    ssb_.FixPhisWithCheckInputs(header);
    ssb_.FixPhisWithCheckInputs(exitBlock);
    ssb_.FixPhisWithCheckInputs(outerBlock);

    return movedInstCount;
}

void LoopPeeling::UpdateClonedInstInputs(Inst *inst, BasicBlock *header, BasicBlock *backEdge)
{
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        auto inputInst = inst->GetInput(i).GetInst();
        if (inputInst->IsPhi() && inputInst->GetBasicBlock() == header) {
            auto phiInput = inputInst->CastToPhi()->GetPhiInput(backEdge);
            // Replace phi by its input, if this input will NOT be moved to the exit block
            bool isMoved = phiInput->GetBasicBlock() == header && !phiInput->IsPhi();
            if (phiInput->IsDominate(inst) && !isMoved) {
                inst->SetInput(i, phiInput);
            }
        }
    }
}
}  // namespace ark::compiler
