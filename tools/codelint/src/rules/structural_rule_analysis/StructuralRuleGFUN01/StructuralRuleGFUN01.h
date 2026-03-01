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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_01_H

#include "rules/structural_rule_analysis/StructuralRule.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
/**
 * 仓颉编程语言通用编程规范的0.1版本
 * G.FUN.01 函数功能要单一
 */
class StructuralRuleGFUN01 : public StructuralRule {
public:
    explicit StructuralRuleGFUN01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFUN01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    void FuncAndCodeControlBlockFinder(Ptr<Codira::AST::Node> node);
    void CheckFuncHelper(const AST::FuncDecl &funcDecl);
    void CheckLambdaExprHelper(const AST::LambdaExpr &lambdaExpr);
    void CheckControlExpr(const  AST::Expr &expr, size_t &depth);
    void CheckControlExprHelper(std::vector<OwnedPtr<AST::Node>>& body, size_t& depth);
    void CheckControlBlock(const AST::Expr& expr, AST::ASTKind astKind);
    void CheckIfExprHelper(const AST::Expr& expr, size_t& depth);
    void CheckWhileExpr(const AST::Expr& expr, size_t& depth);
    void CheckDoWhileExpr(const AST::Expr& expr, size_t& depth);
    void CheckMatchExpr(const AST::Expr& expr, size_t& depth);
    void CheckForInExpr(const AST::Expr& expr, size_t& depth);
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_01_H
