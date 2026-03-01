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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_04_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_04_H

#include "nlohmann/json.hpp"
#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "common/TaintData.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.CHK.04 禁止直接使用不可信数据构造正则表达式
 */
class StructuralRuleGCHK04 : public StructuralRule {
public:
    explicit StructuralRuleGCHK04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGCHK04() override = default;

protected:
    void MatchPattern(ASTContext&, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    void RegexFinder(Ptr<Codira::AST::Node> node);
    void GetLitConstExpr(const Codira::AST::CallExpr &callExpr);
    void RegexChecker(const OwnedPtr<AST::FuncArg> &arg);
    void GetLitConstExprHelperRefExpr(Ptr<Codira::AST::Expr> expr, const Position start, const Position end,
        std::string refName, std::string &str);
    void MemberAccessTargetCheck(Ptr<Codira::AST::MemberAccess> memberAccess, const Position start, const Position end,
        const std::string refName, std::string &tmpStr);
    std::string GetLitConstExprHelper(Ptr<Codira::AST::Expr> expr, const Position start, const Position end,
        const std::string &refName = "");
    std::map<std::string, Codira::Position> regexSet;
};
} // namespace Codira::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_04_H
