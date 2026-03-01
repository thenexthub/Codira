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

#include "StructuralRuleGNAM04.h"
#include <regex>
#include "common/CommonFunc.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * lower camel case rule
 * When naming a variable or function name, the first word starts with a lowercase letter,
 * and the first letter of each word starts with an uppercase letter from the second word.
 * right eg:
 * checkFuncName , findFuncName ,
 * wrong eg:
 * CheckFuncName , FindFuncName , 33come
 */
static const std::string LOWER_CAMEL_CASE = "[a-z][A-Z0-9a-z]*";

void StructuralRuleGNAM04::CheckFuncName(const FuncDecl& funcDecl)
{
    if (funcDecl.IsFinalizer() || funcDecl.identifier == "__mainInvoke") {
        return;
    }
    
    if (funcDecl.keywordPos.ToString() == "(0, 0, 0)") {
        return;
    }
    for (auto mod : funcDecl.modifiers) {
        if (mod.ToString() == "operator" || mod.ToString() == "foreign") {
            return;
        }
    }
    if (!regex_match(funcDecl.identifier.Val(), std::regex(LOWER_CAMEL_CASE)) &&
        !CommonFunc::IsStdDerivedMacro(diagEngine, funcDecl.begin)) {
        Diagnose(funcDecl.begin, funcDecl.end, CodeCheckDiagKind::G_NAM_04_function_naming_information,
            funcDecl.identifier.Val());
    }
}

void StructuralRuleGNAM04::FindFuncName(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                CheckFuncName(funcDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const VarDecl&) { return VisitAction::SKIP_CHILDREN; }, []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGNAM04::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFuncName(node);
}
