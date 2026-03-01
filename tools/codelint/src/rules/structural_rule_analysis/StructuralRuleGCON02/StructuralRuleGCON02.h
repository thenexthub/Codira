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

#ifndef CODIRACODECHECK_STRUCTURALRULEGCON02_H
#define CODIRACODECHECK_STRUCTURALRULEGCON02_H

#include "rules/structural_rule_analysis/StructuralRule.h"

#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using Gcon02Lock = std::map<std::string, std::vector<int>>;

const int TUPLE_PARAM_1ST = 0;
const int TUPLE_PARAM_2ND = 1;
const int TUPLE_PARAM_3RD = 2;
const int TUPLE_PARAM_4TH = 3;

class StructuralRuleGCON02 : public StructuralRule {
public:
    explicit StructuralRuleGCON02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCON02() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    std::unordered_map<std::string, std::string> varMap;
    std::unordered_map<std::string, std::tuple<Codira::Position, Codira::Position, bool, std::string>> tryLockMap;
    std::set<std::string> tryBlockKey;

    void CheckResult(Ptr<Codira::AST::Node> node);
    void CheckTryExpr(const Codira::AST::TryExpr &tryExpr);
    void PrintDiagInfo();
    void TryLockCheck(const Codira::AST::MemberAccess &memberAccess);
    void FinallyUnLockCheck(const Codira::AST::MemberAccess &memberAccess);
    std::string GetMemberAccessField(const Codira::AST::MemberAccess &memberAccess);
    void GetLockInfo(const Codira::AST::VarDecl &varDecl);
    std::string GetLockKey(const Codira::AST::VarDecl &varDecl);
    std::string GetMemberAccessKey(const Codira::AST::MemberAccess &memberAccess);
};
}
#endif
