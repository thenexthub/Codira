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
 * This file declares typecheck handlers for Objective-C mirror declarations.
 */

#ifndef CODIRA_SEMA_OBJ_C_TYPECHECK_HANDLERS
#define CODIRA_SEMA_OBJ_C_TYPECHECK_HANDLERS

#include "NativeFFI/ObjC/Utils/Handler.h"
#include "Context.h"

namespace Codira::Interop::ObjC {

class CheckInterface : public Handler<CheckInterface, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckAbstractClass : public Handler<CheckAbstractClass, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckMultipleInherit : public Handler<CheckMultipleInherit, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckMirrorSubtypeAttr : public Handler<CheckMirrorSubtypeAttr, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckMirrorInheritMirror : public Handler<CheckMirrorInheritMirror, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckImplInheritMirror : public Handler<CheckImplInheritMirror, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckGeneric : public Handler<CheckGeneric, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

class CheckMemberTypes : public Handler<CheckMemberTypes, TypeCheckContext> {
public:
    explicit CheckMemberTypes()
    {
    }
    void HandleImpl(TypeCheckContext& ctx);

private:
    void CheckFuncTypes(AST::FuncDecl& fd, TypeCheckContext& ctx);
    void CheckFuncParamTypes(AST::FuncDecl& fd, TypeCheckContext& ctx);
    void CheckFuncRetType(AST::FuncDecl& fd, TypeCheckContext& ctx);

    void CheckPropTypes(AST::PropDecl& pd, TypeCheckContext& ctx);
    void CheckVarTypes(AST::VarDecl& vd, TypeCheckContext& ctx);
    std::string GetDeclInteropName();
};

class CheckTopLevelFuncTypes : public Handler<CheckTopLevelFuncTypes, TypeCheckContext> {
public:
    void HandleImpl(TypeCheckContext& ctx);
};

} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_OBJ_C_TYPECHECK_HANDLERS
