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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_02_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_02_H
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * 仓颉编程语言通用编程规范的0.1版本
 * G.FUN.02 禁止函数有未被使用的参数
 */
class StructuralRuleGFUN02 : public StructuralRule {
public:
    explicit StructuralRuleGFUN02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFUN02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    void FuncParamFinder(Ptr<Codira::AST::Node> node);
    void FuncParamFinderHelper(Ptr<Codira::AST::Node> node, std::map<std::string, PositionPair> &paramMap);
    static void EraseItemOfMap(const std::string& id, std::map<std::string, PositionPair>& paramMap);
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUN_02_H
