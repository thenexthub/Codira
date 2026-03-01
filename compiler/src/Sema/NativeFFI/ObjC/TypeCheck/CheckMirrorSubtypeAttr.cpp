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
 * This file implements check that Objective-C mirror subtypes declaration MUST be annotated
 * either with @ObjCImpl or with @ObjCImpl (which leads to have an OBJ_C_MIRROR_SUBTYPE attribute, enabled by Parser).
 */

#include "Handlers.h"
#include "Codira/AST/Match.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void CheckMirrorSubtypeAttr::HandleImpl(TypeCheckContext& ctx)
{
    auto& ty = *ctx.target.ty;
    if (!ctx.typeMapper.IsObjCMirrorSubtype(ty)) {
        return;
    }

    if ((ctx.typeMapper.IsValidObjCMirror(ty) || ctx.typeMapper.IsObjCImpl(ty))) {
        return;
    }

    ctx.diag.DiagnoseRefactor(DiagKindRefactor::sema_objc_mirror_subtype_must_be_annotated, ctx.target);
    ctx.target.EnableAttr(Attribute::IS_BROKEN);
}
