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

#include "StructuralRuleGFUNC03.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

static bool IsSubClass(Ptr<AST::Ty> base, Ptr<AST::Ty> derived)
{
    if (!derived->IsClassLike()) {
        return false;
    }
    if (base == derived) {
        return true;
    }
    auto classDecl = StaticCast<AST::ClassLikeDecl*>(AST::Ty::GetDeclOfTy(derived).get());
    for (auto& super : classDecl->inheritedTypes) {
        if (IsSubClass(base, super->ty)) {
            return true;
        }
    }
    return false;
}

static bool IsNotSameParams(std::vector<Ptr<AST::Ty>>& current, std::vector<Ptr<AST::Ty>>& target)
{
    if (current.size() != target.size()) {
        return true;
    }
    for (size_t i = 0; i < current.size(); i++) {
        if (current[i]->String() != target[i]->String()) {
            return true;
        }
    }
    return false;
}

static bool HasParentChildTypeRelation(std::vector<Ptr<AST::Ty>>& current, std::vector<Ptr<AST::Ty>>& target)
{
    if (current.size() != target.size()) {
        return false;
    }
    for (size_t i = 0; i < current.size(); i++) {
        if (!IsSubClass(current[i], target[i]) && !IsSubClass(target[i], current[i])) {
            return false;
        }
    }
    return true;
}

void StructuralRuleGFUNC03::FuncDeclProcessor(Ptr<Node> node)
{
    auto funcDecl = StaticCast<AST::FuncDecl*>(node);
    auto functy = DynamicCast<AST::FuncTy*>(funcDecl->ty);
    if (functy) {
        auto paramTys = functy->paramTys;
        for (auto modifier : funcDecl->modifiers) {
            if (modifier.modifier== TokenKind::COMMON || modifier.modifier == TokenKind::PLATFORM) {
                return;
            }
        }
        auto funcInfo = FuncInfo(funcDecl->identifier.GetRawText(), paramTys, blocks.top());
        localFuncInBlock[blocks.top()].insert(funcInfo);
        auto status = CheckFuncDecl(allFuncs, funcInfo);
        if (status == Status::IN_DIFF_SCOPE) {
            Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                CodeCheckDiagKind::G_FUN_03_overload_in_different_scopes, funcDecl->identifier.Val());
        }
        if (status == Status::SUB_CLASS) {
            Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                CodeCheckDiagKind::G_FUN_03_params_in_overload_funcs, funcDecl->identifier.Val());
        }
        allFuncs.insert(funcInfo);
    }
}

void StructuralRuleGFUNC03::FileProcessor(Ptr<Node> node)
{
    auto file = StaticCast<AST::File*>(node);
    auto& decls = file->decls;
    for (auto& decl : decls) {
        // The second part of the condition is for checking user code that does not conform to Codira syntax,
        // to prevent compilation failure.
        if (decl->astKind != ASTKind::FUNC_DECL || decl->ty->kind != TypeKind::TYPE_FUNC) {
            continue;
        }
        auto funcDecl = StaticCast<AST::FuncDecl*>(decl.get());
        auto functy = DynamicCast<AST::FuncTy*>(funcDecl->ty);
        if (!functy) {
            continue;
        }
        auto paramTys = functy->paramTys;
        for (auto modifier : funcDecl->modifiers) {
            if (modifier.modifier== TokenKind::COMMON || modifier.modifier == TokenKind::PLATFORM) {
                return;
            }
        }
        auto funcInfo = FuncInfo(funcDecl->identifier.GetRawText(), paramTys, nullptr);
        auto status = CheckFuncDecl(allFuncs, funcInfo);
        if (status == Status::SUB_CLASS) {
            Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                CodeCheckDiagKind::G_FUN_03_params_in_overload_funcs, funcDecl->identifier.Val());
        }
        allFuncs.insert(funcInfo);
    }
}

StructuralRuleGFUNC03::Status StructuralRuleGFUNC03::CheckFuncDecl(std::set<FuncInfo>& allFuncs, FuncInfo target)
{
    for (auto func : allFuncs) {
        if (func.identifier != target.identifier) {
            continue;
        }
        if (func.block != target.block && IsNotSameParams(func.paramTys, target.paramTys)) {
            return Status::IN_DIFF_SCOPE;
        }
        if (func.block == target.block && HasParentChildTypeRelation(func.paramTys, target.paramTys)) {
            return Status::SUB_CLASS;
        }
    }
    return Status::NONE;
}

void StructuralRuleGFUNC03::CheckFuncOverload(Ptr<Node> node)
{
    if (!node) {
        return;
    }

    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::FILE) {
            FileProcessor(node);
        }
        if (node->astKind == ASTKind::FUNC_DECL) {
            if (!blocks.empty()) {
                FuncDeclProcessor(node);
            }
        }
        if (node->astKind == ASTKind::BLOCK) {
            auto block = StaticCast<AST::Block*>(node);
            blocks.push(block);
        }
        return VisitAction::WALK_CHILDREN;
    };

    auto postVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::BLOCK) {
            auto block = StaticCast<AST::Block*>(node);
            for (auto func : localFuncInBlock[block]) {
                allFuncs.erase(func);
            }
            localFuncInBlock.erase(block);
            if (!blocks.empty()) {
                blocks.pop();
            }
        }
        return VisitAction::KEEP_DECISION;
    };

    Walker walker(node, preVisit, postVisit);
    walker.Walk();
}

void StructuralRuleGFUNC03::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckFuncOverload(node);
}
} // namespace Codira::CodeCheck
