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
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/dominators_tree.h"
#include "redundant_loop_elimination.h"

#include "optimizer/ir/basicblock.h"
namespace ark::compiler {
bool RedundantLoopElimination::RunImpl()
{
    COMPILER_LOG(DEBUG, RLE_OPT) << "Run " << GetPassName();
    RunLoopsVisitor();

    if (isApplied_) {
        COMPILER_LOG(DEBUG, RLE_OPT) << GetPassName() << " is applied";
    }
    return isApplied_;
}

BasicBlock *RedundantLoopElimination::IsRedundant(Loop *loop)
{
    BasicBlock *outsideSucc = nullptr;
    for (auto block : loop->GetBlocks()) {
        // check that loop have only one exit and one outside blocks
        for (auto succ : block->GetSuccsBlocks()) {
            if (succ->GetLoop() == loop) {
                continue;
            }
            if (outsideSucc == nullptr) {
                outsideSucc = succ;
                loopExit_ = block;
            } else {
                return nullptr;
            }
        }
        // check that loop doesn't contains not redundant insts
        for (auto inst : block->AllInsts()) {
            if (inst->IsNotRemovable() && inst->GetOpcode() != Opcode::IfImm &&
                inst->GetOpcode() != Opcode::SafePoint) {
                return nullptr;
            }
            auto users = inst->GetUsers();
            auto it = std::find_if(users.begin(), users.end(),
                                   [loop](auto &user) { return user.GetInst()->GetBasicBlock()->GetLoop() != loop; });
            if (it != users.end()) {
                return nullptr;
            }
        }
    }
    // Check that loop is finite.
    // We can remove only loops that is always finite.
    if (!CountableLoopParser(*loop).Parse().has_value()) {
        return nullptr;
    }
    return outsideSucc;
}

void RedundantLoopElimination::DeleteLoop(Loop *loop, BasicBlock *outsideSucc) const
{
    auto header = loop->GetHeader();
    auto preHeader = loop->GetPreHeader();
    ASSERT(loop->GetBackEdges().size() == 1);
    ASSERT(preHeader != nullptr);

    preHeader->ReplaceSucc(header, outsideSucc, true);
    ASSERT(outsideSucc->GetPredsBlocks().back() == preHeader);
    // replace loop_exit_ by pre_header in pred_blocks of outside_succ
    // (important when outside_succ has Phi instructions)
    outsideSucc->RemovePred(loopExit_);
    // prevent trying to remove loop_exit_ from outside_succ preds again in DisconnectBlock
    loopExit_->RemoveSucc(outsideSucc);

    for (auto block : loop->GetBlocks()) {
        GetGraph()->DisconnectBlock(block, false, false);
    }
}

bool RedundantLoopElimination::TransformLoop(Loop *loop)
{
    COMPILER_LOG(DEBUG, RLE_OPT) << "Visit loop with id = " << loop->GetId();
    auto outsideSucc = IsRedundant(loop);
    if (outsideSucc != nullptr) {
        DeleteLoop(loop, outsideSucc);
        isApplied_ = true;
        COMPILER_LOG(DEBUG, RLE_OPT) << "Loop with id = " << loop->GetId() << " is removed";
        GetGraph()->GetEventWriter().EventRedundantLoopElimination(loop->GetId(), loop->GetHeader()->GetGuestPc());
        return true;
    }
    return false;
}

void RedundantLoopElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace ark::compiler
