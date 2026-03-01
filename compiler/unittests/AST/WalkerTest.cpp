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

#include <vector>
#include "gtest/gtest.h"

#define private public
#include "Codira/AST/Match.h"
#include "Codira/AST/PrintNode.h"
#include "Codira/AST/Walker.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;
using namespace AST;

class WalkerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        std::string code = "main(argc : Int32, argv : Array<String>) {\n"
                           "	let a : Int = 40\n"
                           "	let b = 2 ** -a\n"
                           "	print((a + 3 * b, (a + 3) * b))\n"
                           "}\n";
        Parser parser(code, diag, sm);
        file = parser.ParseTopLevel();
    }

    DiagnosticEngine diag;
    SourceManager sm;
    OwnedPtr<File> file;
};

TEST_F(WalkerTest, WalkPair)
{
    int count = 0;

    Walker walker(
        file.get(),
        [&count](Ptr<Node> node) -> VisitAction {
            ++count;
            return VisitAction::WALK_CHILDREN;
        },
        [&count](Ptr<Node> node) -> VisitAction {
            --count;
            return VisitAction::WALK_CHILDREN;
        });
    walker.Walk();

    EXPECT_EQ(0, count);
}

TEST_F(WalkerTest, WalkPairSkipChildren)
{
    int count = 0;

    Walker walker(
        file.get(),
        [&count](Ptr<Node> node) -> VisitAction {
            ++count;
            return VisitAction::SKIP_CHILDREN;
        },
        [&count](Ptr<Node> node) -> VisitAction {
            --count;
            return VisitAction::WALK_CHILDREN;
        });
    walker.Walk();

    EXPECT_EQ(0, count);
}

TEST_F(WalkerTest, WalkPairStopNow)
{
    int count = 0;

    Walker walker(
        file.get(),
        [&count](Ptr<Node> node) -> VisitAction {
            ++count;
            return VisitAction::STOP_NOW;
        },
        [&count](Ptr<Node> node) -> VisitAction {
            --count;
            return VisitAction::WALK_CHILDREN;
        });
    walker.Walk();

    EXPECT_EQ(1, count);
}

TEST_F(WalkerTest, WalkShareID)
{
    // Walker and ConstWalker must share same counter.
    Walker::nextWalkerID = 1;
    ConstWalker::nextWalkerID = 1;
    auto id1 = Walker(file.get()).ID;
    auto id2 = ConstWalker(file.get()).ID;
    EXPECT_NE(id1, id2);
}

TEST_F(WalkerTest, GetDecls)
{
    std::vector<std::string> identifiers;

    Walker walker(file.get(), [&identifiers](Ptr<Node> node) -> VisitAction {
        if (auto decl = AST::As<ASTKind::DECL>(node); decl) {
            identifiers.push_back(decl->identifier);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();

    std::string expectedIdentifiers[] = {"main", "argc", "argv", "a", "b"};

    ASSERT_EQ(std::size(expectedIdentifiers), identifiers.size());
    for (size_t i = 0; i < identifiers.size(); i++) {
        EXPECT_EQ(expectedIdentifiers[i], identifiers[i]);
    }
}

TEST_F(WalkerTest, GetDeclsPost)
{
    std::vector<std::string> identifiers;

    Walker walker(file.get(), nullptr, [&identifiers](Ptr<Node> node) -> VisitAction {
        if (auto decl = AST::As<ASTKind::DECL>(node); decl) {
            identifiers.push_back(decl->identifier);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();

    std::string expectedIdentifiers[] = {"argc", "argv", "a", "b", "main"};

    ASSERT_EQ(std::size(expectedIdentifiers), identifiers.size());
    for (size_t i = 0; i < identifiers.size(); i++) {
        EXPECT_EQ(expectedIdentifiers[i], identifiers[i]);
    }
}

TEST_F(WalkerTest, GetCallExprs)
{
    std::vector<std::string> callExprNames;

    Walker walker(file.get(), [&callExprNames](Ptr<Node> node) -> VisitAction {
        if (auto ce = AST::As<ASTKind::CALL_EXPR>(node); ce) {
            if (auto re = AST::As<ASTKind::REF_EXPR>(ce->baseFunc.get()); re) {
                callExprNames.push_back(re->ref.identifier);
            }
        }
        return VisitAction::WALK_CHILDREN;
    });

    walker.Walk();

    std::string expectedCallExprNames[] = {"print"};

    ASSERT_EQ(std::size(expectedCallExprNames), callExprNames.size());
    for (size_t i = 0; i < callExprNames.size(); i++) {
        EXPECT_EQ(expectedCallExprNames[i], callExprNames[i]);
    }
}
