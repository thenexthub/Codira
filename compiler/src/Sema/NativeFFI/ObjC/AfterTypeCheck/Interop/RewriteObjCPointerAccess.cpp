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
 * This file implements desugaring of ObjCPointer struct accessors
 */


#include "NativeFFI/Utils.h"
#include "NativeFFI/ObjC/Utils/Common.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Walker.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Clone.h"
#include "Handlers.h"

using namespace Codira::AST;
using namespace Codira::Interop::ObjC;
using namespace Codira::Native::FFI;
using namespace Codira;

namespace {

constexpr auto READ_POINTER_INTRINSIC = "readPointer";
constexpr auto WRITE_POINTER_INTRINSIC = "writePointer";
constexpr auto OBJCPOINTER_READ_METHOD = "read";
constexpr auto OBJCPOINTER_WRITE_METHOD = "write";

OwnedPtr<Expr> CreateZero(Ptr<Ty> ty)
{
    return CreateLitConstExpr(LitConstKind::INTEGER, "0", ty);
}

/**
 * if T is CType
 * PTR.read() -> readPointer<T>(CPointer<T>(PTR.ptr))
 *
 * if T is itself ObjCPointer<U>
 * PTR.read() -> ObjCPointer<U>(readPointer<CPointer<Unit>>(CPointer<CPointer<Unit>>(PTR.ptr)))
 *
 * if T is @ObjCImpl/@ObjCMirror class
 * PTR.read() -> T(readPointer<NativeHandle>(CPointer<NativeHandle>(PTR.ptr)))
 *
 * if T is @ObjCImpl/@ObjCMirror interface -> (not implemented yet)
 * PTR.read() -> T$WrapperClass(readPointer<NativeHandle>(CPointer<NativeHandle>(PTR.ptr)))
 *
 * All these cases can be unified as
 * WRAP(readPointer<NATIVE_C_TYPE>(CPointer<NATIVE_C_TYPE>(PTR.ptr)))
 * where NATIVE_C_TYPE is the C type representation of T
 *       WRAP(expr) is the construction procedure of T from NATIVE_C_TYPE
*/
void HandleObjCPointerRead(InteropContext& ctx, CallExpr& callExpr)
{
    auto ma = As<ASTKind::MEMBER_ACCESS>(callExpr.baseFunc);
    CODEC_NULLPTR_CHECK(ma);
    auto receiver = ASTCloner::Clone<Expr>(ma->baseExpr);
    CODEC_NULLPTR_CHECK(receiver);
    auto elementType = receiver->ty->typeArgs[0];
    auto rawCType = ctx.typeMapper.Code2CType(elementType);
    Ptr<Ty> pointerType = ctx.typeManager.GetPointerTy(rawCType);
    auto ptrFieldDecl = ctx.factory.GetObjCPointerPointerField();
    CODEC_ASSERT(ptrFieldDecl);
    Ptr<Ty> int64Type = ctx.typeManager.GetPrimitiveTy(TypeKind::TYPE_INT64);

    auto readPointerFunc = ctx.importManager.GetCoreDecl<FuncDecl>(READ_POINTER_INTRINSIC);
    CODEC_NULLPTR_CHECK(readPointerFunc);
    auto readPointerRef = CreateRefExpr(*readPointerFunc, callExpr);
    readPointerRef->instTys.push_back(rawCType);
    readPointerRef->ty = ctx.typeManager.GetFunctionTy(std::vector { pointerType, int64Type }, rawCType);

    auto ptrExpr = ctx.factory.CreateUnsafePointerCast(
        CreateMemberAccess(std::move(receiver), *ptrFieldDecl),
        rawCType);
    CopyBasicInfo(&callExpr, ptrExpr);
    auto callArgs = std::vector<OwnedPtr<FuncArg>> {};
    callArgs.emplace_back(CreateFuncArg(std::move(ptrExpr)));
    callArgs.emplace_back(CreateFuncArg(CreateZero(int64Type)));
    auto call = CreateCallExpr(
        std::move(readPointerRef),
        std::move(callArgs),
        readPointerFunc,
        rawCType,
        CallKind::CALL_INTRINSIC_FUNCTION);
    CopyBasicInfo(&callExpr, call);
    auto wrapExpr = ctx.factory.WrapEntity(std::move(call), *elementType);
    wrapExpr->sourceExpr = &callExpr;
    callExpr.desugarExpr = std::move(wrapExpr);
    callExpr.desugarArgs = std::nullopt;
}

/**
 * if T is CType
 * PTR.write(V) -> writePointer<T>(CPointer<T>(PTR.ptr), V)
 *
 * if T is itself ObjCPointer<U>
 * PTR.write(V) -> writePointer<CPointer<Unit>>(CPointer<CPointer<Unit>>(PTR.ptr), V.ptr)
 *
 * if T is @ObjCImpl/@ObjCMirror class
 * PTR.write(V) -> writePointer<NativeHandle>(CPointer<NativeHandle>(PTR.ptr), V.$obj)
 *
 * if T is @ObjCImpl/@ObjCMirror interface -> (not implemented yet)
 * PTR.write(V) -> writePointer<NativeHandle>(CPointer<NativeHandle>(PTR.ptr)), V.$getObj())
 *
 * All these cases can be unified as
 * writePointer<NATIVE_C_TYPE>(CPointer<NATIVE_C_TYPE>(PTR.ptr), UNWRAP(V))
 * where NATIVE_C_TYPE is the C type representation of T
 *       UNWRAP(expr) is the extraction procedure of NATIVE_C_TYPE from T
*/
void HandleObjCPointerWrite(InteropContext& ctx, CallExpr& callExpr)
{
    auto ma = As<ASTKind::MEMBER_ACCESS>(callExpr.baseFunc);
    CODEC_NULLPTR_CHECK(ma);
    auto receiver = ASTCloner::Clone<Expr>(ma->baseExpr);
    CODEC_ASSERT(callExpr.args.size() == 1);
    auto valueArg = ASTCloner::Clone<Expr>(callExpr.args[0]->expr);
    CODEC_ASSERT_WITH_MSG(!receiver->ty->typeArgs.empty(), "ObjCPointer typeArgs is empty");
    auto elementType = receiver->ty->typeArgs[0];
    auto rawCType = ctx.typeMapper.Code2CType(elementType);
    Ptr<Ty> pointerType = ctx.typeManager.GetPointerTy(rawCType);
    auto ptrFieldDecl = ctx.factory.GetObjCPointerPointerField();
    CODEC_ASSERT(ptrFieldDecl);
    Ptr<Ty> int64Type = ctx.typeManager.GetPrimitiveTy(TypeKind::TYPE_INT64);
    Ptr<Ty> unitType = ctx.typeManager.GetPrimitiveTy(TypeKind::TYPE_UNIT);

    auto writePointerFunc = ctx.importManager.GetCoreDecl<FuncDecl>(WRITE_POINTER_INTRINSIC);
    CODEC_NULLPTR_CHECK(writePointerFunc);
    auto writePointerRef = CreateRefExpr(*writePointerFunc, callExpr);
    writePointerRef->instTys.push_back(rawCType);
    writePointerRef->ty =
        ctx.typeManager.GetFunctionTy({ pointerType, int64Type, rawCType }, unitType);

    auto ptrExpr = ctx.factory.CreateUnsafePointerCast(
        CreateMemberAccess(std::move(receiver), *ptrFieldDecl),
        rawCType);
    CopyBasicInfo(&callExpr, ptrExpr);
    auto callArgs = std::vector<OwnedPtr<FuncArg>> {};
    callArgs.emplace_back(CreateFuncArg(std::move(ptrExpr)));
    callArgs.emplace_back(CreateFuncArg(CreateZero(int64Type)));
    callArgs.emplace_back(CreateFuncArg(ctx.factory.UnwrapEntity(std::move(valueArg))));
    auto call = CreateCallExpr(
        std::move(writePointerRef),
        std::move(callArgs),
        writePointerFunc,
        unitType,
        CallKind::CALL_INTRINSIC_FUNCTION);
    CopyBasicInfo(&callExpr, call);
    call->sourceExpr = &callExpr;
    callExpr.desugarExpr = std::move(call);
    callExpr.desugarArgs = std::nullopt;
}
}

void RewriteObjCPointerAccess::HandleImpl(InteropContext& ctx)
{
    for (auto& file : ctx.pkg.files) {
        Walker(file, Walker::GetNextWalkerID(), [&file, &ctx](auto node) {
            if (!node->IsSamePackage(*file->curPackage)) {
                return VisitAction::WALK_CHILDREN;
            }
            Ptr<CallExpr> callExpr = As<ASTKind::CALL_EXPR>(node);
            if (!callExpr) {
                return VisitAction::WALK_CHILDREN;
            }

            auto funcDecl = callExpr->resolvedFunction;
            if (!funcDecl || !funcDecl->outerDecl || !ctx.typeMapper.IsObjCPointer(*funcDecl->outerDecl)) {
                return VisitAction::WALK_CHILDREN;
            }

            if (funcDecl->identifier == OBJCPOINTER_READ_METHOD
                && callExpr->args.size() == 0
                && ctx.typeMapper.IsObjCCompatible(*callExpr->ty)) {
                HandleObjCPointerRead(ctx, *callExpr);
                return VisitAction::WALK_CHILDREN;
            }

            if (funcDecl->identifier == OBJCPOINTER_WRITE_METHOD
                && callExpr->args.size() == 1
                && ctx.typeMapper.IsObjCCompatible(*callExpr->args[0]->ty)) {
                HandleObjCPointerWrite(ctx, *callExpr);
                return VisitAction::WALK_CHILDREN;
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();
    }
}
