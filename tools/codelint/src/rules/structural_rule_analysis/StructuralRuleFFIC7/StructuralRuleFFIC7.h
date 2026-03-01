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

#ifndef CODIRACODECHECK_STRUCTURALRULEFFIC7_H
#define CODIRACODECHECK_STRUCTURALRULEFFIC7_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
class StructuralRuleFFIC7 : public StructuralRule {
public:
    explicit StructuralRuleFFIC7(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleFFIC7() override = default;
protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;
private:
    void CheckPointerExpr(Ptr<Codira::AST::Node> node);
};
}

#endif
