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

#ifndef CODIRACODECHECK_STRUCTURAL_RULE_G_SYN_0101_H
#define CODIRACODECHECK_STRUCTURAL_RULE_G_SYN_0101_H

#include <fstream>
#include <iostream>
#include "Codira/Basic/Match.h"
#include "nlohmann/json.hpp"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGSYN0101 : public StructuralRule {
public:
    explicit StructuralRuleGSYN0101(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGSYN0101() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;

private:
    nlohmann::json jsonInfo;
    std::set<AST::TypeKind> allowedType = {AST::TypeKind::TYPE_BOOLEAN, AST::TypeKind::TYPE_INT64,
        AST::TypeKind::TYPE_FLOAT64, AST::TypeKind::TYPE_UNIT, AST::TypeKind::TYPE_NOTHING};
    using NodeHandler = std::function<void(const AST::Node&)>;
    std::unordered_map<std::string, NodeHandler> disabledSyntaxCheckMap;
    std::unordered_set<std::string> syntaxList = {};
    void CollectDisabledSyntax();
    void FindSyntax(Ptr<Codira::AST::Node> node);
    void InitDisabledSyntaxCheckMap();
    std::unordered_map<AST::TypeKind, std::string> typeMapToStringMap = {{AST::TypeKind::TYPE_UNIT, "Unit"},
        {AST::TypeKind::TYPE_INT8, "Int8"}, {AST::TypeKind::TYPE_INT16, "Int16"}, {AST::TypeKind::TYPE_INT32, "Int32"},
        {AST::TypeKind::TYPE_INT64, "Int64"}, {AST::TypeKind::TYPE_INT_NATIVE, "IntNative"},
        {AST::TypeKind::TYPE_IDEAL_INT, "Int"}, {AST::TypeKind::TYPE_UINT8, "UInt8"},
        {AST::TypeKind::TYPE_UINT16, "UInt16"}, {AST::TypeKind::TYPE_UINT32, "UInt32"},
        {AST::TypeKind::TYPE_UINT64, "UInt64"}, {AST::TypeKind::TYPE_UINT_NATIVE, "UIntNative"},
        {AST::TypeKind::TYPE_FLOAT16, "Float16"}, {AST::TypeKind::TYPE_FLOAT32, "Float32"},
        {AST::TypeKind::TYPE_FLOAT64, "Float64"}, {AST::TypeKind::TYPE_IDEAL_FLOAT, "Float"},
        {AST::TypeKind::TYPE_RUNE, "Rune"}, {AST::TypeKind::TYPE_NOTHING, "Nothing"},
        {AST::TypeKind::TYPE_BOOLEAN, "Bool"}};
    NodeHandler handlePrimitiveType = [this](const AST::Node& node) {
        auto nodeTy = node.ty;
        if (!nodeTy) {
            return;
        }
        if (nodeTy->IsPrimitive() && allowedType.count(nodeTy->kind) == 0) {
            Diagnose(node.begin, node.end, CodeCheckDiagKind::G_SYN_01_disable_syntax_information_02,
                typeMapToStringMap[nodeTy->kind]);
        }
    };
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_STRUCTURAL_RULE_G_SYN_0101_H
