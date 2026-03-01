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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_03
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_OTH_03

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/RegexRule.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.OTH.03  禁止代码中包含公网地址
 */


class StructuralRuleGOTH03 : public RegexRule {
public:
    explicit StructuralRuleGOTH03(CodeCheckDiagnosticEngine *diagEngine) : RegexRule(diagEngine) {};
    ~StructuralRuleGOTH03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    bool CheckIpAddress(const std::string ip);
    bool isIpHardcode(std::vector<int> ips);
    bool IsSpecialIpHardcode(std::vector<int> ips) const;
    void RecordErrorLocation(const std::vector<ResultInfo>::iterator iter, CodeCheckDiagKind kind);
};
} // namespace Codira::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_IP_HARDCODE
