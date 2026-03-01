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

#include "condition_chain_manager.h"
#include "condition_chain.h"

namespace ark::compiler {
ConditionChainManager::ConditionChainManager(ArenaAllocator *allocator)
    : allocator_(allocator), conditionChainBb_(allocator->Adapter())
{
}

ConditionChain *ConditionChainManager::FindConditionChain(BasicBlock *bb)
{
    if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }

    auto conditionChain = TryConditionChain(bb, bb->GetTrueSuccessor(), bb->GetFalseSuccessor());
    if (conditionChain != nullptr) {
        return conditionChain;
    }

    return TryConditionChain(bb, bb->GetFalseSuccessor(), bb->GetTrueSuccessor());
}

ConditionChain *ConditionChainManager::TryConditionChain(BasicBlock *bb, BasicBlock *multiplePredsSucc,
                                                         BasicBlock *chainBb)
{
    auto loop = bb->GetLoop();
    if (multiplePredsSucc->GetLoop() != loop) {
        return nullptr;
    }
    if (chainBb->GetLoop() != loop) {
        return nullptr;
    }
    auto chainPos = conditionChainBb_.size();
    conditionChainBb_.push_back(bb);

    size_t singlePredSuccIndex = 0;
    while (true) {
        if (IsConditionChainCandidate(chainBb)) {
            if (chainBb->GetTrueSuccessor() == multiplePredsSucc) {
                conditionChainBb_.push_back(chainBb);
                chainBb = chainBb->GetFalseSuccessor();
                singlePredSuccIndex = BasicBlock::FALSE_SUCC_IDX;
                continue;
            }
            if (chainBb->GetFalseSuccessor() == multiplePredsSucc) {
                conditionChainBb_.push_back(chainBb);
                chainBb = chainBb->GetTrueSuccessor();
                singlePredSuccIndex = BasicBlock::TRUE_SUCC_IDX;
                continue;
            }
        }
        auto chainSize = conditionChainBb_.size() - chainPos;
        if (chainSize > 1) {
            // store successors indices instead of pointers to basic blocks because they can be changed during loop
            // transformation
            return allocator_->New<ConditionChain>(conditionChainBb_.begin() + chainPos, chainSize,
                                                   bb->GetSuccBlockIndex(multiplePredsSucc), singlePredSuccIndex);
        }
        conditionChainBb_.pop_back();
        break;
    }
    return nullptr;
}

bool ConditionChainManager::IsConditionChainCandidate(const BasicBlock *bb)
{
    auto loop = bb->GetLoop();
    return bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM && bb->GetLastInst()->GetPrev() == nullptr &&
           bb->GetTrueSuccessor()->GetLoop() == loop && bb->GetFalseSuccessor()->GetLoop() == loop;
}

void ConditionChainManager::Reset()
{
    conditionChainBb_.clear();
}
}  // namespace ark::compiler
