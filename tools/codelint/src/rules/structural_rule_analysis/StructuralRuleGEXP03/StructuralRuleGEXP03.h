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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_03_H

#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.EXP.04：&& 、 ||、? 和 ?? 操作符的右侧操作数不要包含副作用
 */
class StructuralRuleGEXP03 : public StructuralRule {
public:
    explicit StructuralRuleGEXP03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGEXP03() override = default;
protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    Ptr<Codira::AST::Node> originNode = nullptr;
    std::set<Codira::Position> sideEffectSet;
    std::vector<Codira::Position> callStack;
    std::set<Codira::Position> checkedFunc;
    void RightOperandFinder(Ptr<Codira::AST::Node> node);
    std::string GetOperandName(const Codira::AST::AssignExpr &assignExpr);
    void SideEffectChecker(Ptr<Codira::AST::Node> node);
    void SideEffectFuncFinder(Ptr<Codira::AST::Node> node);
    void SideEffectExprFinder(Ptr<Codira::AST::Node> node, const Codira::Position position);
    void GlobalRefChecker(Ptr<Codira::AST::Node> node, const Codira::Position originFuncPosition,
        const Codira::Position refFuncPosition);
    void ClassRefChecker(Ptr<Codira::AST::Node> node, const Codira::Position originFuncPosition,
        const Codira::Position refFuncPosition);
    bool AddRefFuncInfo(Ptr<const Codira::AST::Node> node, const Codira::Position originFuncPosition,
        const Codira::Position refFuncPosition);
    void ProcessRefFuncInfo(const AST::RefExpr &refExpr, const Codira::Position originFuncPosition);
    void FindMemberAccess(const AST::MemberAccess &memberAccess);
    void ClassRefCheckerDetail(const AST::RefExpr &refExpr, const Codira::Position originFuncPosition);
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_EXP_03_H
