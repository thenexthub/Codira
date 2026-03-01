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

#include "move_constants.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace panda::compiler {

MoveConstants::MoveConstants(Graph *graph)
    : Optimization {graph},
      user_dominators_cache_ {graph->GetLocalAllocator()->Adapter()},
      user_dominating_blocks_ {graph->GetLocalAllocator()->Adapter()},
      moved_constants_counter_ {0}
{
}

static Inst *SingleBlockNoPhiDominatingUser(Inst *inst);

bool MoveConstants::RunImpl()
{
    for (auto const_inst = GetGraph()->GetFirstConstInst(); const_inst != nullptr;) {
        // save next const because it can be lost while move
        auto next_const = const_inst->GetNextConst();
        if (const_inst->HasUsers()) {
            MoveFromStartBlock(const_inst);
        }
        const_inst = next_const;
    }

    return moved_constants_counter_ > 0;
}

bool IsBlockSuitable(const BasicBlock *bb)
{
    return (!bb->IsLoopValid() || bb->GetLoop()->IsRoot()) && !bb->IsTryBegin() && !bb->IsCatchBegin();
}

void MoveConstants::MoveFromStartBlock(Inst *inst)
{
    auto graph = GetGraph();

    BasicBlock *target_bb = nullptr;
    auto user_inst = SingleBlockNoPhiDominatingUser(inst);
    if (user_inst != nullptr) {
        target_bb = user_inst->GetBasicBlock();
        if (IsBlockSuitable(target_bb)) {
            graph->GetStartBlock()->EraseInst(inst);
            target_bb->InsertBefore(inst, user_inst);
            moved_constants_counter_++;
            return;
        }
    } else {
        GetUsersDominatingBlocks(inst);
        target_bb = FindCommonDominator();
        ASSERT(target_bb);
    }

    while (!IsBlockSuitable(target_bb)) {
        target_bb = target_bb->GetDominator();
    }

    if (target_bb != graph->GetStartBlock()) {
        graph->GetStartBlock()->EraseInst(inst);
        target_bb->PrependInst(inst);
        moved_constants_counter_++;
    }
}

static Inst *SingleBlockNoPhiDominatingUser(Inst *inst)
{
    Inst *first_inst {};
    for (auto &user : inst->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst->IsPhi() || user_inst->IsCatchPhi()) {
            return nullptr;
        }
        if (first_inst == nullptr) {
            first_inst = user_inst;
            continue;
        }
        if (first_inst->GetBasicBlock() != user_inst->GetBasicBlock()) {
            return nullptr;
        }
        if (user_inst->IsDominate(first_inst)) {
            first_inst = user_inst;
        }
    }
    return first_inst;
}

void MoveConstants::GetUsersDominatingBlocks(const Inst *inst)
{
    ASSERT(inst->HasUsers());

    user_dominating_blocks_.clear();

    for (auto &user : inst->GetUsers()) {
        user_dominating_blocks_.emplace_back(GetDominators(user));
    }
}

const ArenaVector<BasicBlock *> *MoveConstants::GetDominators(const User &user)
{
    auto inst = user.GetInst();
    if (inst->IsCatchPhi()) {
        // do not move catch-phi's input over throwable instruction
        inst = inst->CastToCatchPhi()->GetThrowableInst(user.GetIndex());
    }
    auto id = inst->GetId();
    auto cached_dominators = user_dominators_cache_.find(id);
    if (cached_dominators != user_dominators_cache_.end()) {
        return &cached_dominators->second;
    }

    ArenaVector<BasicBlock *> dominators(GetGraph()->GetLocalAllocator()->Adapter());

    // method does not mutate user but returns non const basic blocks
    auto first_dominator = const_cast<BasicBlock *>(inst->GetBasicBlock());
    if (inst->IsPhi()) {
        // block where phi-input is located should dominate predecessor block corresponding to this input
        first_dominator = first_dominator->GetDominator();
    }
    for (auto blk = first_dominator; blk != nullptr; blk = blk->GetDominator()) {
        dominators.push_back(blk);
    }

    auto result = user_dominators_cache_.emplace(id, dominators);
    return &result.first->second;
}

BasicBlock *MoveConstants::FindCommonDominator()
{
    ASSERT(!user_dominating_blocks_.empty());

    BasicBlock *common_dominator {};

    for (size_t i = 0;; ++i) {
        BasicBlock *common_dominator_candidate {};

        for (auto blocks : user_dominating_blocks_) {
            if (i >= blocks->size()) {
                return common_dominator;
            }

            auto blk = (*blocks)[blocks->size() - i - 1];
            if (common_dominator_candidate == nullptr) {
                common_dominator_candidate = blk;
                continue;
            }
            if (common_dominator_candidate != blk) {
                return common_dominator;
            }
        }

        common_dominator = common_dominator_candidate;
    }

    return common_dominator;
}

}  // namespace panda::compiler
