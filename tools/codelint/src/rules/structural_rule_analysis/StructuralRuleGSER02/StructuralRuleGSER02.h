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

#ifndef CODIRACODECHECK_STRUCTURAL_RULE_G_SER_02_H
#define CODIRACODECHECK_STRUCTURAL_RULE_G_SER_02_H

#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRuleGSER.h"

namespace Codira::CodeCheck {
/**
 * G.SER.02 防止反序列化被利用来绕过构造方法中的安全操作
 */

struct VarStruct {
    int directlyAssigned = 0;
    Ptr<const Codira::AST::VarDecl> varDecl;
};

class StructuralRuleGSER02 : public StructuralRuleGSER {
public:
    explicit StructuralRuleGSER02(CodeCheckDiagnosticEngine* diagEngine) : StructuralRuleGSER(diagEngine){};
    ~StructuralRuleGSER02() override = default;

protected:
    void MatchPattern(ASTContext&, Ptr<Codira::AST::Node> node) override;

private:
    std::map<std::string, VarStruct> varSet;
    using PositionPair = std::pair<Position, Position>;
    std::map<PositionPair, std::string> initDeserialVar;
    void JudgeSer(Ptr<Codira::AST::Node> node);
    void RecordLocation(const Codira::AST::RefExpr& ref);
    void DeserializeHandler(const Codira::AST::FuncDecl& funcDecl);
    void ConstructorHandler(const Codira::AST::FuncDecl& funcDecl);
    void CheckInit();
    template <typename T> inline void DeclHandler(const T& decl);
    template <typename T> inline auto SerJudgeHandler(const T& decl);
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURAL_RULE_G_SER_02_H
