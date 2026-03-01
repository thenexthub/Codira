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

#ifndef CODIRACODECHECK_STRUCTURALRULEGITF01_H
#define CODIRACODECHECK_STRUCTURALRULEGITF01_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
class StructuralRuleGITF01 : public StructuralRule {
public:
    explicit StructuralRuleGITF01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGITF01() override = default;

    void DoAnalysis(CODELintCompilerInstance* instance) override;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;
    void MatchPattern(Ptr<Codira::AST::Node> node, TypeManager* typeManager);

private:
    void HasModifiedVar(Ptr<AST::AssignExpr> pAssignExpr, std::unordered_set<Ptr<AST::Decl>>& varDecls,
        const Ptr<AST::Decl> parentFuncDecl);
    void AnalysisFuncDecl(Ptr<AST::FuncDecl> pFuncDecl, std::unordered_set<Ptr<AST::Decl>>& varDecls,
        const Ptr<AST::Decl> parentFuncDecl);
    void CoverDeclToFuncDecl(TypeManager* typeManager, Ptr<AST::Decl> pDecl,
        std::unordered_set<Ptr<AST::Decl>>& varDecls, std::unordered_set<Ptr<AST::Decl>>& funcDecls);
    void AnalysisFuncDeclsOfExtendDecl(TypeManager* typeManager, Ptr<AST::ExtendDecl> pExtendDecl,
        std::unordered_set<Ptr<AST::Decl>>& varDecls, std::unordered_set<Ptr<AST::Decl>>& funcDecls);
    void AnalysisFuncDeclsOfStructDecl(TypeManager* typeManager, Ptr<AST::StructDecl> pStructDecl,
        std::unordered_set<Ptr<AST::Decl>>& varDecls, std::unordered_set<Ptr<AST::Decl>>& funcDecls);
    void CollectVarDeclsOfStructDecl(Ptr<AST::StructDecl> pStructDecl, std::unordered_set<Ptr<AST::Decl>>& varDecls);
    void AnalysisInterfaceDecl(const AST::InterfaceDecl& interfaceDecl, TypeManager* typeManager);
    void FindInterfaceDecl(Ptr<Codira::AST::Node> node, TypeManager* typeManager);
};
} // namespace Codira::CodeCheck

#endif // CODIRACODECHECK_STRUCTURALRULEGITF01_H
