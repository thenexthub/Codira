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

#ifndef CODIRACODECHECK_STRUCTURALRULEGNAM06_H
#define CODIRACODECHECK_STRUCTURALRULEGNAM06_H

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
/**
 * G.NAM.06 变量的名称采用小驼峰
 */
class StructuralRuleGNAM06 : public StructuralRule {
public:
    explicit StructuralRuleGNAM06(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM06() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    void CheckCamelCaseFormat(Ptr<Codira::AST::Node> node);
    void PrintDiagnoseInfo(SrcIdentifier& identifier, std::string pattern, CodeCheckDiagKind kind);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURALRULEGNAM06_H
