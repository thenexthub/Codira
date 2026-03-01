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
 * This file implements confliction checking of generic upper bounds.
 */
#include "StructInheritanceChecker.h"

#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Types.h"
#include "Codira/AST/Utils.h"
#include "Codira/AST/Walker.h"
#include "Codira/Sema/TypeManager.h"

#include "TypeCheckUtil.h"

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;

namespace {
Ptr<Ty> GetSmallestClassTy(TypeManager& tyMgr, const std::set<Ptr<Ty>>& uppers)
{
    Ptr<Ty> smallest = nullptr;
    for (auto it : uppers) {
        if (!it->IsClass()) {
            continue;
        }
        if (!Ty::IsTyCorrect(smallest) || tyMgr.IsSubtype(it, smallest)) {
            smallest = it;
        }
    }
    return smallest;
}
} // namespace

void StructInheritanceChecker::CheckAllUpperBoundsConfliction()
{
    std::vector<Ptr<const Generic>> genericsWithConstraint;
    Walker(&pkg, [&genericsWithConstraint](auto node) {
        // Only incremental compilation case will meet generic instantiated decl. Ignore them all.
        if (node->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (auto generic = DynamicCast<Generic*>(node); generic && !generic->genericConstraints.empty()) {
            (void)genericsWithConstraint.emplace_back(generic);
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();

    for (auto generic : std::as_const(genericsWithConstraint)) {
        CheckUpperBoundsConfliction(*generic);
    }
}

void StructInheritanceChecker::CheckUpperBoundsConfliction(const Generic& generic)
{
    for (auto& gc : generic.genericConstraints) {
        CODEC_ASSERT(gc && gc->type);
        auto gTy = DynamicCast<GenericsTy*>(gc->type->ty);
        if (gTy == nullptr || gTy->decl->TestAttr(Attribute::IN_REFERENCE_CYCLE)) {
            continue; // Ignore invalid generic types.
        }
        // 'upperbounds' contains directly and indirectly defined non-generic upper bounds.
        auto uppers = gTy->upperBounds;
        if (uppers.empty()) {
            continue;
        }
        auto classTy = GetSmallestClassTy(typeManager, uppers);
        // 1. Erase non-interface from upperbounds.
        Utils::EraseIf(uppers, [](auto ty) { return !ty->IsInterface(); });
        MemberMap inheritedMembers;
        // 2. Merge members of interface upper bounds.
        for (auto iTy : uppers) {
            auto interfaceDecl = Ty::GetDeclPtrOfTy<InheritableDecl>(iTy);
            if (auto found = structInheritedMembers.find(interfaceDecl); found != structInheritedMembers.end()) {
                MergeInheritedMembers(inheritedMembers, found->second, *iTy, true);
            }
        }
        // 3. Update member if valid class upperBound existed.
        if (auto cd = Ty::GetDeclPtrOfTy<InheritableDecl>(classTy)) {
            auto members = GetInheritedSuperMembers(*cd, *classTy, *generic.curFile);
            // Since tys are upperbounds of generic, treat them as same inherited types to update 'inconstent' types.
            MergeInheritedMembers(inheritedMembers, members, *classTy, true);
        }
        // 4. Report for members which have conflict upperbounds.
        for (auto& [_, member] : std::as_const(inheritedMembers)) {
            if (member.inconsistentTypes.empty()) {
                continue;
            }
            DiagnoseInheritedInsconsistType(member, *gc);
        }
    }
}
