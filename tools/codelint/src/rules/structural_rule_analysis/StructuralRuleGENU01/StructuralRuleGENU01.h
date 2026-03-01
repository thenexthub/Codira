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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_ENU_01_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_ENU_01_H

#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.ENU.01：避免枚举的构造成员与顶层元素同名
 */
class StructuralRuleGENU01 : public StructuralRule {
public:
    explicit StructuralRuleGENU01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGENU01() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    std::set<std::pair<std::string, PositionPair>> enumSet;
    void DuplicateNameFinderHelper(Ptr<Codira::AST::Node> node);
    void DiagnosticsFunc(const Codira::AST::Decl &decl);
    void DuplicateNameFinder(Ptr<Codira::AST::Node> node);
    void DiagnosticsPackage(const Codira::AST::PackageSpec &packageSpec);
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_ENU_01_H
