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
 * This file implements desugaring of Objective-C mirror subtypes.
 */

#include "NativeFFI/ObjC/Utils/Common.h"
#include "Handlers.h"
#include "Codira/AST/Match.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;

void GenerateWrappers::HandleImpl(InteropContext& ctx)
{
    auto genWrapper = [this, &ctx](Decl& decl) {
        if (decl.TestAttr(Attribute::IS_BROKEN)) {
            return;
        }

        for (auto& memberDecl : decl.GetMemberDeclPtrs()) {
            if (memberDecl->TestAnyAttr(Attribute::IS_BROKEN, Attribute::CONSTRUCTOR)) {
                continue;
            }
            if (!memberDecl->TestAnyAttr(Attribute::PUBLIC)) {
                continue;
            }

            if (ctx.factory.IsGeneratedMember(*memberDecl)) {
                continue;
            }

            switch (memberDecl->astKind) {
                case ASTKind::FUNC_DECL:
                    this->GenerateWrapper(ctx, *StaticAs<ASTKind::FUNC_DECL>(memberDecl));
                    break;
                case ASTKind::PROP_DECL:
                    this->GenerateWrapper(ctx, *StaticAs<ASTKind::PROP_DECL>(memberDecl));
                    break;
                case ASTKind::VAR_DECL:
                    this->GenerateWrapper(ctx, *StaticAs<ASTKind::VAR_DECL>(memberDecl));
                    break;
                default:
                    break;
            }
        }
    };

    for (auto& impl : ctx.impls) {
        genWrapper(*impl);
    }
}

void GenerateWrappers::GenerateWrapper(InteropContext& ctx, FuncDecl& method)
{
    auto wrapper = ctx.factory.CreateMethodWrapper(method);
    CODEC_NULLPTR_CHECK(wrapper);
    ctx.genDecls.emplace_back(std::move(wrapper));
}

void GenerateWrappers::GenerateWrapper(InteropContext& ctx, PropDecl& prop)
{
    auto wrapper = ctx.factory.CreateGetterWrapper(prop);
    CODEC_NULLPTR_CHECK(wrapper);
    ctx.genDecls.emplace_back(std::move(wrapper));

    if (prop.isVar) {
        GenerateSetterWrapper(ctx, prop);
    }
}

void GenerateWrappers::GenerateSetterWrapper(InteropContext& ctx, PropDecl& prop)
{
    auto wrapper = ctx.factory.CreateSetterWrapper(prop);
    CODEC_NULLPTR_CHECK(wrapper);
    ctx.genDecls.emplace_back(std::move(wrapper));
}

void GenerateWrappers::GenerateWrapper(InteropContext& ctx, VarDecl& field)
{
    if (ctx.factory.IsGeneratedNativeHandleField(field)) {
        return;
    }

    auto wrapper = ctx.factory.CreateGetterWrapper(field);
    CODEC_NULLPTR_CHECK(wrapper);
    ctx.genDecls.emplace_back(std::move(wrapper));

    if (field.isVar) {
        GenerateSetterWrapper(ctx, field);
    }
}

void GenerateWrappers::GenerateSetterWrapper(InteropContext& ctx, VarDecl& field)
{
    auto wrapper = ctx.factory.CreateSetterWrapper(field);
    CODEC_NULLPTR_CHECK(wrapper);
    ctx.genDecls.emplace_back(std::move(wrapper));
}
