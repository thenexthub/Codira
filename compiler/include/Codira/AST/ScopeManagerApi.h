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
 * This file declares the ScopeManagerApi.
 */

#ifndef CODIRA_AST_SCOPEMANAGERAPI_H
#define CODIRA_AST_SCOPEMANAGERAPI_H

#include <string>
#include "Codira/AST/Symbol.h"

namespace Codira {
// Toplevel scope name "a".
const std::string TOPLEVEL_SCOPE_NAME = "a";

class ASTContext;
/**
 * Static methods and fields of ScopeManager.
 */
struct ScopeManagerApi {
    /**
     * Remove the child scope name info.
     *
     * @return A scope name without tailing '_*', given a0a_a, will get a0a.
     */
    static std::string GetScopeNameWithoutTail(const std::string& scopeName)
    {
        auto found = scopeName.find_last_of(childScopeNameSplit);
        if (found != std::string::npos) {
            return scopeName.substr(0, found);
        } else {
            return scopeName;
        }
    }

    /**
     * Get parent scope name.
     *
     * @param scopeName The scope name.
     *
     * @return Parent scope name, parent scope name of @c aa0ab_a is @c aa,
     * if already reach the toplevel, will get empty string.
     */
    static std::string GetParentScopeName(const std::string& scopeName)
    {
        if (scopeName == "A") {
            return "";
        }
        // If already reach toplevel.
        if (scopeName == TOPLEVEL_SCOPE_NAME) {
            return "A";
        }
        auto found = scopeName.find_last_of(scopeNameSplit);
        if (found == std::string::npos) {
            return TOPLEVEL_SCOPE_NAME;
        }
        return scopeName.substr(0, found);
    }

    /**
     * Get child scope name.
     *
     * @return Child scope name of scope gate, example: aa0ab_a -> aa0ab0a.
     */
    static std::string GetChildScopeName(const std::string& scopeName)
    {
        std::string result = scopeName;
        auto found = result.find_last_of(childScopeNameSplit);
        if (found == std::string::npos) {
            return result;
        }
        result.replace(result.find(childScopeNameSplit), 1, std::string(1, scopeNameSplit));
        return result;
    }

    /**
     * Get scope gate of @p scopeName.
     *
     * @return scope gate.
     */
    static AST::Symbol* GetScopeGate(const ASTContext& ctx, const std::string& scopeName);

    /**
     * Get scope gate name of @p scopeName.
     *
     * @param scopeNameOrGateName The scope name or scope gate name.
     * @return scope gate name, scope gate name of a0b is a_b, a0b_a is a_b.
     */
    static std::string GetScopeGateName(const std::string& scopeNameOrGateName);

    /**
     * Each level of scope name are encoding will 52 chars, split by '0'.
     *
     * ```
     * func foo
     * {
     *     // Scope name here is a0a.
     *     func goo
     *     {
     *         // Scope name here is a0a0a.
     *     }
     * }
     * ```
     */
    constexpr static char scopeNameSplit = '0';

    /**
     * Scope gate use this split to contain child scope info.
     *
     * ```
     * func foo // scope name of node FuncDecl @c foo is a_a
     * {
     *     // scope name here is a0a
     * }
     * ```
     */
    constexpr static char childScopeNameSplit = '_';
};
} // namespace Codira

#endif
