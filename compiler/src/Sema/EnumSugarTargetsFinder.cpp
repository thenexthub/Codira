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

/**
 * @file
 *
 * This file implements the EnumSugarTargetFiner class.
 */

#include "EnumSugarTargetsFinder.h"

#include <vector>

#include "TypeCheckUtil.h"

#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Utils.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;

std::vector<Ptr<Decl>> EnumSugarTargetsFinder::FindEnumSugarTargets()
{
    if (refExpr.TestAttr(Attribute::MACRO_INVOKE_BODY)) {
        return {};
    }
    // 'Lookup' is only able to found target from exactly one enum decl.
    if (IsAllFuncDecl(enumSugarTargets)) {
        // No shadow happened if targets are all funcDecl, need to re-find all target globally.
        enumSugarTargets.clear();
    } else {
        RefineTargets();
    }
    if (enumSugarTargets.empty()) {
        size_t argSize = refExpr.OuterArgSize();
        auto decls = ctx.FindEnumConstructor(refExpr.ref.identifier, argSize);
        if (argSize != 0) { // for `()` operator overloading
            auto& varDecls = ctx.FindEnumConstructor(refExpr.ref.identifier, 0);
            decls.insert(decls.end(), varDecls.cbegin(), varDecls.cend());
        }
        Utils::EraseIf(decls, [this](auto decl) {
            CODEC_NULLPTR_CHECK(decl);
            auto& ed = *StaticCast<EnumDecl*>(decl->outerDecl);
            // Filter out the `enum`s that mismatch the number of type arguments.
            return !refExpr.typeArguments.empty() &&
                (ed.generic == nullptr || (refExpr.typeArguments.size() != ed.generic->typeParameters.size()));
        });
        if (refExpr.TestAttr(Attribute::IN_CORE)) {
            std::copy_if(decls.cbegin(), decls.cend(), std::back_inserter(enumSugarTargets),
                [](auto decl) { return decl->fullPackageName == CORE_PACKAGE_NAME; });
        } else {
            // Only keep toplevel `enum`s if `refExpr` doesn't have target ty.
            std::copy_if(decls.cbegin(), decls.cend(), std::back_inserter(enumSugarTargets),
                [this](auto decl) { return ctx.HasTargetTy(&refExpr) || decl->IsSamePackage(refExpr); });
            // If toplevel is empty, keep the imported `enum`s.
            if (enumSugarTargets.empty()) {
                enumSugarTargets = decls;
            }
        }
        RefineTargets();
    }
    auto it = std::unique(enumSugarTargets.begin(), enumSugarTargets.end());
    enumSugarTargets.resize(static_cast<size_t>(std::distance(enumSugarTargets.begin(), it)));
    std::sort(enumSugarTargets.begin(), enumSugarTargets.end(), CmpNodeByPos());
    return enumSugarTargets;
}

void EnumSugarTargetsFinder::RefineTargets()
{
    if (!ctx.HasTargetTy(&refExpr) || refExpr.callOrPattern != nullptr) {
        return;
    }
    std::vector<Ptr<AST::Decl>> inCandidates = enumSugarTargets;
    for (auto it = enumSugarTargets.begin(); it != enumSugarTargets.end();) {
        auto targetTy = ctx.targetTypeMap[&refExpr];
        if (auto refinedTargetTy = RefineTargetTy(tyMgr, targetTy, *it)) {
            ctx.targetTypeMap[&refExpr] = *refinedTargetTy;
            ++it;
        } else {
            it = enumSugarTargets.erase(it);
        }
    }
    // If there is no target left after refining, restore targets in current package,
    // OR if targets are all imported, retore all of them.
    if (enumSugarTargets.empty()) {
        bool hasTargetInCurrentPkg =
            Utils::In(inCandidates, [](auto it) { return it && !it->TestAttr(Attribute::IMPORTED); });
        if (hasTargetInCurrentPkg) {
            std::copy_if(inCandidates.begin(), inCandidates.end(), std::back_inserter(enumSugarTargets),
                [](auto it) { return it && !it->TestAttr(Attribute::IMPORTED); });
        } else {
            enumSugarTargets = inCandidates;
        }
    }
}

// Get real enum type of the given target. Only return value with valid type.
std::optional<Ptr<AST::Ty>> EnumSugarTargetsFinder::RefineTargetTy(
    TypeManager& typeManager, Ptr<Ty> targetTy, Ptr<const Decl> target)
{
    if (!target || !targetTy) {
        return {};
    }
    Ptr<Ty> currentTy = targetTy;
    while (currentTy != nullptr && currentTy->kind == TypeKind::TYPE_ENUM) {
        auto targetEnumTy = RawStaticCast<EnumTy*>(currentTy);
        if (targetEnumTy->declPtr == target->outerDecl) {
            return currentTy;
        }
        // Option type allow type auto box.
        if (targetEnumTy->IsCoreOptionType()) {
            currentTy = targetEnumTy->typeArgs[0];
        } else {
            return {};
        }
    }
    CODEC_ASSERT(target->outerDecl);
    // When target type is enum implemented interface type, directly return current enum type.
    if (auto currentInterfaceTy = DynamicCast<InterfaceTy*>(currentTy); currentInterfaceTy && target->outerDecl->ty) {
        auto allInterfaceTys = typeManager.GetAllSuperTys(*target->outerDecl->ty);
        if (allInterfaceTys.count(currentInterfaceTy) > 0) {
            return target->outerDecl->ty;
        }
        for (auto ty : allInterfaceTys) {
            if (auto iTy = DynamicCast<InterfaceTy*>(ty); iTy && iTy->declPtr == currentInterfaceTy->declPtr) {
                return target->outerDecl->ty;
            }
        }
    }
    return {};
}
