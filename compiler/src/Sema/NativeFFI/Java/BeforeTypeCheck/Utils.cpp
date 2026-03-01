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

#include "Utils.h"
#include "NativeFFI/Utils.h"
#include "Codira/Utils/CastingTemplate.h"
#include "Codira/AST/Create.h"
#include "TypeCheckUtil.h"

using namespace Codira;
using namespace Codira::Native::FFI;
using namespace Codira::AST;

namespace {
using namespace Codira::TypeCheckUtil;
using namespace Codira::Interop::Java;

void InsertMethodStub(FuncDecl& fd, const ImportManager& importManager, TypeManager& typeManager)
{
    CODEC_ASSERT(fd.funcBody);
    auto argTy = GetStringDecl(importManager).ty;
    auto arg = CreateLitConstExpr(LitConstKind::STRING, "It's compiler generated stub.", argTy);
    std::vector<OwnedPtr<Expr>> args;
    args.emplace_back(std::move(arg));

    static auto& exception = GetExceptionDecl(importManager);
    auto throwExpr = CreateThrowException(exception, std::move(args), *fd.curFile, typeManager);

    std::vector<OwnedPtr<Node>> nodes;
    nodes.emplace_back(std::move(throwExpr));

    fd.funcBody->body = CreateBlock(std::move(nodes));
}
}

namespace Codira::Interop::Java {

void InsertJavaHasDefaultMethodStubs(
    const InterfaceDecl& id,
    const ImportManager& importManager,
    TypeManager& typeManager)
{
    for (auto& decl : id.GetMemberDeclPtrs()) {
        if (auto fd = As<ASTKind::FUNC_DECL>(decl);
            fd && fd->TestAttr(Attribute::JAVA_HAS_DEFAULT)) {
            InsertMethodStub(*fd, importManager, typeManager);
        }
    }
}

void RemoveAbstractAttributeForJavaHasDefaultMethods(const InterfaceDecl& decl)
{
    for (const auto& member : decl.GetMemberDeclPtrs()) {
        if (member->TestAttr(Attribute::JAVA_HAS_DEFAULT)) {
            member->DisableAttr(Attribute::ABSTRACT);
            /*
            code and java have different typechecks,
            default attribute makes this difference.
            */
            member->EnableAttr(Attribute::DEFAULT);
        }
    }
}

ClassDecl& GetExceptionDecl(const ImportManager& importManager)
{
    const auto exceptionDecl = importManager.GetCoreDecl("Exception");
    CODEC_NULLPTR_CHECK(exceptionDecl);
    
    ClassDecl* exception = nullptr;
    if (auto ex = As<ASTKind::CLASS_DECL>(exceptionDecl)) {
        exception = ex;
    }
    CODEC_NULLPTR_CHECK(exception);

    return *exception;
}

}
