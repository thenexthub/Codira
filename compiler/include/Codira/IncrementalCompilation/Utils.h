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

#ifndef CODIRA_CACHE_DATA_UTILS_H
#define CODIRA_CACHE_DATA_UTILS_H

#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Codira/IncrementalCompilation/IncrementalScopeAnalysis.h"
#include "Codira/Parse/ASTHasher.h"
#include "Codira/Utils/SipHash.h"

namespace Codira::IncrementalCompilation {
void PrintDecl(std::ostream& out, const AST::Decl& decl);

bool IsVirtual(const AST::Decl& decl);

inline bool IsImported(const AST::Decl& decl)
{
    return decl.TestAttr(AST::Attribute::IMPORTED);
}

inline bool IsInstance(const AST::Decl& decl)
{
    return !decl.TestAttr(AST::Attribute::STATIC) && !decl.TestAttr(AST::Attribute::CONSTRUCTOR);
}

bool IsTyped(const AST::Decl& decl);

inline bool IsUntyped(const AST::Decl& decl)
{
    return !IsTyped(decl);
}

inline bool IsEnumConstructor(const AST::Decl& decl)
{
    return decl.TestAttr(AST::Attribute::ENUM_CONSTRUCTOR);
}

std::vector<Ptr<AST::Decl>> GetMembers(const AST::Decl& decl);

// returns true if this decl is possibly affected by decl order, either by use-after-check or by side effects
bool IsOOEAffectedDecl(const AST::Decl& decl);

// convert unsorted container to sorted, used commonly in incremental compilation to maintain or discard the order
template <class Cont, class Cmp, class T = decltype(*std::declval<Cont>().cbegin())>
std::vector<std::add_pointer_t<T>> ToSorted(const Cont& cont, Cmp&& cmp)
{
    std::vector<std::add_pointer_t<T>> ans(cont.size());
    size_t i{0};
    for (auto& elem: cont) {
        ans[i++] = &elem;
    }
    std::stable_sort(ans.begin(), ans.end(), [&cmp](auto a, auto b) { return cmp(*a, *b); });
    return ans;
}

template <class Cont, class T = decltype(*std::declval<Cont>().cbegin())>
std::vector<std::add_pointer_t<T>> ToSorted(const Cont& cont)
{
    std::vector<std::add_pointer_t<T>> ans(cont.size());
    size_t i{0};
    for (auto& elem: cont) {
        ans[i++] = &elem;
    }
    std::stable_sort(ans.begin(), ans.end(), [](auto a, auto b) { return *a < *b; });
    return ans;
}

// no default cmp function for container of pointers, because numeric values of pointers are never consistent
// always pass a compare function
template <class Cont, class Cmp, class T = decltype(*std::declval<Cont>().cbegin())>
std::vector<std::decay_t<T>> ToSortedPointers(const Cont& cont, Cmp&& cmp)
{
    std::vector<std::decay_t<T>> ans(cont.size());
    size_t i{0};
    for (auto elem: cont) {
        ans[i++] = elem;
    }
    std::stable_sort(ans.begin(), ans.end(), [&cmp](auto a, auto b) { return cmp(a, b); });
    return ans;
}

// ordered set is not sorted, return a reference to itself
template <class T>
const auto& ToSorted(const std::set<T>& cont)
{
    return cont;
}

template <class T, class K>
const auto& ToSorted(const std::map<T, K>& cont)
{
    return cont;
}

std::string TrimPackagePath(const std::string& path);
std::string GetTrimmedPath(AST::File* file);
} // namespace Codira::IncrementalCompilation
#endif
