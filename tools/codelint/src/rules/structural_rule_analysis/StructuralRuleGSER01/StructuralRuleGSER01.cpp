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

#include "StructuralRuleGSER01.h"
#include "Codira/AST/Match.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

const std::string* StructuralRuleGSER01::GetStringExprVal(Ptr<Expr> expr)
{
    if (auto constExpr = DynamicCast<LitConstExpr*>(expr.get()); constExpr) {
        return &constExpr->stringValue;
    } else if (auto refExpr = DynamicCast<RefExpr*>(expr.get()); refExpr) {
        if (auto varDecl = DynamicCast<VarDecl*>(refExpr->ref.target.get()); varDecl) {
            return GetStringExprVal(varDecl->initializer.get());
        }
    }

    return nullptr;
}

void StructuralRuleGSER01::CheckSensitiveInfo(Ptr<Expr> expr, CodeCheckDiagKind kind)
{
    auto str = GetStringExprVal(expr);
    if (!str) {
        return;
    }

    CheckSensitiveInfo(*str, kind, expr->begin, expr->end);
}

void StructuralRuleGSER01::CheckSensitiveInfo(
    const std::string& str, CodeCheckDiagKind kind, const Position& start, const Position& end)
{
    for (const auto& item : sensitiveKeys) {
        if (str.find(item.get<std::string>()) == std::string::npos) {
            continue;
        }
        Diagnose(start, end, kind, str.c_str());
    }
}

void StructuralRuleGSER01::SerializeHandler(const FuncDecl& funcDecl)
{
    auto funcBody = funcDecl.funcBody.get();
    if (!funcBody) {
        return;
    }
    Walker walker(funcBody, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const CallExpr& callExpr) {
                auto baseFunc = As<ASTKind::MEMBER_ACCESS>(callExpr.baseFunc.get());
                if (!baseFunc) {
                    return VisitAction::SKIP_CHILDREN;
                }

                if (baseFunc->field == "serialize") {
                    CheckSensitiveInfo(baseFunc->baseExpr.get(), CodeCheckDiagKind::G_SER_01_serialize_sensitive_data);
                    return VisitAction::SKIP_CHILDREN;
                }

                if (baseFunc->field != "add" || baseFunc->baseExpr->ty->name != "DataModelStruct") {
                    return VisitAction::SKIP_CHILDREN;
                }

                auto arg = As<ASTKind::CALL_EXPR>(callExpr.args[0]->expr.get());
                if (!arg) {
                    return VisitAction::SKIP_CHILDREN;
                }

                auto field = As<ASTKind::REF_EXPR>(arg->baseFunc.get());
                if (!field || field->ref.identifier != "field") {
                    return VisitAction::SKIP_CHILDREN;
                }

                CheckSensitiveInfo(arg->args[0]->expr.get(), CodeCheckDiagKind::G_SER_01_serialize_sensitive_field);
                CheckSensitiveInfo(arg->args[1]->ToString(), CodeCheckDiagKind::G_SER_01_serialize_sensitive_field,
                    arg->args[1]->begin, arg->args[1]->end);

                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

template <typename T> void StructuralRuleGSER01::JudgeSerialize(const T& decl)
{
    Node* node = (Node*)&decl;
    Walker walkerToMarkSerialize(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                if (funcDecl.identifier == "serialize") {
                    SerializeHandler(funcDecl);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walkerToMarkSerialize.Walk();
}

void StructuralRuleGSER01::MatchPattern(ASTContext&, Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    FindExtendSer(node);

    nlohmann::json sensitiveInfo;
    if (CommonFunc::ReadJsonFileToJsonInfo("/config/structural_rule_G_SER_01.json",
        ConfigContext::GetInstance(), sensitiveInfo) == ERR) {
        return;
    }
    if (!sensitiveInfo.contains("SensitiveKeyword")) {
        Errorln("/config/structural_rule_G_SER_01.json", " read json data failed!");
        return;
    } else {
        sensitiveKeys = sensitiveInfo["SensitiveKeyword"];
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ClassDecl& decl) {
                if (IsImpSerializable(decl)) {
                    JudgeSerialize(decl);
                    return VisitAction::SKIP_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this](const StructDecl& decl) {
                if (IsImpSerializable(decl)) {
                    JudgeSerialize(decl);
                    return VisitAction::SKIP_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this](const ExtendDecl& decl) {
                if (decl.extendedType && extendSers.find(decl.extendedType->ToString()) != extendSers.end()) {
                    JudgeSerialize(decl);
                    return VisitAction::SKIP_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this](const CallExpr& callExpr) {
                auto baseFunc = As<ASTKind::MEMBER_ACCESS>(callExpr.baseFunc.get());
                if (baseFunc && baseFunc->field == "serialize") {
                    CheckSensitiveInfo(baseFunc->baseExpr.get(), CodeCheckDiagKind::G_SER_01_serialize_sensitive_data);
                    return VisitAction::SKIP_CHILDREN;
                }

                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}
} // namespace Codira::CodeCheck
