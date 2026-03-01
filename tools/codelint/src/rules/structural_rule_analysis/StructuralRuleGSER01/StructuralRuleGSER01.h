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

#ifndef CODIRACODECHECK_STRUCTURAL_RULE_G_SER_01_H
#define CODIRACODECHECK_STRUCTURAL_RULE_G_SER_01_H

#include "Codira/Basic/Match.h"
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRuleGSER.h"

namespace Codira::CodeCheck {
class StructuralRuleGSER01 : public StructuralRuleGSER {
public:
    explicit StructuralRuleGSER01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRuleGSER(diagEngine){};
    ~StructuralRuleGSER01() override = default;

protected:
    void MatchPattern(ASTContext&, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json sensitiveKeys;
    const std::string* GetStringExprVal(Ptr<Codira::AST::Expr> expr);
    void CheckSensitiveInfo(Ptr<Codira::AST::Expr> expr, CodeCheckDiagKind kind);
    void CheckSensitiveInfo(
        const std::string& str, CodeCheckDiagKind kind, const Codira::Position& start, const Codira::Position& end);
    void SerializeHandler(const Codira::AST::FuncDecl& funcDecl);
    template <typename T> void JudgeSerialize(const T& decl);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURAL_RULE_G_SER_01_H
