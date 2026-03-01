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

#ifndef CODIRACODECHECK_STRUCTURALRULEGNAM03_H
#define CODIRACODECHECK_STRUCTURALRULEGNAM03_H

#include <fstream>
#include <iostream>
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.NAM.03 接口，类，Struct、枚举类型和枚举成员构造，类型别名，采用大驼峰命名
 */
class StructuralRuleGNAM03 : public StructuralRule {
public:
    explicit StructuralRuleGNAM03(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGNAM03() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    void FindAssignTypes(Ptr<Codira::AST::Node> node);
    void CheckExceptionRule(const Codira::AST::ClassDecl& classDecl);
    template <typename T> auto CheckNameRule(T& decl);
    bool IsExceptionSubclass(const Codira::AST::ClassDecl& decl, std::set<Ptr<AST::ClassDecl>> declSet = {});
    void CheckQuoteExprToken(std::vector<Token>& tokens);
    void CheckQuoteExpr(const Codira::AST::QuoteExpr& quoteExpr);
    void PrintDiagnoseInfo(bool condition, Position start, Position end, const std::string& value);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURALRULEGNAM03_H
