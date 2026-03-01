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

#include "StructuralRuleGSEC01.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

const std::string JSONPATH = "/config/structural_rule_G_SEC_01.json";

void StructuralRuleGSEC01::RecordLocation(Ptr<FuncDecl> funcDecl)
{
    if (!jsonInfo.contains("CheckingFunction")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    for (const auto& item : jsonInfo["CheckingFunction"]) {
        std::regex reg = std::regex(item.get<std::string>(), std::regex_constants::icase);
        if (std::regex_match(funcDecl->identifier.Val(), reg)) {
            Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                CodeCheckDiagKind::G_SEC_01_open_check_function_information, funcDecl->identifier.Val());
        }
    }
}

bool StructuralRuleGSEC01::IsExtendClass(const ClassDecl& classDecl) const
{
    for (auto i : classDecl.modifiers) {
        if (i.modifier == TokenKind::OPEN) {
            return true;
        }
    }
    if (!classDecl.body) {
        return false;
    }
    for (auto& classBody : classDecl.body->decls) {
        for (auto i : classBody->modifiers) {
            if (i.modifier == TokenKind::OPEN) {
                return true;
            }
        }
    }
    return false;
}

void StructuralRuleGSEC01::ClassDeclHandlerDetail(const OwnedPtr<Decl>& classBody, const bool isExtend)
{
    if (classBody.get() != nullptr && classBody.get()->astKind == ASTKind::FUNC_DECL) {
        auto funcDecl = StaticAs<ASTKind::FUNC_DECL>(classBody.get());
        for (auto item : funcDecl->modifiers) {
            if ((item.modifier == TokenKind::OPEN) || (item.modifier == TokenKind::STATIC && isExtend)) {
                RecordLocation(funcDecl);
            }
        }
    }
}

void StructuralRuleGSEC01::ClassDeclHandler(const ClassDecl& classDecl)
{
    bool isExtend = IsExtendClass(classDecl);
    if (!classDecl.body) {
        return;
    }
    for (auto& classBody : classDecl.body->decls) {
        ClassDeclHandlerDetail(classBody, isExtend);
    }
}

void StructuralRuleGSEC01::InterfaceDeclHandler(const InterfaceDecl& interfaceDecl)
{
    if (interfaceDecl.body) {
        for (auto& interfaceBoyd : interfaceDecl.body->decls) {
            if (interfaceBoyd.get() != nullptr && interfaceBoyd.get()->astKind == ASTKind::FUNC_DECL) {
                auto funcDecl = StaticAs<ASTKind::FUNC_DECL>(interfaceBoyd.get());
                RecordLocation(funcDecl);
            }
        }
    }
}

void StructuralRuleGSEC01::FindCheckingFunction(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            // Identifying Variable Declaration Statements in Programs
            [this](const ClassDecl& classDecl) {
                ClassDeclHandler(classDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const InterfaceDecl& interfaceDecl) {
                InterfaceDeclHandler(interfaceDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            // The rest of the statements are here.
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGSEC01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;

    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    FindCheckingFunction(node);
}
} // namespace Codira::CodeCheck
