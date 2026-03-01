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
 * This file implements checks of types used with ObjCPointer
 */


#include "NativeFFI/Utils.h"
#include "NativeFFI/ObjC/Utils/Common.h"
#include "Codira/AST/Walker.h"
#include "Handlers.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void CheckObjCPointerTypeArguments::HandleImpl(InteropContext& ctx)
{
    for (auto& file : ctx.pkg.files) {
        Walker(file, Walker::GetNextWalkerID(), [&file, &ctx](auto node) {
            if (!node->IsSamePackage(*file->curPackage)) {
                return VisitAction::WALK_CHILDREN;
            }
            Ptr<Type> typeUsage = As<ASTKind::TYPE>(node);
            if (typeUsage
                && typeUsage->GetTypeArgs().size() == 1
                && ctx.typeMapper.IsObjCPointer(*typeUsage->ty)
                && !ctx.typeMapper.IsObjCCompatible(*typeUsage->ty->typeArgs[0])) {
                ctx.diag.DiagnoseRefactor(
                    DiagKindRefactor::sema_objc_pointer_argument_must_be_objc_compatible, 
                    *typeUsage);
                typeUsage->EnableAttr(Attribute::IS_BROKEN);
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();
    }
}
