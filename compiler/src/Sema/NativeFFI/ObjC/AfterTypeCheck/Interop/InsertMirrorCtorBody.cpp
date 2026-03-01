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
 * This file implements generating and inserting a constructor of handle to each objective-c mirror
 */

#include "NativeFFI/Utils.h"
#include "NativeFFI/ObjC/Utils/Common.h"
#include "Codira/AST/Create.h"
#include "Handlers.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;
using namespace Codira::Native::FFI;

void InsertMirrorCtorBody::HandleImpl(InteropContext& ctx)
{
    for (auto& mirror : ctx.mirrors) {
        if (mirror->TestAttr(Attribute::IS_BROKEN)) {
            continue;
        }
        auto mirrorClass = As<ASTKind::CLASS_DECL>(mirror);
        if (!mirrorClass) {
            continue;
        }

        auto ctor = ctx.factory.GetGeneratedMirrorCtor(*mirrorClass);
        CODEC_NULLPTR_CHECK(ctor);
        auto curFile = ctor->curFile;

        CODEC_ASSERT_WITH_MSG(!ctor->funcBody->paramLists[0]->params.empty(), "Ctor param list is empty");
        auto handleParam = WithinFile(CreateRefExpr(*ctor->funcBody->paramLists[0]->params[0]), curFile);

        if (HasMirrorSuperClass(*mirrorClass)) {
            auto superCtor = ctx.factory.GetGeneratedMirrorCtor(*mirrorClass->GetSuperClassDecl());
            auto superCall = CreateSuperCall(*mirrorClass, *superCtor, superCtor->ty);
            superCall->args.emplace_back(CreateFuncArg(std::move(handleParam)));
            ctor->funcBody->body->body.emplace_back(std::move(superCall));
        } else {
            auto lhs = CreateMemberAccess(CreateThisRef(mirrorClass, mirrorClass->ty, curFile),
                ASTFactory::NATIVE_HANDLE_IDENT);
            static auto unitTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
            auto nativeHandleAssignExpr = CreateAssignExpr(std::move(lhs), std::move(handleParam), unitTy);
            ctor->funcBody->body->body.emplace_back(std::move(nativeHandleAssignExpr));
        }
    }
}
