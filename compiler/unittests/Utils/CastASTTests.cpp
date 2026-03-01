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

#include <cstdio>
#include <fstream>
#include <string>

#include "gtest/gtest.h"

#define private public
#include "Codira/AST/Node.h"
#include "Codira/Basic/DiagnosticEngine.h"

#ifdef _WIN32
#define STATIC_ASSERT static_assert
#else
struct DummyExpr : Codira::AST::OverloadableExpr {
    DummyExpr() : Codira::AST::OverloadableExpr(Codira::AST::ASTKind::INVALID_EXPR)
    {
    }
};
// Replace the macro defined in 'ASTCasting.h'
#define STATIC_ASSERT(CONDITION) EXPECT_TRUE((std::is_same_v<To, DummyExpr>) || (CONDITION))
#endif

#include "Codira/AST/ASTCasting.h"

using namespace Codira;

template <typename T> constexpr bool IgnoredType()
{
    return ShouldInstantiate<T, AST::Decl, AST::Expr, AST::Node, AST::Modifier, AST::ClassLikeDecl, AST::BuiltInDecl,
        AST::PackageDecl, AST::InvalidDecl, AST::Pattern, AST::VarOrEnumPattern, AST::ThisType, AST::InvalidType,
        AST::PrimitiveTypeExpr, AST::IfAvailableExpr>::value;
}

static const std::vector<AST::ASTKind> ignoredKind = {
    AST::ASTKind::DECL,
    AST::ASTKind::EXPR,
    AST::ASTKind::NODE,
    AST::ASTKind::PATTERN,
    AST::ASTKind::CLASS_LIKE_DECL,
    AST::ASTKind::INVALID_DECL,
    AST::ASTKind::INVALID_TYPE,
};

class CastASTTests : public testing::Test {
protected:
    static void SetUpTestCase()
    {
#define ASTKIND(KIND, VALUE, TYPE, SIZE)                                                                               \
    if constexpr (IgnoredType<AST::TYPE>()) {                                                                          \
        auto nodePtr = MakeOwned<AST::TYPE>();                                                                         \
        astMap.emplace(AST::ASTKind::KIND, nodePtr.get());                                                             \
        astPool.emplace_back(std::move(nodePtr));                                                                      \
    }
#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
        astPool.emplace_back(MakeOwned<AST::Modifier>(TokenKind::OPEN, DEFAULT_POSITION));
        astMap.emplace(AST::ASTKind::MODIFIER, astPool.back().get());

        astPool.emplace_back(MakeOwned<AST::BuiltInDecl>(AST::BuiltInType::ARRAY));
        astMap.emplace(AST::ASTKind::BUILTIN_DECL, astPool.back().get());

        auto pkg = RawStaticCast<AST::Package*>(astMap[AST::ASTKind::PACKAGE]);
        astPool.emplace_back(MakeOwned<AST::PackageDecl>(*pkg));
        astMap.emplace(AST::ASTKind::PACKAGE_DECL, astPool.back().get());

        astPool.emplace_back(MakeOwned<AST::VarOrEnumPattern>(SrcIdentifier{"E"}));
        astMap.emplace(AST::ASTKind::VAR_OR_ENUM_PATTERN, astPool.back().get());

        astPool.emplace_back(MakeOwned<AST::ThisType>(DEFAULT_POSITION));
        astMap.emplace(AST::ASTKind::THIS_TYPE, astPool.back().get());

        astPool.emplace_back(MakeOwned<AST::PrimitiveTypeExpr>(AST::TypeKind::TYPE_INT64));
        astMap.emplace(AST::ASTKind::PRIMITIVE_TYPE_EXPR, astPool.back().get());

        auto arg = MakeOwned<AST::FuncArg>();
        arg->name = "APILevel";
        arg->expr = MakeOwned<AST::LitConstExpr>();
        astPool.emplace_back(MakeOwned<AST::IfAvailableExpr>(
            std::move(arg), MakeOwned<AST::LambdaExpr>(), MakeOwned<AST::LambdaExpr>()));
        astMap.emplace(AST::ASTKind::IF_AVAILABLE_EXPR, astPool.back().get());
    }

    static std::map<AST::ASTKind, Ptr<AST::Node>> astMap;
    static std::vector<OwnedPtr<AST::Node>> astPool;
};

std::map<AST::ASTKind, Ptr<AST::Node>> CastASTTests::astMap = {};
std::vector<OwnedPtr<AST::Node>> CastASTTests::astPool = {};

TEST_F(CastASTTests, VerifyCastingCount)
{
    // Expect casting size is hardcode, should be changed manually when any new AST node is added.
    // If the new added type is an intermediate type, it may should be added into 'IgnoredType()'.
    // NOTE: 'NODE' is the lase enum value in 'ASTKind'.
    size_t totalCount = static_cast<uint8_t>(AST::ASTKind::NODE) + 1;
    EXPECT_TRUE(totalCount > ignoredKind.size());
    size_t size = totalCount - ignoredKind.size();
    EXPECT_EQ(size, 107);
    EXPECT_EQ(astPool.size(), size);

    // Added node's kind should same with key.
    for (auto [kind, node] : astMap) {
        EXPECT_EQ(node->astKind, kind);
    }
}

TEST_F(CastASTTests, VerifyMonoCasting)
{
    for (auto [_, ptrNode] : astMap) {
        auto node = ptrNode.get();
        const AST::Node* constNode = node;
#define ASTKIND(KIND, VALUE, TYPE, SIZE)                                                                               \
    EXPECT_EQ(DynamicCast<AST::TYPE>(node), dynamic_cast<AST::TYPE*>(node));                                           \
    EXPECT_EQ(DynamicCast<AST::TYPE*>(node), dynamic_cast<AST::TYPE*>(node));                                          \
    EXPECT_EQ(DynamicCast<AST::TYPE>(constNode), dynamic_cast<const AST::TYPE*>(constNode));                           \
    EXPECT_EQ(DynamicCast<AST::TYPE*>(constNode), dynamic_cast<const AST::TYPE*>(constNode));                          \
    EXPECT_EQ(Is<AST::TYPE*>(node), (dynamic_cast<AST::TYPE*>(node) != nullptr));                                      \
    EXPECT_EQ(Is<AST::TYPE>(node), (dynamic_cast<AST::TYPE*>(node) != nullptr));

#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
    }
}

TEST_F(CastASTTests, VerifyIntermediateCasting)
{
    for (auto [_, ptrNode] : astMap) {
        auto node = ptrNode.get();
        EXPECT_EQ(DynamicCast<AST::Expr*>(node), dynamic_cast<AST::Expr*>(node));
        EXPECT_EQ(DynamicCast<AST::InheritableDecl*>(node), dynamic_cast<AST::InheritableDecl*>(node));
        EXPECT_EQ(DynamicCast<AST::VarDeclAbstract*>(node), dynamic_cast<AST::VarDeclAbstract*>(node));
        EXPECT_EQ(DynamicCast<AST::OverloadableExpr*>(node), dynamic_cast<AST::OverloadableExpr*>(node));
        EXPECT_EQ(DynamicCast<AST::NameReferenceExpr*>(node), dynamic_cast<AST::NameReferenceExpr*>(node));
        EXPECT_EQ(Is<AST::InheritableDecl*>(node), dynamic_cast<AST::InheritableDecl*>(node) != nullptr);
        EXPECT_EQ(Is<AST::VarDeclAbstract*>(node), dynamic_cast<AST::VarDeclAbstract*>(node) != nullptr);
        EXPECT_EQ(Is<AST::OverloadableExpr*>(node), dynamic_cast<AST::OverloadableExpr*>(node) != nullptr);
        EXPECT_EQ(Is<AST::NameReferenceExpr*>(node), dynamic_cast<AST::NameReferenceExpr*>(node) != nullptr);
        EXPECT_EQ(Is<AST::NameReferenceExpr>(node), dynamic_cast<AST::NameReferenceExpr*>(node) != nullptr);
        EXPECT_EQ(Is<AST::NameReferenceExpr>(*node), dynamic_cast<AST::NameReferenceExpr*>(node) != nullptr);
    }
}

#ifndef _WIN32
// Only test in linux
// If the 'NodeType<DummyExpr>' is not defined, the 'DynamicCast<DummyExpr*>' will fail to be compiled.
template <> struct NodeType<DummyExpr> {
    static const AST::ASTKind kind = AST::ASTKind::NODE;
};
// Test for the template can guard new added type.
TEST_F(CastASTTests, TryAddNewType)
{
    // Converting to new type without specific instantiation of 'SubTypeChecker' will trigger 'static_assert'.
    DynamicCast<DummyExpr*>(astMap[AST::ASTKind::SUBSCRIPT_EXPR]);
    EXPECT_FALSE(NodeType<DummyExpr>::kind != AST::ASTKind::NODE);
}
#endif

TEST_F(CastASTTests, VerifyStaticCasting)
{
    std::map<AST::ASTKind, Ptr<AST::Node>>::iterator it;
#define ASTKIND(KIND, VALUE, TYPE, SIZE)                                                                               \
    it = astMap.find(AST::ASTKind::KIND);                                                                              \
    if (it != astMap.end()) {                                                                                          \
        EXPECT_TRUE(StaticCast<AST::TYPE>(it->second.get()) != nullptr);                                               \
        EXPECT_TRUE(StaticCast<AST::TYPE*>(it->second.get()) != nullptr);                                              \
        StaticCast<AST::TYPE&>(*it->second.get());                                                                     \
        const AST::Node* constNode = it->second.get();                                                                 \
        EXPECT_TRUE(StaticCast<AST::TYPE>(constNode) != nullptr);                                                      \
        EXPECT_TRUE(StaticCast<AST::TYPE*>(constNode) != nullptr);                                                     \
        auto& n1 = StaticCast<AST::TYPE&>(*constNode);                                                                 \
        auto& n2 = StaticCast<AST::TYPE>(*constNode);                                                                  \
        EXPECT_TRUE(&n1 == &n2);                                                                                       \
    }

#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
}
