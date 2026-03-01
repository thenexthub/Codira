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
 * This file implements check that @ObjCImpl annotated declaration inherits an @ObjCMirror declaration.
 */

#include "Handlers.h"
#include "Codira/AST/Match.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void CheckImplInheritMirror::HandleImpl(TypeCheckContext& ctx)
{
    if (!ctx.typeMapper.IsObjCImpl(*ctx.target.ty)) {
        return;
    }

    if (ctx.typeMapper.IsObjCMirrorSubtype(*ctx.target.ty)) {
        return;
    }

    ctx.diag.DiagnoseRefactor(DiagKindRefactor::sema_objc_mirror_subtype_must_inherit_mirror, ctx.target);
    ctx.target.EnableAttr(Attribute::IS_BROKEN);
}
