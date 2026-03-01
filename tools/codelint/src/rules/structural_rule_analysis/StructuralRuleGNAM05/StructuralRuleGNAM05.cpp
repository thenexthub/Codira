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

#include <regex>
#include "StructuralRuleGNAM05.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * The names of immutable global variables or static variables must be in uppercase.
 * right eg:
 * MAXNUM, STRINGNAME
 * wrong eg:
 * maxNum, MaxNum
 */
static const std::string ALL_CAPITAL_LETTERS = "^[A-Z_][A-Z0-9_]*";

void StructuralRuleGNAM05::CheckGlobalOrStaticVarWithLetName(const Codira::AST::VarDecl& varDecl)
{
    if (!regex_match(varDecl.identifier.Val(), std::regex(ALL_CAPITAL_LETTERS))) {
        Diagnose(varDecl.begin, varDecl.end, CodeCheckDiagKind::G_NAM_05_immutable_global_variable_naming_information,
            varDecl.identifier.Val());
    }
}

void StructuralRuleGNAM05::FindGlobalOrStaticVarWithLetName(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl& varDecl) {
                if (!varDecl.isVar &&
                    (varDecl.TestAttr(AST::Attribute::GLOBAL) || varDecl.TestAttr(AST::Attribute::STATIC))) {
                    CheckGlobalOrStaticVarWithLetName(varDecl);
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGNAM05::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindGlobalOrStaticVarWithLetName(node);
}
