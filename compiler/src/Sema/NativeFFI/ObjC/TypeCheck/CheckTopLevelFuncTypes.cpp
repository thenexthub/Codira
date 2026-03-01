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
 * This file implements checks for Objective-C mirror/subtype member declarations.
 */

#include "Handlers.h"
#include "Codira/AST/Match.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void CheckTopLevelFuncTypes::HandleImpl(TypeCheckContext& ctx)
{
    auto fd = As<ASTKind::FUNC_DECL>(&ctx.target);
    if (fd == nullptr) {
        return;
    }

    for (auto& paramList : fd->funcBody->paramLists) {
        for (auto& param : paramList->params) {
            if (!ctx.typeMapper.IsObjCCompatible(*param->ty)) {
                ctx.diag.DiagnoseRefactor(DiagKindRefactor::sema_objc_interop_toplevel_param_must_be_objc_compatible,
                    *param->type, fd->identifier.Val());

                fd->EnableAttr(Attribute::IS_BROKEN);
            }
        }
    }
    if (fd->funcBody->retType && !ctx.typeMapper.IsObjCCompatible(*fd->funcBody->retType->ty)) {
        ctx.diag.DiagnoseRefactor(DiagKindRefactor::sema_objc_interop_toplevel_ret_must_be_objc_compatible,
            *fd->funcBody->retType, fd->identifier.Val());

        fd->EnableAttr(Attribute::IS_BROKEN);
    }
}
