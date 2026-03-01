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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_02
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_02

#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/RegexRule.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.OTH.02 禁止将敏感信息硬编码在程序中
 */
class StructuralRuleGOTH02 : public RegexRule {
public:
    explicit StructuralRuleGOTH02(CodeCheckDiagnosticEngine* diagEngine) : RegexRule(diagEngine){};
    ~StructuralRuleGOTH02() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    void FindSenInfoInAST(Ptr<Codira::AST::Node> node);
    bool IsWeakPasscode(const std::string& text);
    bool IsSenInfoName(const std::string& text);
    void MatchPatternInAST(const Codira::AST::VarDecl& varDecl);
    void MatchPatternInAST(const Codira::AST::VarWithPatternDecl& varWithPatternDecl);
    void DealWithMatchResultfromRegex(Ptr<Codira::AST::Node> node);
    bool SenInfoFilter(const std::string& key, const std::string& text);
};
} // namespace Codira::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_SYNTAX_RULE_G_OTH_02
