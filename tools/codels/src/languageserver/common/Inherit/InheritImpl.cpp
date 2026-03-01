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

#include "InheritImpl.h"
#include "../Utils.h"

using namespace Codira;

namespace ark {
void HandleSuperDecl(std::queue<Ptr<InheritableDecl> > &queues, InheritableDecl &inheritableDecl,
                     std::vector<Ptr<InheritableDecl> > &topClasses, std::vector<Ptr<InheritableDecl> > &libClasses)
{
    size_t invalidDeclCount = 0;
    for (auto &it : inheritableDecl.inheritedTypes) {
        if (it->ty == nullptr) {
            invalidDeclCount++;
            continue;
        }
        Ptr<ClassLikeDecl> superDecl = nullptr;
        if (it->ty->kind == TypeKind::TYPE_CLASS) {
            superDecl = dynamic_cast<ClassTy *>(it->ty.get())->decl;
        } else if (it->ty->kind == TypeKind::TYPE_INTERFACE) {
            superDecl = dynamic_cast<InterfaceTy *>(it->ty.get())->decl;
        }
        if (!superDecl) {
            invalidDeclCount++;
            continue;
        }
        if (IsFromSrcOrNoSrc(superDecl)) {
            queues.push(superDecl);
        } else {
            libClasses.push_back(superDecl);
            invalidDeclCount++;
        }
    }

    if (invalidDeclCount == inheritableDecl.inheritedTypes.size()) {
        topClasses.push_back(&inheritableDecl);
    }
}

std::vector<Ptr<InheritableDecl> > GetTopClassDecl(InheritableDecl &classLikeOrStructDecl, bool isLib)
{
    std::vector<Ptr<InheritableDecl> > topClasses{};
    std::vector<Ptr<InheritableDecl> > libClasses{};
    std::queue<Ptr<InheritableDecl> > queues{};
    queues.push(&classLikeOrStructDecl);
    while (!queues.empty()) {
        auto topClass = queues.front();
        queues.pop();
        if (topClass) {
            HandleSuperDecl(queues, *topClass, topClasses, libClasses);
        }
    }
    if (isLib) {
        return libClasses;
    }
    return topClasses;
}
} // namespace ark
