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
 * This file declares cache for type check.
 */

#ifndef CODIRA_AST_CACHE_H
#define CODIRA_AST_CACHE_H

#include "Codira/AST/Types.h"
#include "Codira/Basic/DiagnosticEngine.h"

namespace Codira::AST {

using TargetCache = std::pair<Ptr<AST::Decl>, Ptr<AST::Decl>>;

struct CacheEntry {
    bool successful = false;
    Ptr<AST::Ty> result = nullptr;
    DiagnosticCache diags;
    TargetCache targets;
};

struct CacheKey {
    Ptr<AST::Ty> target;
    bool isDesugared;
    DiagnosticCache::DiagCacheKey diagKey;
    bool operator==(const CacheKey& b) const;
};

struct CacheKeyHash {
    size_t operator()(const CacheKey& key) const
    {
        auto v = std::hash<Ptr<AST::Ty>>()(key.target);
        v = hash_combine(v, key.isDesugared);
        v = hash_combine(v, key.diagKey);
        return v;
    }
};

/* type check cache for one AST node */
struct TypeCheckCache {
    std::unordered_map<CacheKey, CacheEntry, CacheKeyHash> synCache;
    std::unordered_map<CacheKey, CacheEntry, CacheKeyHash> chkCache;
    std::optional<CacheKey> lastKey{};
};

// member signature information available by just syntax check
struct MemSig {
    std::string id;
    bool isVarOrProp;
    size_t arity = 0; // arity in case of member function, otherwise 0; variadic arg not considered
    size_t genArity = 0; // number of possible explicit generic args in case of member function, otherwise 0
                         // note that all generic func can possibly have 0 explicit gen args
    bool operator==(const MemSig& b) const;
};

struct MemSigHash {
    size_t operator()(const MemSig& sig) const
    {
        auto v = std::hash<std::string>()(sig.id);
        v = hash_combine(v, sig.isVarOrProp);
        v = hash_combine(v, sig.arity);
        v = hash_combine(v, sig.genArity);
        return v;
    }
};

// Collect and restore necessary target decls in the sub-tree.
// Most targets are needed only after post-check, when they are filled by the normal procedure.
// Currently, this cache is only for checking enum constructor without type args.
TargetCache CollectTargets(const AST::Node& node);
void RestoreTargets(AST::Node& node, const TargetCache& targets);
} // namespace Codira

#endif
