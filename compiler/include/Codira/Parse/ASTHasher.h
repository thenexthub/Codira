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
 */
#ifndef CODIRA_ASTHASHER_H
#define CODIRA_ASTHASHER_H
#include <unordered_map>

#include "Codira/AST/Node.h"
#include "Codira/Lex/Lexer.h"

namespace Codira::AST {
class ASTHasher {
public:
    // pass global options to ASTHasher before creating any instance of ASTHasher
    static void Init(const GlobalOptions& op);

    using hash_type = size_t;
    static hash_type HashWithPos(Ptr<const Node> node);
    static hash_type HashNoPos(Ptr<const Node> node);
    static hash_type HashDeclBody(Ptr<const AST::Decl> decl);
    static hash_type HashDeclSignature(Ptr<const AST::Decl> decl);
    // hash package specs and import specs of package
    static hash_type HashSpecs(const Package& pk);

    static inline size_t CombineHash(const size_t acc, const size_t value)
    {
        // 6, 2 are specific constants in the hash algorithm.
        return acc ^ (value + 0x9e3779b9 + (acc << 6) + (acc >> 2));
    }

    // incr 2.0
    static hash_type SigHash(const AST::Decl& decl);
    static hash_type SrcUseHash(const AST::Decl& decl);
    // hashAnnos true to consider/ignore the annotations in the hash computation
    static hash_type BodyHash(const AST::Decl& decl, const std::pair<bool, bool>& srcInfo, bool hashAnnos = true);
    static hash_type ImportedDeclBodyHash(const AST::Decl& decl);
    static hash_type VirtualHash(const Decl& decl);
    static hash_type HashMemberAPIs(std::vector<Ptr<const Decl>>&& memberAPIs);
};
} // namespace Codira::AST

#endif // CODIRA_ASTHASHER_H
