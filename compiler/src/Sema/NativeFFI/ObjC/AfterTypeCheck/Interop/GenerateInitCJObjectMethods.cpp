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
 * This file implements generating init Codira object method for @ObjCImpls.
 */

#include "Handlers.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void GenerateInitCODEObjectMethods::HandleImpl(InteropContext& ctx)
{
    auto genNativeInitMethod = [&ctx](Decl& decl) {
        if (decl.TestAttr(Attribute::IS_BROKEN)) {
            return;
        }

        for (auto& memberDecl : decl.GetMemberDeclPtrs()) {
            if (memberDecl->TestAttr(Attribute::IS_BROKEN)) {
                continue;
            }

            if (!memberDecl->TestAttr(Attribute::CONSTRUCTOR)) {
                continue;
            }

            if (!memberDecl->TestAttr(Attribute::PUBLIC)) {
                continue;
            }

            if (memberDecl->astKind != ASTKind::FUNC_DECL) {
                // skip primary ctor, as it is desugared to init already
                continue;
            }

            CODEC_ASSERT_WITH_MSG(memberDecl->astKind == ASTKind::FUNC_DECL,
                "Expected ASTKind::FUNC_DECL, found " + ASTKIND_TO_STR.at(memberDecl->astKind));

            auto& ctorDecl = *StaticAs<ASTKind::FUNC_DECL>(memberDecl);

            // skip original ctors
            if (!ctx.factory.IsGeneratedCtor(ctorDecl)) {
                continue;
            }

            auto initCodeObject = ctx.factory.CreateInitCodeObject(decl, ctorDecl, false);
            CODEC_ASSERT(initCodeObject);
            ctx.genDecls.emplace_back(std::move(initCodeObject));
        }
    };

    for (auto& impl : ctx.impls) {
        genNativeInitMethod(*impl);
    }

}
