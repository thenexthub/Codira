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

#ifndef CODIRACODECHECK_STRUCTURALRULEGITF04_H
#define CODIRACODECHECK_STRUCTURALRULEGITF04_H

#include "rules/structural_rule_analysis/StructuralRule.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
class StructuralRuleGITF04 : public StructuralRule {
public:
    explicit StructuralRuleGITF04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGITF04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;
    void FindFuncDecl(Codira::AST::Node *node);
    void CheckFuncDeclParams(const Codira::AST::FuncDecl &funcDecl);
};
}
#endif // CODIRACODECHECK_STRUCTURALRULEGITF04_H
