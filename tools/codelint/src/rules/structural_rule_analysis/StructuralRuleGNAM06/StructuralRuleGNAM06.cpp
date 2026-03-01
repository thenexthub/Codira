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

#include "StructuralRuleGNAM06.h"
#include <regex>

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

static const std::string CAMEL_CASE_PATTERN = "^(?!^[a-z]$)[a-z]+([A-Z][a-z]*)*$"; // Excluding single char
static const std::string GENERIC_PARAMETER_PATTERN = "^[^a-z]*$";                  // Generic Parameter

void StructuralRuleGNAM06::PrintDiagnoseInfo(SrcIdentifier& identifier, std::string pattern, CodeCheckDiagKind kind)
{
    if (!regex_match(identifier.Val(), std::regex(pattern))) {
        Diagnose(identifier.Begin(), identifier.End(), kind, identifier.Val());
    }
}

void StructuralRuleGNAM06::CheckCamelCaseFormat(Ptr<Node> node)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)(
            [this](const PropDecl& propDecl) {
                auto identifier = propDecl.identifier;
                PrintDiagnoseInfo(identifier, CAMEL_CASE_PATTERN, CodeCheckDiagKind::G_NAM_06_propertie_camelCase_name);
            },
            [this](const FuncParam& funcParam) {
                auto identifier = funcParam.identifier;
                PrintDiagnoseInfo(identifier, CAMEL_CASE_PATTERN, CodeCheckDiagKind::G_NAM_06_parameter_camelCase_name);
            },
            [this](const VarDecl& varDecl) {
                // Exception Scenario: Numeric Constants Used Within the Function
                if (!varDecl.isVar && (varDecl.ty->IsFloating() || varDecl.ty->IsInteger())) {
                    return;
                }
                auto identifier = varDecl.identifier;
                PrintDiagnoseInfo(identifier, CAMEL_CASE_PATTERN, CodeCheckDiagKind::G_NAM_06_var_camelCase_name);
            },
            [this](const GenericParamDecl& genericParamDecl) {
                auto identifier = genericParamDecl.identifier;
                PrintDiagnoseInfo(
                    identifier, GENERIC_PARAMETER_PATTERN, CodeCheckDiagKind::G_NAM_06_generic_type_parameters);
            });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGNAM06::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckCamelCaseFormat(node);
}
