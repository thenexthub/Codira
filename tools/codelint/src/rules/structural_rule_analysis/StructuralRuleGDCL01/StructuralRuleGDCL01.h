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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_01_H
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.DCL.01 避免遮盖（shadow）
 */
class StructuralRuleGDCL01 : public StructuralRule {
public:
    explicit StructuralRuleGDCL01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGDCL01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    void FindShadowNode(Ptr<Codira::AST::Node>& node);
    template <typename T>
    void TraversingNode(std::vector<OwnedPtr<T>>& node,
        std::map<std::string, std::map<std::string, std::stack<AST::Decl*>>>& container,
        std::map<std::string, std::map<std::string, std::vector<AST::Decl*>>>& tmpContainer, bool isStaticFunc = false);
    void VarDeclProcessor(Codira::AST::VarDecl& varDecl, std::map<std::string, std::stack<AST::Decl*>>& varContainer,
        std::map<std::string, std::vector<AST::Decl*>>& varTmpContainer, bool isStaticFunc = false);
    void FuncDeclProcessor(AST::FuncDecl& funcDecl, std::map<std::string, std::stack<AST::Decl*>>& funcContainer);
    static void PostFuncDeclProcessor(
        Codira::AST::FuncDecl& funcDecl, std::map<std::string, std::stack<AST::Decl*>>& funcContainer);
    static void PostVarDeclProcessor(AST::VarDecl& varDecl, std::map<std::string, std::stack<AST::Decl*>>& varContainer,
        std::set<std::string>& varSet);
    template <typename T>
    static void PostBlockAndClassBodyProcessor(std::vector<OwnedPtr<T>>& nodes,
        std::map<std::string, std::map<std::string, std::stack<AST::Decl*>>>& container,
        std::map<std::string, std::map<std::string, std::vector<AST::Decl*>>>& tmpContainer);
    void FindGenericShadowNode(Ptr<Codira::AST::Node>& node);
    template <typename T>
    void MemberDeclProcessor(
        const std::vector<OwnedPtr<T>>& members, std::map<std::string, AST::Decl*>& genericContainer);
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_DCL_01_H
