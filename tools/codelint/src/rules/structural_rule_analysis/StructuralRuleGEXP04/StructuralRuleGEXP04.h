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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_04_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_04_H

#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * 仓颉编程语言通用编程规范
 * G.EXP.04：尽量避免副作用发生依赖于操作符的求值顺序
 */
class StructuralRuleGEXP04 : public StructuralRule {
public:
    explicit StructuralRuleGEXP04(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGEXP04() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    void CheckSideEffect(Ptr<Codira::AST::Node> node);
    bool HasSideEffectInFunc(Ptr<Codira::AST::FuncDecl> funcDecl);
    bool CheckSideEffectInExpr(Ptr<AST::Expr> expr);
    std::unordered_map<Ptr<AST::FuncDecl>, bool> funcHasCheckedMap;
    Ptr<AST::FuncDecl> funcWithSideEffect;
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_04_H
