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

#ifndef CODIRACODECHECK_STRUCTURALRULEGSEC01_H
#define CODIRACODECHECK_STRUCTURALRULEGSEC01_H

#include <fstream>
#include <iostream>
#include <regex>
#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.SEC.01 进行安全检查的方法禁止声明为open
 */

class StructuralRuleGSEC01 : public StructuralRule {
public:
    explicit StructuralRuleGSEC01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGSEC01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    void FindCheckingFunction(Ptr<Codira::AST::Node> node);
    void ClassDeclHandler(const Codira::AST::ClassDecl& classDecl);
    void RecordLocation(Ptr<AST::FuncDecl> funcDecl);
    void InterfaceDeclHandler(const Codira::AST::InterfaceDecl& interfaceDecl);
    bool IsExtendClass(const Codira::AST::ClassDecl& classDecl) const;
    void ClassDeclHandlerDetail(const OwnedPtr<Codira::AST::Decl>& classBody, const bool isExtend);
};
} // namespace Codira::CodeCheck

#endif // CODIRACODECHECK_STRUCTURALRULEGSEC01_H
