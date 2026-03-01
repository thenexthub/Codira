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

#ifndef CODIRACODECHECK_STRUCTURALRULEGEXP07_H
#define CODIRACODECHECK_STRUCTURALRULEGEXP07_H

#include "Codira/AST/Match.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGEXP07 : public StructuralRule {
public:
    explicit StructuralRuleGEXP07(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGEXP07() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    void BinaryDesugar(const Codira::AST::BinaryExpr &binaryExpr);
    void BinaryNotDesugar(const Codira::AST::BinaryExpr &binaryExpr);

    void FindBinaryExpr(Codira::AST::Node *node);
    void CheckBinaryExpr(const Codira::AST::BinaryExpr &binaryExpr);
    void ReferenceExprAnalysis(Ptr<AST::Expr> expr, Ptr<AST::VarDecl> rightVarDecl);
    void VarDeclDiagnose(Ptr<AST::Decl> decl, Position& start, Position& end, Ptr<AST::VarDecl> rightVarDecl);
    bool ExcludeInterval(const Codira::AST::BinaryExpr &binaryExpr);
    bool ExcludeIntervalHelper(const Codira::AST::BinaryExpr &binaryExpr);
    bool ExcludeIntervalCoverToVarDecl(Codira::AST::VarDecl *lv, Codira::AST::VarDecl *rv,
        Ptr<Codira::AST::BinaryExpr> lb, Ptr<Codira::AST::BinaryExpr> rb);
    bool IsTuple(const AST::BinaryExpr &binaryExpr);
    void ExprCoverToVarDecl(Ptr<AST::Expr> expr, Ptr<AST::VarDecl> &varDecl);
};
}
#endif // CODIRACODECHECK_STRUCTURALRULEGEXP07_H
