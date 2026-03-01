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

#ifndef CODIRACODECHECK_STRUCTURALRGOTH04_H
#define CODIRACODECHECK_STRUCTURALRGOTH04_H

#include "nlohmann/json.hpp"
#include "Codira/AST/Match.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.OTH.04 不要使用String 存储敏感数据，敏感数据使用结束后应立即清 0
 */
class StructuralRuleGOTH04 : public StructuralRule {
public:
    explicit StructuralRuleGOTH04(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGOTH04() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;
private:
    nlohmann::json jsonInfo;
    void FindVarDecl(Ptr<Codira::AST::Node> node);
    void AnalyzeVarDecl(const Codira::AST::VarDecl &varDecl);
    void VarDeclTypeAnalysis(Ptr<Codira::AST::Ty> ty, bool &flag);
    void TypeStructAnalysis(Ptr<Codira::AST::Ty> ty, bool &flag);
    void TypeEnumAnalysis(Ptr<Codira::AST::Ty> ty, bool &flag);
    void TypeTupleAnalysis(Ptr<Codira::AST::Ty> ty, bool &flag);
    bool SenInfoFilter(const std::string& key, const std::string& text);
    bool IsSensitiveDataVar(const std::string &text);

    void TypeClassAnalysis(Ptr<Codira::AST::Ty> ty, bool& flag);
    void IsIncludeStringType(Ptr<Codira::AST::Ty> ty, bool& flag);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURALRGOTH04_H
