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

/**
 * @file
 *
 * This file implements type check cache utils.
 */

#include "Codira/AST/Cache.h"
#include "Codira/Utils/Casting.h"

using namespace Codira;
using namespace AST;

namespace Codira::AST {
TargetCache CollectTargets(const Node& node)
{
    if (auto farg = DynamicCast<const FuncArg*>(&node)) {
        return CollectTargets(*farg->expr);
    }
    if (auto target1 = node.GetTarget()) {
        if (auto ma = DynamicCast<const MemberAccess*>(&node);
               ma && ma->baseExpr && ma->baseExpr->IsReferenceExpr()) {
            auto target2 = ma->baseExpr->GetTarget();
            return std::make_pair(target1, target2);
        } else {
            return std::make_pair(target1, nullptr);
        }
    }
    return std::make_pair(nullptr, nullptr);
}

void RestoreTargets(Node& node, const TargetCache& targets)
{
    if (auto farg = DynamicCast<const FuncArg*>(&node)) {
        RestoreTargets(*farg->expr, targets);
    }
    node.SetTarget(targets.first);
    if (auto ma = DynamicCast<const MemberAccess*>(&node);
           ma && ma->baseExpr && ma->baseExpr->IsReferenceExpr()) {
        ma->baseExpr->SetTarget(targets.second);
    }
}

bool CacheKey::operator==(const CacheKey& b) const
{
    return target == b.target && isDesugared == b.isDesugared && diagKey == b.diagKey;
}

bool MemSig::operator==(const MemSig& b) const
{
    return id == b.id && isVarOrProp == b.isVarOrProp && arity == b.arity && genArity == b.genArity;
}
} // namespace Codira::AST
