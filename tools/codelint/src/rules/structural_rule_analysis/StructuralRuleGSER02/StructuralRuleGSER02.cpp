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

#include "StructuralRuleGSER02.h"
#include "Codira/AST/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

/*
 * The function that Serialization Judgment Processing.
 */
template <typename T> inline auto StructuralRuleGSER02::SerJudgeHandler(const T& decl)
{
    if (decl.body == nullptr) {
        return VisitAction::WALK_CHILDREN;
    }
    if (IsImpSerializable(decl)) {
        DeclHandler(decl);
        StructuralRuleGSER02::varSet.clear();
        StructuralRuleGSER02::initDeserialVar.clear();
        return VisitAction::SKIP_CHILDREN;
    }
    return VisitAction::WALK_CHILDREN;
}

template <typename T> inline void StructuralRuleGSER02::DeclHandler(const T& decl)
{
    if (decl.body == nullptr) {
        return;
    }
    for (auto& bodyDecl : decl.body->decls) {
        match (*bodyDecl)([this](const VarDecl& varDecl) {
            VarStruct varStruct;
            varStruct.directlyAssigned = 0;
            varStruct.varDecl = &varDecl;
            (void)StructuralRuleGSER02::varSet.emplace(varDecl.identifier, varStruct);
        });
    }

    Node* node = (Node*)&decl;
    std::string declName = decl.identifier;

    // walkerToMarkInit: Mark the variable that operated in constructor
    Walker walkerToMarkInit(node, [declName, this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [declName, this](const FuncDecl& funcDecl) {
                if (funcDecl.identifier == declName || funcDecl.identifier == "init" ||
                    funcDecl.identifier == "super") {
                    ConstructorHandler(funcDecl);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walkerToMarkInit.Walk();

    CheckInit();

    // walkerToMarkDeserialize: Mark the variable that directly assigned in deserialization
    Walker walkerToMarkDeserialize(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                if (funcDecl.identifier == "deserialize") {
                    DeserializeHandler(funcDecl);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walkerToMarkDeserialize.Walk();
}

void StructuralRuleGSER02::RecordLocation(const Codira::AST::RefExpr& ref)
{
    Diagnose(ref.begin, ref.end, CodeCheckDiagKind::G_SER_02_Deserialization_Bypass, ref.ToString());
}

void StructuralRuleGSER02::CheckInit()
{
    std::vector<PositionPair> keyList;
    for (auto it = initDeserialVar.begin(); it != initDeserialVar.end(); ++it) {
        keyList.push_back(it->first);
    }

    for (auto& key : keyList) {
        auto varName = initDeserialVar[key];
        if (varSet.count(varName) != 0) {
            Diagnose(key.first, key.second, CodeCheckDiagKind::G_SER_02_Deserialization_Bypass, varName);
        }
    }
}

void StructuralRuleGSER02::ConstructorHandler(const FuncDecl& funcDecl)
{
    auto funcBody = funcDecl.funcBody.get();
    if (!funcBody) {
        return;
    }
    Walker walker1(funcBody, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const AssignExpr& assignExpr) {
                RefExpr* left;
                if (assignExpr.leftValue->astKind == ASTKind::MEMBER_ACCESS) {
                    auto ma = As<ASTKind::MEMBER_ACCESS>(assignExpr.leftValue.get());
                    if (!ma) {
                        return VisitAction::SKIP_CHILDREN;
                    }
                    left = As<ASTKind::REF_EXPR>(ma->baseExpr.get());
                } else {
                    left = As<ASTKind::REF_EXPR>(assignExpr.leftValue.get());
                }
                if (!left) {
                    return VisitAction::SKIP_CHILDREN;
                }

                auto rightCallExpr = As<ASTKind::CALL_EXPR>(&(*assignExpr.rightExpr));
                if (rightCallExpr) {
                    auto right = As<ASTKind::MEMBER_ACCESS>(&(*rightCallExpr->baseFunc));
                    if (right && right->ToString().find("deserialize") != std::string::npos) {
                        if (!left->ref.identifier.Empty()) {
                            auto pos = std::make_pair(assignExpr.leftValue->begin, assignExpr.leftValue->end);
                            initDeserialVar[pos] = left->ref.identifier;
                        }
                        return VisitAction::SKIP_CHILDREN;
                    }
                }

                if (varSet.count(left->ref.identifier)) {
                    varSet[left->ref.identifier].directlyAssigned = 1;
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker1.Walk();
}

void StructuralRuleGSER02::DeserializeHandler(const FuncDecl& funcDecl)
{
    auto funcBody = funcDecl.funcBody.get();
    if (!funcBody) {
        return;
    }
    Walker walker(funcBody, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const AssignExpr& assignExpr) {
                auto rightCallExpr = As<ASTKind::CALL_EXPR>(&(*assignExpr.rightExpr));
                if (rightCallExpr) {
                    auto right = As<ASTKind::MEMBER_ACCESS>(&(*rightCallExpr->baseFunc));
                    if (!right || right->ToString().find("deserialize") == std::string::npos) {
                        return VisitAction::SKIP_CHILDREN;
                    }
                } else {
                    return VisitAction::SKIP_CHILDREN;
                }
                auto left = As<ASTKind::REF_EXPR>(assignExpr.leftValue.get());
                if (left == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (varSet.count(left->ToString()) && varSet[left->ToString()].directlyAssigned == 1) {
                    RecordLocation(*left);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGSER02::JudgeSer(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker1(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)([this](const ClassDecl& classDecl) { return SerJudgeHandler(classDecl); },
            [this](const StructDecl& structDecl) { return SerJudgeHandler(structDecl); },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker1.Walk();
}

void StructuralRuleGSER02::MatchPattern(ASTContext&, Ptr<Node> node)
{
    FindExtendSer(node);
    JudgeSer(node);
}
} // namespace Codira::CodeCheck
