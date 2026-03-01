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
#ifndef CODIRA_ASTCHECKER_H
#define CODIRA_ASTCHECKER_H
#include <unordered_map>

#include "Codira/AST/Node.h"
#include "Codira/Lex/Lexer.h"

#define AST_NULLPTR_CHECK(node, f)                                                                                     \
    do {                                                                                                               \
        if ((f) == nullptr) {                                                                                          \
            CollectInfo(node, #f);                                                                                     \
        }                                                                                                              \
    } while (0)

#define ATTR_NULLPTR_CHECK(node, f)                                                                                    \
    do {                                                                                                               \
        if ((f) == nullptr) {                                                                                          \
            CollectInfo(node, #f);                                                                                     \
        }                                                                                                              \
    } while (0)

#define ZERO_POSITION_CHECK(node, pos)                                                                                 \
    do {                                                                                                               \
        if ((pos).IsZero()) {                                                                                          \
            /** CollectInfo(node, #pos);            */                                                                 \
        }                                                                                                              \
    } while (0)

#define VEC_AST_NULLPTR_CHECK(node, vec)                                                                               \
    do {                                                                                                               \
        for (auto& f : (vec)) {                                                                                        \
            if (f == nullptr) {                                                                                        \
                CollectInfo(node, #vec);                                                                               \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

#define VEC_ZERO_POS_CHECK(node, vec)                                                                                  \
    do {                                                                                                               \
        for (auto& pos : (vec)) {                                                                                      \
            if (pos.IsZero()) {                                                                                        \
                /** CollectInfo(node, #vec);            */                                                             \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

#define VEC_EMPTY_STRING_CHECK(node, vec)                                                                              \
    do {                                                                                                               \
        for (auto& f : (vec)) {                                                                                        \
            if (f.empty()) {                                                                                           \
                /** CollectInfo(node, #vec);            */                                                             \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

#define EMPTY_STRING_CHECK(node, str)                                                                                  \
    do {                                                                                                               \
        if ((str).empty()) {                                                                                           \
            /** CollectInfo(node, #str);     */                                                                        \
        }                                                                                                              \
    } while (0)

#define EMPTY_IDENTIFIER_CHECK(node, id)                                                                               \
    do {                                                                                                               \
        if ((id).Empty() || (id).ZeroPos()) {                                                                          \
            /** CollectInfo(node, #id);  */                                                                            \
        }                                                                                                              \
    } while (0)
#define EMPTY_VEC_CHECK(node, vec)                                                                                     \
    do {                                                                                                               \
        if ((vec).empty()) {                                                                                           \
            /** CollectInfo(node, #vec);  */                                                                           \
        }                                                                                                              \
    } while (0)

namespace Codira::AST {
class ASTChecker {

public:
    void CheckAST(Node& node);
    void CheckAST(const std::vector<OwnedPtr<Package>>& pkgs);
    void CheckBeginEnd(Ptr<Node> node);
    void CheckBeginEnd(const std::vector<OwnedPtr<Package>>& pkgs);

private:
    std::unordered_map<ASTKind, std::function<void(ASTChecker*, Ptr<Node>)>> checkFuncMap{
#define ASTKIND(KIND, VALUE, NODE, SIZE) {ASTKind::KIND, &ASTChecker::Check##NODE},
#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
    };

// Function declarations of Checking several kinds of Node
#define ASTKIND(KIND, VALUE, NODE, SIZE) void Check##NODE(Ptr<Node> node);
#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
    
    std::set<std::string> checkInfoSet;
    void CollectInfo(Ptr<Node> node, const std::string& subInfo);

    void CheckInheritableDecl(Ptr<Node> node);
};
} // namespace Codira::AST

#endif // CODIRA_ASTCHECKER_H
