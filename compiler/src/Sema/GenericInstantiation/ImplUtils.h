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
 * Define utils methods for generic instantiation.
 */

#ifndef CODIRA_SEMA_GENERIC_INSTANTIATION_UTILS_H
#define CODIRA_SEMA_GENERIC_INSTANTIATION_UTILS_H

#include <functional>

#include "Codira/AST/Node.h"
#include "Codira/Sema/TestManager.h"
namespace Codira {
/** Get @p decl 's sema type. If decl is extend decl, get it's extended sema type. */
inline Ptr<AST::Ty> GetDeclTy(const AST::Decl& decl)
{
    auto ty = decl.ty;
    if (decl.astKind == AST::ASTKind::EXTEND_DECL) {
        ty = static_cast<const AST::ExtendDecl&>(decl).extendedType->ty;
    }
    return ty;
}

inline Ptr<AST::Decl> GetOuterStructDecl(const AST::Decl& decl)
{
    auto outerDecl = decl.outerDecl;
    while (outerDecl && !outerDecl->IsNominalDecl()) {
        outerDecl = outerDecl->outerDecl;
    }
    return outerDecl;
}

/** Check if given @p decl is generic decl in generic structure declaration. */
inline bool IsGenericInGenericStruct(const AST::Decl& decl)
{
    auto outerDecl = GetOuterStructDecl(decl);
    return outerDecl && outerDecl->generic && outerDecl->IsNominalDecl() && decl.GetGeneric();
}

inline std::vector<Ptr<AST::Decl>> GetRealIndexingMembers(
    const std::vector<OwnedPtr<AST::Decl>>& decls, bool inGenericDecl = true)
{
    // NOTE: Filter primary constructors and members generated for test purpuses, for generic decls,
    // to get fixed members for indexing usage.
    std::vector<Ptr<AST::Decl>> ret;
    for (auto& member : decls) {
        if ((inGenericDecl && member->astKind == AST::ASTKind::PRIMARY_CTOR_DECL) ||
            TestManager::IsDeclGeneratedForTest(*member)
        ) {
            continue;
        }
        (void)ret.emplace_back(member.get());
    }
    return ret;
}

inline void WorkForMembers(AST::Decl& decl, const std::function<void(AST::Decl&)>& worker)
{
    if (decl.astKind == AST::ASTKind::PROP_DECL) {
        auto& pd = static_cast<AST::PropDecl&>(decl);
        std::for_each(pd.getters.begin(), pd.getters.end(), [&worker](auto& fd) { worker(*fd); });
        std::for_each(pd.setters.begin(), pd.setters.end(), [&worker](auto& fd) { worker(*fd); });
    } else {
        worker(decl);
    }
}

inline bool NeedSwitchContext(const AST::Decl& decl)
{
    auto outerDecl = GetOuterStructDecl(decl);
    return decl.IsNominalDecl() ||
        (outerDecl && outerDecl->IsNominalDecl() && (decl.GetGeneric() || decl.TestAttr(AST::Attribute::IMPORTED)));
}
} // namespace Codira
#endif
