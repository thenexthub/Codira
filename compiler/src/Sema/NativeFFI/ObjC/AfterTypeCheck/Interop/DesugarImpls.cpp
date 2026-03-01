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
 * This file implements desugaring of @ObjCImpl.
 */

#include "Handlers.h"
#include "NativeFFI/ObjC/Utils/Common.h"
#include "NativeFFI/Utils.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Walker.h"

using namespace Codira::AST;
using namespace Codira::Native::FFI;
using namespace Codira::Interop::ObjC;

void DesugarImpls::HandleImpl(InteropContext& ctx)
{
    for (auto& impl : ctx.impls) {
        if (impl->TestAttr(Attribute::IS_BROKEN)) {
            continue;
        }

        for (auto& memberDecl : impl->GetMemberDeclPtrs()) {
            if (memberDecl->TestAttr(Attribute::IS_BROKEN)) {
                continue;
            }

            switch (memberDecl->astKind) {
                case ASTKind::FUNC_DECL: {
                    auto& fd = *StaticAs<ASTKind::FUNC_DECL>(memberDecl);

                    if (fd.TestAttr(Attribute::CONSTRUCTOR)) {
                        DesugarCtor(ctx, *impl, fd);
                    } else {
                        DesugarMethod(ctx, *impl, fd);
                    }
                    break;
                }
                case ASTKind::PROP_DECL: {
                    Desugar(ctx, *impl, *StaticAs<ASTKind::PROP_DECL>(memberDecl));
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void DesugarImpls::DesugarMethod(
    [[maybe_unused]]
    InteropContext& ctx,
    [[maybe_unused]]
    ClassDecl& impl,
    [[maybe_unused]]
    FuncDecl& method)
{
}

void DesugarImpls::DesugarCtor([[maybe_unused]] InteropContext& ctx, [[maybe_unused]] ClassDecl& impl, FuncDecl& ctor)
{
    if (!ctx.factory.IsGeneratedCtor(ctor)) {
        ctor.funcBody->body = CreateBlock(Nodes(ctx.factory.CreateThrowUnreachableCodeExpr(*ctor.curFile)),
            TypeManager::GetPrimitiveTy(TypeKind::TYPE_NOTHING));

        return;
    }
}

void DesugarImpls::Desugar(InteropContext& ctx, ClassDecl& impl, PropDecl& prop)
{
    for (auto& getter : prop.getters) {
        DesugarMethod(ctx, impl, *getter.get());
    }

    for (auto& setter : prop.setters) {
        DesugarMethod(ctx, impl, *setter.get());
    }
}
