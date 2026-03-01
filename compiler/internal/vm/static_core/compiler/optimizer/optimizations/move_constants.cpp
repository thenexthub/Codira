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
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {

MoveConstants::MoveConstants(Graph *graph)
    : Optimization {graph},
      userDominatorsCache_ {graph->GetLocalAllocator()->Adapter()},
      userDominatingBlocks_ {graph->GetLocalAllocator()->Adapter()}
{
}

static Inst *SingleBlockNoPhiDominatingUser(Inst *inst);

bool MoveConstants::RunImpl()
{
    for (auto constInst = GetGraph()->GetFirstConstInst(); constInst != nullptr;) {
        // save next const because it can be lost while move
        auto nextConst = constInst->GetNextConst();
        if (constInst->HasUsers()) {
            MoveFromStartBlock(constInst);
        }
        constInst = nextConst;
    }

    if (GetGraph()->HasNullPtrInst()) {
        auto nullPtr = GetGraph()->GetNullPtrInst();
        if (nullPtr->HasUsers()) {
            MoveFromStartBlock(nullPtr);
        }
    }

    if (GetGraph()->HasUniqueObjectInst()) {
        auto uniqueObj = GetGraph()->GetUniqueObjectInst();
        if (uniqueObj->HasUsers()) {
            MoveFromStartBlock(uniqueObj);
        }
    }

    return movedConstantsCounter_ > 0;
}

bool IsBlockSuitable(const BasicBlock *bb)
{
    return (!bb->IsLoopValid() || bb->GetLoop()->IsRoot()) && !bb->IsTryBegin();
}

void MoveConstants::MoveFromStartBlock(Inst *inst)
{
    auto graph = GetGraph();

    BasicBlock *targetBb = nullptr;
    auto userInst = SingleBlockNoPhiDominatingUser(inst);
    if (userInst != nullptr) {
        targetBb = userInst->GetBasicBlock();
        if (IsBlockSuitable(targetBb)) {
            graph->GetStartBlock()->EraseInst(inst);
            targetBb->InsertBefore(inst, userInst);
            movedConstantsCounter_++;
            return;
        }
    } else {
        GetUsersDominatingBlocks(inst);
        targetBb = FindCommonDominator();
        ASSERT(targetBb);
    }

    while (!IsBlockSuitable(targetBb)) {
        targetBb = targetBb->GetDominator();
    }

    if (targetBb != graph->GetStartBlock()) {
        graph->GetStartBlock()->EraseInst(inst);
        auto firstInst = targetBb->GetFirstInst();
        if (firstInst != nullptr && (firstInst->IsCatchPhi() || firstInst->IsPhi())) {
            while (firstInst != nullptr && (firstInst->IsCatchPhi() || firstInst->IsPhi())) {
                firstInst = firstInst->GetNext();
            }
            if (firstInst != nullptr) {
                targetBb->InsertBefore(inst, firstInst);
            } else {
                targetBb->AppendInst(inst);
            }
        } else {
            targetBb->PrependInst(inst);
        }
        movedConstantsCounter_++;
    }
}

static Inst *SingleBlockNoPhiDominatingUser(Inst *inst)
{
    Inst *firstInst {};
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsPhi() || userInst->IsCatchPhi()) {
            return nullptr;
        }
        if (firstInst == nullptr) {
            firstInst = userInst;
            continue;
        }
        if (firstInst->GetBasicBlock() != userInst->GetBasicBlock()) {
            return nullptr;
        }
        if (userInst->IsDominate(firstInst)) {
            firstInst = userInst;
        }
    }
    return firstInst;
}

void MoveConstants::GetUsersDominatingBlocks(const Inst *inst)
{
    ASSERT(inst->HasUsers());

    userDominatingBlocks_.clear();

    for (auto &user : inst->GetUsers()) {
        userDominatingBlocks_.emplace_back(GetDominators(user));
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
    auto cachedDominators = userDominatorsCache_.find(id);
    if (cachedDominators != userDominatorsCache_.end()) {
        return &cachedDominators->second;
    }

    ArenaVector<BasicBlock *> dominators(GetGraph()->GetLocalAllocator()->Adapter());

    // method does not mutate user but returns non const basic blocks
    auto firstDominator = const_cast<BasicBlock *>(inst->GetBasicBlock());
    if (inst->IsPhi()) {
        // block where phi-input is located should dominate predecessor block corresponding to this input
        firstDominator = firstDominator->GetDominator();
    }
    for (auto blk = firstDominator; blk != nullptr; blk = blk->GetDominator()) {
        dominators.push_back(blk);
    }

    auto result = userDominatorsCache_.emplace(id, dominators);
    return &result.first->second;
}

BasicBlock *MoveConstants::FindCommonDominator()
{
    ASSERT(!userDominatingBlocks_.empty());

    BasicBlock *commonDominator {};

    for (size_t i = 0;; ++i) {
        BasicBlock *commonDominatorCandidate {};

        for (auto blocks : userDominatingBlocks_) {
            if (i >= blocks->size()) {
                return commonDominator;
            }

            auto blk = (*blocks)[blocks->size() - i - 1];
            if (commonDominatorCandidate == nullptr) {
                commonDominatorCandidate = blk;
                continue;
            }
            if (commonDominatorCandidate != blk) {
                return commonDominator;
            }
        }

        commonDominator = commonDominatorCandidate;
    }

    return commonDominator;
}

}  // namespace ark::compiler
