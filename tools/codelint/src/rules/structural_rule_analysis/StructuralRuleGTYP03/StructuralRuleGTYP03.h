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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_TYP_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_TYP_03_H

#include "Codira/AST/Match.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.TYP.03 判断变量是否为 NaN 时必须使用 isNaN() 方法
 */
class StructuralRuleGTYP03 : public StructuralRule {
public:
    explicit StructuralRuleGTYP03(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGTYP03() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    void FloatBinaryFinder(Ptr<Codira::AST::Node> node);
    void CheckBinaryExpr(const Codira::AST::BinaryExpr& binaryExpr);
    void AnalyzeBinaryExpr(const Ptr<Codira::AST::Expr> expr1, const Ptr<Codira::AST::Expr>& expr2);
};
} // namespace Codira::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_TYP_03_H
