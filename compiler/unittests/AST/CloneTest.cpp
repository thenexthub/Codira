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

#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "Codira/AST/Clone.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/PrintNode.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;
using namespace AST;

class CloneTest : public testing::Test {
protected:
    void SetUp() override
    {
        parser = new Parser(code, diag, sm);
        file = parser->ParseTopLevel();
    }
    std::string code = R"(
        let clockPort   = 12
        let dataPort    = 5
        let ledNum      = 64         // led number
        let lightColor  = 0xffff0000 // led light color -> b: 255, g: 0, r: 0

        // for LED show
        var pos : int   = 0          // LED position
        var leds : int[]

        // c libary api ======= fake FFI
        func print() : unit {}
        func print(str : String) : unit {}
        func sleep(inv : int) : unit {}
        func OpenGPIO(pin : int) : unit {}
        func WriteGPIO(pin : int, val : int) : unit {}
        func SetWord(clkPort : int, dataPort : int, val : int) : unit {}

        // Util function
        func CDW(val : int) : unit {
            SetWord(clockPort, dataPort, val)
        }

        // Set the global array
        func SetChaserPattern() : unit {
            leds[pos] = lightColor
            pos = (pos + 1) % ledNum;
        }

        // Show LED: Right Shift Zero
        func ShowLED(leds : int, lightPWM : int) : unit {
            CDW(0)
            lightPWM = 0xFF000000
            CDW(0xffffffff)
        }

        func StartChaserMode() {
            while (true) {
                SetChaserPattern()
                ShowLED(leds, 0xFF000000)
                sleep(50) // fake sleep
            }
        }

        main() : int {
            print("hello world")

            // Initialize GPIO
            OpenGPIO(clockPort)
            WriteGPIO(clockPort, 1)

            OpenGPIO(dataPort)

            // Show LED
            print("Start Marquee...")
            StartChaserMode()
            return 0
        }
)";
    Parser* parser;
    DiagnosticEngine diag;
    SourceManager sm;
    OwnedPtr<File> file;
};

namespace {
/// Match AST nodes according to Node Type, and return matched nodes.
template <typename T> std::vector<Ptr<Node>> MatchASTByNode(Ptr<Node> node)
{
    std::vector<Ptr<Node>> ret{};
    if (!node) {
        return ret;
    }
    Walker walker(node, [&ret](Ptr<Node> node) -> VisitAction {
        if (dynamic_cast<T*>(node.get())) {
            ret.push_back(node);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
    return ret;
}
} // namespace

TEST_F(CloneTest, CloneExpr)
{
    std::vector<Ptr<Node>> binaryExprs = MatchASTByNode<BinaryExpr>(file.get());
    for (auto& it : binaryExprs) {
        PrintNode(ASTCloner::Clone(Ptr(As<ASTKind::EXPR>(it))).get());
        EXPECT_TRUE(Is<BinaryExpr>(ASTCloner::Clone(Ptr(As<ASTKind::EXPR>(it))).get()));
    }
    std::vector<Ptr<Node>> callExprs = MatchASTByNode<CallExpr>(file.get());
    for (auto& it : callExprs) {
        PrintNode(ASTCloner::Clone(Ptr(As<ASTKind::EXPR>(it))).get());
        EXPECT_TRUE(Is<CallExpr>(ASTCloner::Clone(Ptr(As<ASTKind::EXPR>(it))).get()));
    }
}

TEST_F(CloneTest, CloneDecl)
{
    std::vector<Ptr<Node>> varDecls = MatchASTByNode<VarDecl>(file.get());
    for (auto& it : varDecls) {
        PrintNode(ASTCloner::Clone(Ptr(As<ASTKind::DECL>(it))).get());
        EXPECT_TRUE(Is<VarDecl>(ASTCloner::Clone(Ptr(As<ASTKind::DECL>(it))).get()));
    }
    std::vector<Ptr<Node>> funcDecls = MatchASTByNode<FuncDecl>(file.get());
    for (auto& it : funcDecls) {
        PrintNode(ASTCloner::Clone(Ptr(As<ASTKind::DECL>(it))).get());
        EXPECT_TRUE(Is<FuncDecl>(ASTCloner::Clone(Ptr(As<ASTKind::DECL>(it))).get()));
    }
}

TEST_F(CloneTest, CloneBlock)
{
    std::vector<Ptr<Node>> blocks = MatchASTByNode<Block>(file.get());
    for (auto& it : blocks) {
        PrintNode(it);
        PrintNode(ASTCloner::Clone(Ptr(As<ASTKind::BLOCK>(it))).get());
        EXPECT_TRUE(Is<Block>(ASTCloner::Clone(Ptr(As<ASTKind::BLOCK>(it))).get()));
    }
}
