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

#include "StructuralRuleGSYN01.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

constexpr const char* JSONPATH = "/config/structural_rule_G_SYN_01.json";

void StructuralRuleGSYN01::InitDisabledSyntaxCheckMap()
{
    disabledSyntaxCheckMap["Import"][AST::ASTKind::IMPORT_SPEC] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Let"][AST::ASTKind::VAR_DECL] = handleLet;
    // Disable check for if-let syntax, such as: if(let Some(v) <- x) {...}
    disabledSyntaxCheckMap["Let"][AST::ASTKind::LET_PATTERN_DESTRUCTOR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Spawn"][AST::ASTKind::SPAWN_EXPR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Synchronized"][AST::ASTKind::SYNCHRONIZED_EXPR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Main"][AST::ASTKind::MAIN_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["MacroQuote"][AST::ASTKind::MACRO_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Foreign"][AST::ASTKind::FUNC_DECL] = handleForeignFunc;
    disabledSyntaxCheckMap["Foreign"][AST::ASTKind::BLOCK] = handleForeignBlock;
    disabledSyntaxCheckMap["While"][AST::ASTKind::WHILE_EXPR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["While"][AST::ASTKind::DO_WHILE_EXPR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Extend"][AST::ASTKind::EXTEND_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Type"][AST::ASTKind::TYPE_ALIAS_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Operator"][AST::ASTKind::FUNC_DECL] = handleOperator;
    disabledSyntaxCheckMap["GlobalVariable"][AST::ASTKind::VAR_DECL] = handleGlobalVariable;
    disabledSyntaxCheckMap["Enum"][AST::ASTKind::ENUM_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Class"][AST::ASTKind::CLASS_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Interface"][AST::ASTKind::INTERFACE_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Struct"][AST::ASTKind::STRUCT_DECL] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["Match"][AST::ASTKind::MATCH_EXPR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["TryCatch"][AST::ASTKind::TRY_EXPR] = handleRestrictedSyntaxNode;
    disabledSyntaxCheckMap["HigherOrderFunc"][AST::ASTKind::FUNC_DECL] = handleHigherOrderFunc;
    disabledSyntaxCheckMap["ContainerType"][AST::ASTKind::REF_EXPR] = handleContainerTypeRefExpr;
    disabledSyntaxCheckMap["ContainerType"][AST::ASTKind::REF_TYPE] = handleContainerTypeRefType;
    for (auto decl = AST::ASTKind::DECL; decl <= AST::ASTKind::INVALID_DECL;
         decl = static_cast<AST::ASTKind>(static_cast<std::underlying_type<AST::ASTKind>::type>(decl) + 1)) {
        disabledSyntaxCheckMap["Generic"][decl] = handleGeneric;
        disabledSyntaxCheckMap["When"][decl] = handleWhen;
    }
    disabledSyntaxCheckMap["Generic"][AST::ASTKind::FUNC_BODY] = handleGenericFuncBody;
}

// Collect keywords for disabling syntax
void StructuralRuleGSYN01::CollectDisabledSyntax()
{
    std::unordered_set<std::string> disabledSyntaxKeywords;
    if (!jsonInfo.contains("SyntaxKeyword")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    // Ensure consistency with the parameter requirements for the JsonArrayStringValueGet() interface
    for (const auto& item : jsonInfo["SyntaxKeyword"]) {
        std::string keyWord = item.get<std::string>();
        disabledSyntaxKeywords.insert(keyWord);
    }
    for (auto& item : disabledSyntaxKeywords) {
        if (disabledSyntaxCheckMap.count(item) != 0) {
            syntaxList.insert(item);
        } else if (disabledSyntaxInSemaPhase.count(item) == 0) {
            std::cout << "This syntax check '" + item +
                    "' is not supported, please check whether the keyword is spelled correctly."
                      << std::endl;
        }
    }
}

void StructuralRuleGSYN01::FindSyntax(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        // Check whether the syntax is violated based on the check on some kind of AST nodes.
        for (auto& syntaxName : syntaxList) {
            if (disabledSyntaxCheckMap[syntaxName].count(node->astKind) > 0) {
                disabledSyntaxCheckMap[syntaxName][node->astKind](*node, syntaxName);
            }
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGSYN01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    InitDisabledSyntaxCheckMap();
    CollectDisabledSyntax();
    FindSyntax(node);
}
} // namespace Codira::CodeCheck
