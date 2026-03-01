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

#ifndef CODIRACODECHECK_STRUCTURAL_RULE_G_ENU_02_H
#define CODIRACODECHECK_STRUCTURAL_RULE_G_ENU_02_H

#include <fstream>
#include <iostream>
#include "rules/structural_rule_analysis/StructuralRule.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
class StructuralRuleGENU02 : public StructuralRule {
public:
    explicit StructuralRuleGENU02(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGENU02() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    struct EnumCtr {
        std::string identifier;
        std::vector<AST::Ty*> args;
        bool isCtr;
        EnumCtr(std::string identifier,
            std::vector<AST::Ty*> args, bool isCtr) : identifier(identifier), args(args), isCtr(isCtr){};
    };
    /** @brief Records information about all enum constructors. */
    std::vector<EnumCtr> enumCtrSet;
    /** @brief Map describing class inheritance introduced by extendDecl. */
    std::map<AST::Ty*, std::vector<AST::Type*>> inheritedClassMap;
    /** @brief Check whether there is an inheritance relationship between two Ty. */
    bool CheckTyEqualityHelper(Codira::AST::Ty* base, Codira::AST::Ty* derived);
    /** @brief If there is an inheritance relationship, the two types are considered to be the same. */
    bool IsEqual(Codira::AST::Ty* base, Codira::AST::Ty* derived);
    /** @brief Check and collect constructors of enum. */
    void FindEnumDeclHelper(Ptr<Codira::AST::Node> node);
    /** @brief Record the inheritance relationship implemented through Extend. */
    void FindExtendHelper(Ptr<Codira::AST::Node> node);
    /** @brief Check whether a function or constructor is duplicated. */
    void DuplicatedEnumCtrOrFuncHelper(const Codira::AST::FuncDecl& funcDecl);
    /** @brief Check whether enum constructor is overloaded. */
    void CheckEnumCtrOverload(const Codira::AST::EnumDecl& enumDecl);
    /** @brief Check whether a function is overloaded. */
    void CheckFuncOverload(const Codira::AST::FuncDecl& funcDecl);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURAL_RULE_G_ENU_02_H
