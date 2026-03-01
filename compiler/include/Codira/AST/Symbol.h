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
 * This file declares Symbol related classes.
 */

#ifndef CODIRA_AST_SYMBOL_H
#define CODIRA_AST_SYMBOL_H

#include <atomic>
#include <string>

#include "Codira/AST/Node.h"

namespace Codira {
class Collector;
struct HashID {
    uint64_t hash64 = 0;
    uint32_t fieldID = 0;
    friend bool operator==(const HashID& lhs, const HashID& rhs)
    {
        return std::tie(lhs.hash64, lhs.fieldID) == std::tie(rhs.hash64, rhs.fieldID);
    }
    friend bool operator<(const HashID& lhs, const HashID& rhs)
    {
        return std::tie(lhs.hash64, lhs.fieldID) < std::tie(rhs.hash64, rhs.fieldID);
    }
};

namespace AST {
static std::unordered_map<ASTKind, std::string> ASTKIND_TO_STRING_MAP{
#define ASTKIND(KIND, VALUE, NODE, SIZE) {ASTKind::KIND, VALUE},
#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
};

class SymbolApi {
public:
    static HashID NextHashID(uint64_t fileHash)
    {
        ids++;
        HashID hashIDs{
            .hash64 = fileHash,
            .fieldID = ids
        };
        return hashIDs;
    }
    static void ResetID()
    {
        ids = -1u;
    }

private:
    inline static std::atomic_uint32_t ids = -1u;
};

struct Symbol {
    const std::string name;               /**< Symbol name. */
    Symbol* const id{nullptr};            /**< Symbol id. */
    Ptr<Node> const node{nullptr};            /**< AST node. */
    const HashID hashID{0, 0};            /**< Combine file content hash id and symbol id. */
    const uint32_t scopeLevel{0};         /**< Scope level, toplevel scope is 0. */
    const std::string scopeName;          /**< Managed by ScopeManager. */
    const ASTKind astKind{ASTKind::NODE}; /**< AST kind, for quick filter. */
    Ptr<Decl> target{nullptr};                /**< Target for all ref symbol. */
    bool invertedIndexBeenDeleted{false}; /**< Mark whether inverted index has been deleted. */
    void UnbindTarget()
    {
        target = nullptr;
    }
private:
    // Only allow 'Codira::Collector' to create symbol.
    friend class Codira::Collector;
    Symbol(uint64_t fileHash, std::string name, Node& src, uint32_t scopeLevel, std::string scopeName)
        : name(std::move(name)), id(this), node(&src), hashID(SymbolApi::NextHashID(fileHash)),
          scopeLevel(scopeLevel), scopeName(std::move(scopeName)), astKind(src.astKind)
    {
    }
    Symbol(const Symbol& sym) = delete;
};
} // namespace AST
} // namespace Codira
#endif // CODIRA_AST_SYMBOL_H
