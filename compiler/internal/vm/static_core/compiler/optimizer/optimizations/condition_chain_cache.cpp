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

#include "optimizer/ir/analysis.h"
#include "condition_chain_cache.h"
#include "condition_chain.h"

namespace ark::compiler {
ConditionChainCache::ConditionChainCache(ArenaAllocator *allocator) : cache_(allocator->Adapter()) {}

void ConditionChainCache::Insert(ConditionChain *chain, Inst *phiInst)
{
    cache_.insert({chain, phiInst});
}

static bool Contains(const ConditionChain *chain0, const ConditionChain *chain1)
{
    auto mps0 = chain0->GetMultiplePredecessorsSuccessor();
    auto mps1 = chain1->GetMultiplePredecessorsSuccessor();
    for (auto it0 = chain0->GetBegin(); it0 != chain0->GetEnd(); ++it0) {
        bool ret = false;
        auto bb0 = *it0;
        auto inverted0 = bb0->GetTrueSuccessor() != mps0;
        for (auto it1 = chain1->GetBegin(); it1 != chain1->GetEnd(); ++it1) {
            auto bb1 = *it1;
            auto inverted1 = bb1->GetTrueSuccessor() != mps1;
            if (IsConditionEqual((*it0)->GetLastInst(), (*it1)->GetLastInst(), inverted0 != inverted1)) {
                ret = true;
                break;
            }
        }
        if (!ret) {
            return false;
        }
    }
    return true;
}

static bool Equal(const ConditionChain *chain0, const ConditionChain *chain1)
{
    return Contains(chain0, chain1) && Contains(chain1, chain0);
}

Inst *ConditionChainCache::FindPhi(const ConditionChain *chain)
{
    for (auto &[c, phi] : cache_) {
        if (Equal(c, chain)) {
            return phi;
        }
    }
    return nullptr;
}

void ConditionChainCache::Clear()
{
    cache_.clear();
}
}  // namespace ark::compiler
