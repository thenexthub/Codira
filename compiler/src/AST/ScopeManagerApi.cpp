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
 * This file implements ScopeManager apis.
 */

#include "Codira/AST/ScopeManagerApi.h"

#include "Codira/AST/ASTContext.h"

using namespace Codira;
using namespace AST;

Symbol* ScopeManagerApi::GetScopeGate(const ASTContext& ctx, const std::string& scopeName)
{
    auto it = ctx.invertedIndex.scopeGateMap.find(scopeName);
    if (it != ctx.invertedIndex.scopeGateMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::string ScopeManagerApi::GetScopeGateName(const std::string& scopeNameOrGateName)
{
    std::string currentScope;
    auto found = scopeNameOrGateName.find_last_of(childScopeNameSplit);
    if (found != std::string::npos) {
        // e.g. a0a_a -> a0a (intermediate step: remove '_' suffix)
        //      Then a0a -> a_a (final: convert to scope gate name)
        //      If input is a scope gate name, returns parent scope gate name.
        //      If input is a scope name, returns current scope gate name.
        currentScope = scopeNameOrGateName.substr(0, found);
    } else {
        currentScope = scopeNameOrGateName;
    }
    found = currentScope.find_last_of(scopeNameSplit);
    if (found != std::string::npos) {
        // e.g. a0a -> a_a
        currentScope.replace(found, 1, 1, childScopeNameSplit);
    } else {
        // Toplevel don't have root name.
        return "";
    }
    return currentScope;
}
