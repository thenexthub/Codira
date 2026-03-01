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

#ifndef CODIRACODECHECK_STRUCTURALRULEGERR02_H
#define CODIRACODECHECK_STRUCTURALRULEGERR02_H

#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
class StructuralRuleGERR02 : public StructuralRule {
public:
    explicit StructuralRuleGERR02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGERR02() override  = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    using PositionPair = std::pair<Codira::Position, Codira::Position>;
    std::vector<std::pair<PositionPair, std::string>> exceptionTypeDiag;
    std::vector<std::pair<PositionPair, std::string>> exceptionInfoDiag;

    void CheckResult(Ptr<Codira::AST::Node> node);
    std::string GetExceptionInfo(Ptr<Codira::AST::CallExpr> callExpr);
    std::string GetBinaryStr(Ptr<Codira::AST::BinaryExpr> binaryExpr);
    std::string GetDeclStr(Ptr<Codira::AST::Decl> decl);
    std::vector<std::string> GetSensitiveKeyWord(const std::string &input);
    bool IsSensitiveException(const std::string &input) const;
    void PrintDebugInfo() const;
    std::vector<std::pair<std::string, int>> debugInfo;
};
}

#endif
