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

#ifndef CODIRACODECHECK_STRUCTURALRULEGCLS01_H
#define CODIRACODECHECK_STRUCTURALRULEGCLS01_H

#include "Codira/AST/Match.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGCLS01 : public StructuralRule {
public:
    explicit StructuralRuleGCLS01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGCLS01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;
    void FindClassDecl(Ptr<Codira::AST::Node> node);
    void CheckClassDecl(const Codira::AST::ClassDecl &classDecl);
    void CheckSubClassFuncAccess(const Codira::AST::ClassDecl &classDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, std::set<Ptr<Codira::AST::Decl>> &visitedClassDecl);
    void CheckSubClassFuncAccessHelper(Ptr<Codira::AST::ClassDecl> classDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, std::set<Ptr<Codira::AST::Decl>> &visitedClassDecl);
    static void CheckParentClassFuncAccess(const Codira::AST::ClassDecl &classDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility);
    static bool CheckFuncCall(Ptr<Codira::AST::FuncDecl> funcDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, const std::string &subClassName, const std::string &access);
    static bool CheckFuncCallHelper(Ptr<Codira::AST::FuncDecl> funcDecl,
        std::map<std::pair<std::string, std::vector<std::string>>, std::string> &funcAccessibility,
        const std::string &parentClassName, const std::string &subClassName, const std::string &access);
    static std::string CheckModifier(const std::set<Codira::AST::Modifier> &modifiers);
    static std::pair<std::string, std::vector<std::string>> GetFuncDecl(Ptr<Codira::AST::FuncDecl> funcDecl);
};
}
#endif // CODIRACODECHECK_STRUCTURALRULEGCLS01_H
