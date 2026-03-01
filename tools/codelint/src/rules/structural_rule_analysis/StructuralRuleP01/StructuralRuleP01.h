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

#ifndef CODIRACODECHECK_STRUCTURALRULEP01_H
#define CODIRACODECHECK_STRUCTURALRULEP01_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
class StructuralRuleP01 : public StructuralRule {
public:
    explicit StructuralRuleP01(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleP01() override = default;
    void DoAnalysis(CODELintCompilerInstance *instance) override;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    using P01DiagFmt = std::tuple<PositionPair, CodeCheckDiagKind, int, std::string>;
    using P01BlockSeq = std::vector<std::pair<std::string, int>>;
    using P01FuncBlocks = std::vector<std::pair<std::pair<PositionPair, std::string>, P01BlockSeq>>;
    std::unordered_map<std::string, P01FuncBlocks> funcLockSeq;

    void GetLockSeq(Ptr<Codira::AST::Node> node);
    void FuncHandler(Codira::AST::FuncBody &funcBody);
    void LambdaExprHandler(const Codira::AST::LambdaExpr &lambdaExpr);
    void SynchronizedExprHandler(Codira::AST::SynchronizedExpr &expr, const std::string funcName);
    void CompareTwoFuncBlocks(P01FuncBlocks &blockI, P01FuncBlocks &blockJ, std::vector<P01DiagFmt> &diagInfos);
    void CheckResult();
    void PrintDebugInfo() const;
};
}

#endif
