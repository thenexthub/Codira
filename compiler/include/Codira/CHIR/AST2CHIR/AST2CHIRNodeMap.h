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
 * This file declares the symbol table of CHIR.
 */

#ifndef CODIRA_CHIR_SYMBOLTABLE_H
#define CODIRA_CHIR_SYMBOLTABLE_H

#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/CHIR/Value.h"
#include "Codira/Mangle/CHIRMangler.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira::CHIR {
template <typename T> class AST2CHIRNodeMap {
public:
    AST2CHIRNodeMap()
    {
    }
    ~AST2CHIRNodeMap() = default;
    bool Has(const Codira::AST::Node& node) const
    {
        return cache.find(&node) != cache.end();
    }

    void Set(const Codira::AST::Node& node, T& chirNode)
    {
        CODEC_ASSERT(cache.emplace(&node, &chirNode).second);
    }

    T* Get(const Codira::AST::Node& node) const
    {
        auto chirNode = cache.at(&node);
        CODEC_NULLPTR_CHECK(chirNode);
        return chirNode;
    }

    T* TryGet(const Codira::AST::Node& node) const
    {
        auto it = cache.find(&node);
        if (it != cache.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::unordered_map<const Codira::AST::Node*, T*>& GetALL() const
    {
        return cache;
    }

    void Erase(const Codira::AST::Node& node)
    {
        (void)cache.erase(&node);
    }

private:
    std::unordered_map<const Codira::AST::Node*, T*> cache;
};
} // namespace Codira::CHIR
#endif
