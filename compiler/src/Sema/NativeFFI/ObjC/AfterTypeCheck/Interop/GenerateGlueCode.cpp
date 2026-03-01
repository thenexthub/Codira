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
 * This file implements generating Objective-C glue code.
 */

#include "NativeFFI/ObjC/ObjCCodeGenerator/ObjCGenerator.h"
#include "Handlers.h"
#include "Codira/Mangle/BaseMangler.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void GenerateGlueCode::HandleImpl(InteropContext& ctx)
{
    for (auto& impl : ctx.impls) {
        if (impl->TestAnyAttr(Attribute::IS_BROKEN, Attribute::HAS_BROKEN)) {
            continue;
        }

        auto codegen = ObjCGenerator(ctx, impl, "objc-gen", ctx.codeLibOutputPath);
        codegen.Generate();
    }
}
