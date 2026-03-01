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

#include "StructuralRuleGENU02.h"
#include <regex>

using namespace Codira;
using namespace AST;
using namespace Meta;
using namespace CodeCheck;

// Check whether there is an inheritance relationship between classes/interfaces.
bool StructuralRuleGENU02::CheckTyEqualityHelper(Codira::AST::Ty* base, Codira::AST::Ty* derived)
{
    if (derived->IsClassLike()) {
        // Same Class/Interface
        if (base == derived) {
            return true;
        }
        auto classDecl = static_cast<Codira::AST::ClassLikeDecl*>(Codira::AST::Ty::GetDeclOfTy(derived).get());
        // Explicit Inheritance When Class Is Declared
        for (auto& super : classDecl->inheritedTypes) {
            if (super->ty == base) {
                return true;
            }
            if (CheckTyEqualityHelper(base, super->ty)) {
                return true;
            }
        }
        // Inheritance relationship defined by extension
        for (auto& super : inheritedClassMap[derived]) {
            if (super->ty == base) {
                return true;
            }
            if (CheckTyEqualityHelper(base, super->ty)) {
                return true;
            }
        }
    }
    return false;
}

// For non-class types, the types must be the same.
// For class/interface, types must be the same or have the inheritance relationship.
bool StructuralRuleGENU02::IsEqual(Codira::AST::Ty* base, Codira::AST::Ty* derived)
{
    if (!base->IsClassLike()) {
        if (base->kind == derived->kind || base->kind == TypeKind::TYPE_GENERICS ||
            derived->kind == TypeKind::TYPE_GENERICS) {
            return true;
        } else {
            return false;
        }
    }
    return CheckTyEqualityHelper(base, derived) || CheckTyEqualityHelper(derived, base);
}

void StructuralRuleGENU02::DuplicatedEnumCtrOrFuncHelper(const Codira::AST::FuncDecl& funcDecl)
{
    auto& params = funcDecl.funcBody->paramLists[0]->params;
    std::vector<AST::Ty*> args;
    for (size_t i = 0; i < params.size(); i++) {
        args.emplace_back(params[i]->ty);
    }
    auto isEnumCtr = funcDecl.TestAttr(Attribute::ENUM_CONSTRUCTOR);
    auto enumCtr = EnumCtr(funcDecl.identifier, args, isEnumCtr);
    auto filter = [&enumCtr, this](const EnumCtr& item) {
        if (item.identifier != enumCtr.identifier || item.args.size() != enumCtr.args.size()) {
            return false;
        }
        for (size_t i = 0; i < item.args.size(); i++) {
            if (!IsEqual(item.args[i], enumCtr.args[i])) {
                return false;
            }
        }
        return true;
    };
    auto iter = std::find_if(enumCtrSet.begin(), enumCtrSet.end(), filter);
    if (iter != enumCtrSet.end() && (isEnumCtr || iter->isCtr)) {
        Diagnose(funcDecl.identifier.Begin(), funcDecl.identifier.End(),
            CodeCheckDiagKind::G_ENU_02_enum_constructor_overload_information, funcDecl.identifier.Val().c_str());
    } else {
        enumCtrSet.insert(enumCtrSet.end(), enumCtr);
    }
}

void StructuralRuleGENU02::CheckEnumCtrOverload(const Codira::AST::EnumDecl& enumDecl)
{
    for (auto& constructor : enumDecl.constructors) {
        if (constructor->astKind == ASTKind::FUNC_DECL) {
            auto funcDecl = static_cast<FuncDecl*>(constructor.get().get());
            DuplicatedEnumCtrOrFuncHelper(*funcDecl);
        }
    }
}

void StructuralRuleGENU02::CheckFuncOverload(const Codira::AST::FuncDecl& funcDecl)
{
    if (funcDecl.scopeLevel == 0 && !funcDecl.outerDecl) {
        DuplicatedEnumCtrOrFuncHelper(funcDecl);
    }
}

void StructuralRuleGENU02::FindEnumDeclHelper(Ptr<Node> node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                CheckFuncOverload(funcDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const EnumDecl& enumDecl) {
                CheckEnumCtrOverload(enumDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGENU02::FindExtendHelper(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ExtendDecl& extendDecl) {
                if (extendDecl.extendedType->ty->IsClassLike()) {
                    for (auto& type : extendDecl.inheritedTypes) {
                        inheritedClassMap[extendDecl.extendedType->ty].emplace_back(type.get());
                    }
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGENU02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    if (node->astKind ==  ASTKind::PACKAGE) {
        auto pkg = static_cast<Package*>(node.get());
        for (auto& file: pkg->files) {
            enumCtrSet.clear();
            inheritedClassMap.clear();
            // Collect the inheritance relationships between classes/interfaces through extendDecl
            FindExtendHelper(file);
            // Check enum's constructor and top-level functions
            FindEnumDeclHelper(file);
        }
    }
}
