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

#include "Codira/AST/Create.h"

#include <iostream>

#include "gtest/gtest.h"

#include "Codira/AST/Match.h"
#include "Codira/AST/PrintNode.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;
using namespace AST;

namespace {
/** Create Expr node according to source code. */
OwnedPtr<Expr> CreateExprASTFromSrc(const std::string& src, DiagnosticEngine& diag, const Position& pos = {})
{
    Parser parser(src, diag, diag.GetSourceManager(), pos);
    return parser.ParseExpr();
}
} // namespace

TEST(CreateTest, CreateASTFromSrc)
{
    std::string src{"a=b+c"};
    DiagnosticEngine diag;
    SourceManager sm;
    sm.AddSource("./", src);
    diag.SetSourceManager(&sm);
    auto expr = CreateExprASTFromSrc(src, diag);
    PrintNode(expr.get());
    EXPECT_TRUE(Is<AssignExpr>(expr.get()));
}

TEST(CreateTest, CreateBinaryExpr)
{
    auto binaryExpr = CreateBinaryExpr(MakeOwned<RefExpr>(), MakeOwned<RefExpr>(), TokenKind::ADD);
    PrintNode(binaryExpr.get());
    EXPECT_TRUE(Is<BinaryExpr>(binaryExpr.get()));
}

TEST(CreateTest, CreateReturnExpr)
{
    auto expr1 = CreateBinaryExpr(MakeOwned<RefExpr>(), MakeOwned<RefExpr>(), TokenKind::ADD);
    auto expr2 = CreateReturnExpr(std::move(expr1));
    EXPECT_TRUE(Is<ReturnExpr>(expr2.get()));
}

TEST(CreateTest, CreateBlock)
{
    std::vector<OwnedPtr<Node>> nodes1;
    nodes1.emplace_back(nullptr);
    nodes1.emplace_back(nullptr);
    auto block1 = CreateBlock(std::move(nodes1));
    PrintNode(block1.get());
    EXPECT_TRUE(Is<Block>(block1.get()));

    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    std::vector<OwnedPtr<Node>> nodes2;
    nodes2.emplace_back(MakeOwned<RefExpr>());
    nodes2.emplace_back(CreateExprASTFromSrc("c+d*e**f/6", diag));
    nodes2.emplace_back(nullptr);
    nodes2.emplace_back(MakeOwned<RefExpr>());
    auto block2 = CreateBlock(std::move(nodes2));
    PrintNode(block2.get());
    EXPECT_TRUE(Is<Block>(block2.get()));
}

TEST(CreateTest, CreateFuncDecl)
{
    auto param1 = CreateFuncParam("a");
    auto param2 = CreateFuncParam("b", MakeOwned<RefType>());
    auto param3 = CreateFuncParam("c", MakeOwned<RefType>(), MakeOwned<RefExpr>());
    EXPECT_TRUE(Is<FuncParam>(param1.get()));
    EXPECT_TRUE(Is<FuncParam>(param2.get()));
    EXPECT_TRUE(Is<FuncParam>(param3.get()));

    auto paramList = CreateFuncParamList({param1.get(), param2.get(), param3.get()});
    EXPECT_TRUE(Is<FuncParamList>(paramList.get()));

    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    std::vector<OwnedPtr<Node>> nodes;
    nodes.emplace_back(MakeOwned<RefExpr>());
    nodes.emplace_back(CreateExprASTFromSrc("c+d*e**f/6", diag));
    nodes.emplace_back(nullptr);
    nodes.emplace_back(MakeOwned<RefExpr>());
    auto body = CreateBlock(std::move(nodes));
    EXPECT_TRUE(Is<Block>(body.get()));

    std::vector<OwnedPtr<FuncParamList>> paramLists;
    paramLists.emplace_back(std::move(paramList));
    auto funcBody = CreateFuncBody(std::move(paramLists), MakeOwned<RefType>(), std::move(body));
    EXPECT_TRUE(Is<FuncBody>(funcBody.get()));

    auto funcDecl = CreateFuncDecl("test", std::move(funcBody));
    EXPECT_TRUE(Is<FuncDecl>(funcDecl.get()));

    PrintNode(funcDecl.get());
}

#ifndef _WIN32
TEST(CreateTest, CheckASTSize)
{
    /**
     * This testcase is used to remind developer to update related methods in "Clone.cpp" "Create.cpp"
     * and other related project code like "fmt", "lsp", "codecheck" and other tools.
     * NOTE: If any node size check failed, please notify and check code which is mentioned above.
     *       After all related usages have been checked, just update the size written in "ASTKind.inc".
     */
    size_t size;
#define ASTKIND(KIND, VALUE, NODE, SIZE)                                                                               \
    do {                                                                                                               \
        size = sizeof(NODE);                                                                                           \
        EXPECT_EQ(size, SIZE);                                                                                         \
    } while (0);
#include "Codira/AST/ASTKind.inc"
#undef ASTKIND
}
#endif
