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
 * This file implements inserting NativeObjCId field in Objective-C mirrors
 */

#include "NativeFFI/ObjC/Utils/Common.h"
#include "Handlers.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void InsertNativeHandleField::HandleImpl(InteropContext& ctx)
{
    for (auto& mirror : ctx.mirrors) {
        if (mirror->TestAttr(Attribute::IS_BROKEN)) {
            continue;
        }
        auto mirrorClass = As<ASTKind::CLASS_DECL>(mirror);
        if (!mirrorClass || HasMirrorSuperClass(*mirrorClass)) {
            continue;
        }

        auto nativeObjCIdField = ctx.factory.CreateNativeHandleField(*mirrorClass);
        mirrorClass->body->decls.emplace_back(std::move(nativeObjCIdField));
    }
}
