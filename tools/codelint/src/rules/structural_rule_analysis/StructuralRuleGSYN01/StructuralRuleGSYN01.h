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

#ifndef CODIRACODECHECK_STRUCTURAL_RULE_G_SYN_01_H
#define CODIRACODECHECK_STRUCTURAL_RULE_G_SYN_01_H

#include <fstream>
#include <iostream>
#include "Codira/Basic/Match.h"
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGSYN01 : public StructuralRule {
public:
    explicit StructuralRuleGSYN01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGSYN01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    std::set<std::string> disabledConatinerType = {"TreeMap", "HashMap", "HashSet", "ArrayList"};
    std::set<std::string> disabledSyntaxInSemaPhase = {"PrimitiveType"};
    using NodeHandler = std::function<void(const AST::Node&, const std::string&)>;
    std::unordered_map<std::string, std::unordered_map<AST::ASTKind, NodeHandler>> disabledSyntaxCheckMap;
    std::unordered_set<std::string> syntaxList = {};
    void CollectDisabledSyntax();
    void FindSyntax(Ptr<Codira::AST::Node> node);
    void InitDisabledSyntaxCheckMap();
    std::unordered_map<std::string, std::string> syntaxNameMap = {{"Import", "import package"}, {"Let", "let variable"},
        {"Spawn", "spawn"}, {"Synchronized", "synchronized"}, {"Main", "main function"},
        {"MacroQuote", "macro definition"}, {"Foreign", "cross-language"}, {"While", "while loop"},
        {"Extend", "extend"}, {"Type", "type alias"}, {"Operator", "operator overloading"},
        {"GlobalVariable", "global variable"}, {"Enum", "enum definition"}, {"Class", "class definition"},
        {"Struct", "struct definition"}, {"Interface", "interface definition"}, {"Generic", "generic definition"},
        {"When", "conditional compilation"}, {"Differentiable", "auto differentiation"}, {"Match", "pattern matching"},
        {"TryCatch", "catch exceptions"}, {"HigherOrderFunc", "higher order function"}};
    /* @brief Disable let-modified variables */
    NodeHandler handleLet = [this](const AST::Node& node, const std::string& syntax) {
        auto varDecl = static_cast<const AST::VarDecl*>(&node);
        // Identify implicit let variables
        auto pos = varDecl->keywordPos;
        bool isImplict = pos.fileID == 0 && pos.line == 0 && pos.column == 0;
        if (!isImplict && !varDecl->isVar) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief For certain disabled syntax, the appearance of a specific type of AST node triggers a warning */
    NodeHandler handleRestrictedSyntaxNode = [this](const AST::Node& node, const std::string& syntax) {
        Diagnose(
            node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
    };
    /* @brief Check if there is operator overloading */
    NodeHandler handleOperator = [this](const AST::Node& node, const std::string& syntax) {
        auto funcDecl = static_cast<const AST::FuncDecl*>(&node);
        if (funcDecl->TestAttr(AST::Attribute::OPERATOR)) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief Check if there is interaction with other programming languages */
    NodeHandler handleForeignFunc = [this](const AST::Node& node, const std::string& syntax) {
        auto funcDecl = static_cast<const AST::FuncDecl*>(&node);
        auto it = std::find_if(funcDecl->modifiers.begin(), funcDecl->modifiers.end(),
            [](const auto& mod) { return mod.modifier == TokenKind::FOREIGN; });
        if (it != funcDecl->modifiers.end()) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief Check if there is interaction with other programming languages */
    NodeHandler handleForeignBlock = [this](const AST::Node& node, const std::string& syntax) {
        auto block = static_cast<const AST::Block*>(&node);
        if (block->TestAttr(AST::Attribute::UNSAFE)) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief Check whether global variables exist */
    NodeHandler handleGlobalVariable = [this](const AST::Node& node, const std::string& syntax) {
        auto varDecl = static_cast<const AST::VarDecl*>(&node);
        if (varDecl->TestAttr(AST::Attribute::GLOBAL)) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief Check if generics are used */
    NodeHandler handleGeneric = [this](const AST::Node& node, const std::string& syntax) {
        if (auto decl = dynamic_cast<const AST::Decl*>(&node);
            decl && decl->generic && !decl->generic->typeParameters.empty()) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief Check if generics are used */
    NodeHandler handleGenericFuncBody = [this](const AST::Node& node, const std::string& syntax) {
        if (auto body = dynamic_cast<const AST::FuncBody*>(&node);
            body && body->generic && !body->generic->typeParameters.empty()) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01, syntaxNameMap[syntax]);
        }
    };
    /* @brief Check whether conditional compilation is used */
    NodeHandler handleWhen = [this](const AST::Node& node, const std::string& syntax) {
        if (auto decl = dynamic_cast<const AST::Decl*>(&node); decl) {
            auto it = std::find_if(decl->annotations.begin(), decl->annotations.end(),
                [](const auto& anno) { return anno->kind == AST::AnnotationKind::WHEN; });
            if (it != decl->annotations.end()) {
                Diagnose(node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01,
                    syntaxNameMap[syntax]);
            }
        }
    };
    /* @brief Check whether higher-order functions are used */
    NodeHandler handleHigherOrderFunc = [this](const AST::Node& node, const std::string& syntax) {
        auto funcDecl = static_cast<const AST::FuncDecl*>(&node);
        for (auto& param : funcDecl->funcBody->paramLists[0]->params) {
            auto& type = param->type;
            if (auto funcType = dynamic_cast<AST::FuncType*>(type.operator->()); funcType) {
                Diagnose(param->begin, param->end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01,
                    syntaxNameMap[syntax]);
            }
        }
        if (!funcDecl->funcBody->retType) {
            return;
        }
        if (auto funcType = dynamic_cast<AST::FuncType*>(funcDecl->funcBody->retType.operator->()); funcType) {
            Diagnose(funcType->begin, funcType->end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_01,
                syntaxNameMap[syntax]);
        }
    };
    /* @brief Check whether a disabled container type is used */
    NodeHandler handleContainerTypeRefExpr = [this](const AST::Node& node, const std::string& syntax) {
        auto refExpr = static_cast<const AST::RefExpr*>(&node);
        auto ref = &refExpr->ref;
        if (ref && disabledConatinerType.count(ref->identifier) > 0) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_03, ref->identifier.Val());
        }
    };
    /* @brief Check whether a disabled container type is used */
    NodeHandler handleContainerTypeRefType = [this](const AST::Node& node, const std::string& syntax) {
        auto refExpr = static_cast<const AST::RefType*>(&node);
        auto ref = &refExpr->ref;
        if (ref && disabledConatinerType.count(ref->identifier) > 0) {
            Diagnose(
                node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_03, ref->identifier.Val());
        }
    };
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURAL_RULE_G_SYN_01_H
