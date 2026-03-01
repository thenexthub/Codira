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

#ifndef CODIRACODECHECK_STRUCTURALRULEGNAM04_H
#define CODIRACODECHECK_STRUCTURALRULEGNAM04_H

#include <fstream>
#include <iostream>
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.NAM.04 函数名称应采用小驼峰命名
 */
class StructuralRuleGNAM04 : public StructuralRule {
public:
    explicit StructuralRuleGNAM04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    void FindFuncName(Ptr<Codira::AST::Node> node);
    void CheckFuncName(const Codira::AST::FuncDecl &funcDecl);
};
}
#endif // CODIRACODECHECK_STRUCTURALRULEGNAM04_H
