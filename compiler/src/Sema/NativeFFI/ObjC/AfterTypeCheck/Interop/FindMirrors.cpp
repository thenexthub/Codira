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
 * This file implements searching for Objective-C mirror declarations and theirs subtypes.
 */

#include "Handlers.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void FindMirrors::HandleImpl(InteropContext& ctx)
{
    for (auto& file : ctx.pkg.files) {
        for (auto& decl : file->decls) {
            if (auto classLikeDecl = As<ASTKind::CLASS_LIKE_DECL>(decl);
                classLikeDecl && ctx.typeMapper.IsObjCMirror(*classLikeDecl)) {
                ctx.mirrors.emplace_back(classLikeDecl);
            }

            if (auto classDecl = As<ASTKind::CLASS_DECL>(decl); classDecl &&
                (ctx.typeMapper.IsObjCImpl(*classDecl) ||
                    (ctx.typeMapper.IsObjCMirrorSubtype(*classDecl) && !ctx.typeMapper.IsObjCImpl(*classDecl) &&
                        !ctx.typeMapper.IsObjCMirror(*classDecl)))) {
                ctx.impls.emplace_back(classDecl);
            }

            if (auto funcDecl = As<ASTKind::FUNC_DECL>(decl);
                funcDecl && ctx.typeMapper.IsObjCMirror(*funcDecl)) {
                ctx.mirrorTopLevelFuncs.emplace_back(funcDecl);
            }
        }
    }
}
