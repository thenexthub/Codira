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

#include <algorithm>
#include <string>
#include "gtest/gtest.h"

#include "Codira/AST/ASTCasting.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Node.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;
using namespace AST;

class ASTToSourceTest : public testing::Test {
protected:
    void SetUp() override
    {
    }
#ifdef PROJECT_SOURCE_DIR
    // Gets the absolute path of the project from the compile parameter.
    std::string projectPath = PROJECT_SOURCE_DIR;
#else
    // Just in case, give it a default value.
    // Assume the initial is in the build directory.
    std::string projectPath = "..";
#endif
    std::string srcPath;
    DiagnosticEngine diag;
    SourceManager sm;
};

TEST_F(ASTToSourceTest, VarDeclToString)
{
    std::string srcVarDecl = R"(public   let a = "hello world!")";
    Parser* parser = new Parser(srcVarDecl, diag, sm);
    OwnedPtr<File> file = parser->ParseTopLevel();
    EXPECT_EQ(srcVarDecl, file->decls[0]->ToString());

    srcVarDecl = R"(var cc    = "hello world!")";
    parser = new Parser(srcVarDecl, diag, sm);
    file = parser->ParseTopLevel();
    EXPECT_EQ(srcVarDecl, file->decls[0]->ToString());

    srcVarDecl = R"(public

                      let

                    a :
                    String

                      =

                    "hello world!")";
    parser = new Parser(srcVarDecl, diag, sm);
    file = parser->ParseTopLevel();
    EXPECT_EQ(srcVarDecl, file->decls[0]->ToString());

    // Comments.
    srcVarDecl = R"(public/*foo*/   let a/*ty infer*/ = "hello world!")";
    parser = new Parser(srcVarDecl, diag, sm);
    file = parser->ParseTopLevel();
    Ptr<VarDecl> vd = RawStaticCast<VarDecl*>(file->decls[0].get());
    sm.AddSource("", srcVarDecl);
    Source& source = sm.GetSource(1);
    size_t origin = 0;
    auto comments = parser->GetCommentsMap()[0];
    std::unordered_map<size_t, Token> commentsInside;
    for (auto& comment : comments) {
        if (comment.Begin().line >= vd->end.line) {
            commentsInside.insert_or_assign(source.PosToOffset(comment.Begin()) - origin, comment);
        }
    }
    std::string result = vd->ToString();
    auto it = result.begin();
    for (auto& it1 : commentsInside) {
        int i = 0;
        for (auto& ch : it1.second.Value()) {
            *(it + it1.first + i) = ch;
            i++;
        }
    }
    EXPECT_EQ(result, srcVarDecl);
}

TEST_F(ASTToSourceTest, CallExprToString)
{
    std::string srcCallExpr = R"(systemlib.TitleBarObj(
    text: "Rune UI Demo",
    textColor: "#ffffff",
    backgroundColor: "#007dff",
    backgroundOpacity: 0.5,
    isMenu: true
))";
    Parser* parser = new Parser(srcCallExpr, diag, sm);
    OwnedPtr<Expr> ce = parser->ParseExpr();
    EXPECT_EQ(srcCallExpr, ce->ToString());
}

TEST_F(ASTToSourceTest, ArrayLitToString)
{
    std::string srcArrayLit = "[  20.px()  , 0.px(),   20.px(), 0.px()]";
    Parser* parser = new Parser(srcArrayLit, diag, sm);
    OwnedPtr<Expr> al = parser->ParseExpr();
    EXPECT_EQ(srcArrayLit, al->ToString());
}

TEST_F(ASTToSourceTest, ToStringCov)
{
    // NOTE: only for coverage now. 'ToString' method may be removed.
    auto pointerExpr = CreateUniquePtr<PointerExpr>();
    pointerExpr->type = CreateRefType("Type");
    pointerExpr->arg = CreateFuncArg(CreateRefExpr("name"));
    auto pStr = pointerExpr->ToString();
    EXPECT_FALSE(pStr.empty());

    std::string srcArrayExpr = "VArray(repeat: 1)";
    Parser* parser = new Parser(srcArrayExpr, diag, sm);
    OwnedPtr<Expr> al = parser->ParseExpr();
    EXPECT_FALSE(al->ToString().empty());
}
