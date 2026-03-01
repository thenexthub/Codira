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

#include "Codira/CHIR/Analysis/GetOrThrowResultAnalysis.h"
#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/Type/StructDef.h"
#include "Codira/CHIR/Utils.h"

using namespace Codira::CHIR;

GetOrThrowResultDomain::GetOrThrowResultDomain(std::unordered_map<const Value*, size_t>* argIdxMap)
    : AbstractDomain(),
      getOrThrowResults(std::vector<FlatSet<const Apply>>(argIdxMap->size(), FlatSet<const Apply>(false))),
      argIdxMap(argIdxMap)
{
}

bool GetOrThrowResultDomain::Join(const GetOrThrowResultDomain& rhs)
{
    this->kind = ReachableKind::REACHABLE;
    return VectorJoin(getOrThrowResults, rhs.getOrThrowResults);
}

std::string GetOrThrowResultDomain::ToString() const
{
    if (this->kind == ReachableKind::UNREACHABLE) {
        return "Unreachable";
    } else {
        std::stringstream ss;
        ss << "{ ";
        for (auto& result : getOrThrowResults) {
            ss << result.ToString() << ", ";
        }
        ss << "}";
        return ss.str();
    }
}

const Apply* GetOrThrowResultDomain::CheckGetOrThrowResult(const Value* location) const
{
    if (auto it = argIdxMap->find(location); it != argIdxMap->end()) {
        return getOrThrowResults[it->second].GetElem().value_or(nullptr);
    } else {
        return nullptr;
    }
}

template <> const std::string Analysis<GetOrThrowResultDomain>::name = "getOrThrow-result";
template <> const std::optional<unsigned> Analysis<GetOrThrowResultDomain>::blockLimit = std::nullopt;

GetOrThrowResultAnalysis::GetOrThrowResultAnalysis(const Func* func, bool isDebug) : Analysis(func, isDebug)
{
    size_t argIdx = 0;
    for (auto bb : func->GetBody()->GetBlocks()) {
        for (auto expr : bb->GetExpressions()) {
            if (IsGetOrThrowFunction(*expr)) {
                auto apply = StaticCast<const Apply*>(expr);
                CODEC_ASSERT(apply->GetArgs().size() > 0);
                auto arg = apply->GetArgs()[0];
                if (auto it = argIdxMap.find(arg); it == argIdxMap.end()) {
                    argIdxMap.emplace(arg, argIdx++);
                }
            }
        }
    }
}

GetOrThrowResultDomain GetOrThrowResultAnalysis::Bottom()
{
    return GetOrThrowResultDomain(&argIdxMap);
}

// Set the initial state of the Function entryBB to Top to make sure the
// initial state of all the BB in the function is correct.
// 1. the entryBB of Function will dominate all the other BB.
// 2. the Top state will dominate all the other state.
void GetOrThrowResultAnalysis::InitializeFuncEntryState(GetOrThrowResultDomain& state)
{
    state.kind = ReachableKind::REACHABLE;
    for (auto i = state.getOrThrowResults.begin(); i != state.getOrThrowResults.end(); ++i) {
        i->SetToBound(/* isTop = */ true);
    }
}

void GetOrThrowResultAnalysis::PropagateExpressionEffect(GetOrThrowResultDomain& state, const Expression* expression)
{
    if (IsGetOrThrowFunction(*expression)) {
        auto apply = StaticCast<const Apply*>(expression);
        CODEC_ASSERT(apply->GetArgs().size() > 0);
        auto arg = apply->GetArgs()[0];
        if (auto it = argIdxMap.find(arg); it != argIdxMap.end()) {
            // Update the result of getOrThrow when the arg of getOrThrow is
            // first seen in this block;
            if (state.getOrThrowResults[it->second].IsBottom() || state.getOrThrowResults[it->second].IsTop()) {
                state.getOrThrowResults[it->second].UpdateElem(apply);
            }
        }
    }
}

std::optional<Block*> GetOrThrowResultAnalysis::PropagateTerminatorEffect(
    GetOrThrowResultDomain& state, const Terminator* terminator)
{
    (void)state;
    (void)terminator;
    return std::nullopt;
}
