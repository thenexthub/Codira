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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CONDITION_CHAIN_MANAGER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CONDITION_CHAIN_MANAGER_H

#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "condition_chain.h"

namespace ark::compiler {

class ConditionChainManager {
public:
    explicit ConditionChainManager(ArenaAllocator *allocator);

    /**
     * Find condition chain and create new ConditionChain object which shares vector of basic blocks to avoid multiple
     * vector allocations.
     */
    ConditionChain *FindConditionChain(BasicBlock *bb);
    void Reset();

private:
    ConditionChain *TryConditionChain(BasicBlock *bb, BasicBlock *multiplePredsSucc, BasicBlock *chainBb);
    bool IsConditionChainCandidate(const BasicBlock *bb);
    ArenaAllocator *allocator_;
    ArenaVector<BasicBlock *> conditionChainBb_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CONDITION_CHAIN_MANAGER_H
