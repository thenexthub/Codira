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

#include "JavaDesugarManager.h"
#include "NativeFFI/Java/JavaCodeGenerator/JavaSourceCodeGenerator.h"
#include "Utils.h"

#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"

namespace Codira::Interop::Java {
using namespace Codira::Native::FFI;

inline void JavaDesugarManager::PushEnvParams(std::vector<OwnedPtr<FuncParam>>& params, std::string name)
{
    auto jniEnvPtrDecl = lib.GetJniEnvPtrDecl();
    CODEC_NULLPTR_CHECK(jniEnvPtrDecl);
    params.push_back(CreateFuncParam(name, ASTCloner::Clone(jniEnvPtrDecl->type.get()), nullptr, lib.GetJNIEnvPtrTy()));
}

inline void JavaDesugarManager::PushObjParams(std::vector<OwnedPtr<FuncParam>>& params, std::string name)
{
    params.push_back(CreateFuncParam(name, lib.CreateJobjectType(), nullptr, lib.GetJobjectTy()));
}

inline void JavaDesugarManager::PushSelfParams(std::vector<OwnedPtr<FuncParam>>& params, std::string name)
{
    params.push_back(CreateFuncParam(name, lib.CreateJlongType(), nullptr, lib.GetJlongTy()));
}

bool JavaDesugarManager::FillMethodParamsByArg(std::vector<OwnedPtr<FuncParam>>& params,
    std::vector<OwnedPtr<FuncArg>>& callArgs, FuncDecl& funcDecl, OwnedPtr<FuncParam>& arg, FuncParam& jniEnvPtrParam)
{
    CODEC_NULLPTR_CHECK(funcDecl.curFile);
    auto jniArgTy = GetJNITy(arg->ty);
    OwnedPtr<FuncParam> param = CreateFuncParam(arg->identifier.GetRawText(), nullptr, nullptr, jniArgTy);
    auto classLikeTy = DynamicCast<ClassLikeTy*>(arg->ty);
    if (classLikeTy && !classLikeTy->commonDecl) {
        return false;
    }
    auto outerDecl = funcDecl.outerDecl;
    auto paramRef = WithinFile(CreateRefExpr(*param), funcDecl.curFile);
    OwnedPtr<FuncArg> methodArg;
    if (IsMirror(*arg->ty)) {
        auto entity = lib.CreateJavaEntityJobjectCall(std::move(paramRef));
        methodArg = CreateFuncArg(CreateMirrorConstructorCall(importManager, std::move(entity), arg->ty));
    } else if (IsImpl(*arg->ty)) {
        auto entity = lib.CreateJavaEntityJobjectCall(std::move(paramRef));
        methodArg = CreateFuncArg(lib.UnwrapJavaEntity(std::move(entity), arg->ty, *outerDecl));
    } else if (arg->ty->IsCoreOptionType() && IsMirror(*arg->ty->typeArgs[0])) {
        // funcDecl(Java_CFFI_JavaEntity(arg)) // if arg is null (as jobject == 0) -> java entity will preserve it
        auto entity = lib.CreateJavaEntityJobjectCall(std::move(paramRef));
        methodArg = CreateFuncArg(lib.UnwrapJavaEntity(std::move(entity), arg->ty, *outerDecl));
    } else if (arg->ty->IsCoreOptionType() && IsImpl(*arg->ty->typeArgs[0])) {
        auto entity = lib.CreateJavaEntityJobjectCall(std::move(paramRef));
        methodArg = CreateFuncArg(lib.UnwrapJavaEntity(std::move(entity), arg->ty, *outerDecl));
    } else if (IsCODEMapping(*arg->ty)) {
        auto entity = lib.CreateGetFromRegistryCall(
            WithinFile(CreateRefExpr(jniEnvPtrParam), funcDecl.curFile), std::move(paramRef), arg->ty);
        methodArg = CreateFuncArg(WithinFile(std::move(entity), funcDecl.curFile));
    } else {
        methodArg = CreateFuncArg(std::move(paramRef));
    }

    params.push_back(std::move(param));
    callArgs.push_back(std::move(methodArg));
    return true;
}

OwnedPtr<Decl> JavaDesugarManager::GenerateNativeMethod(FuncDecl& sampleMethod, Decl& decl)
{
    auto curFile = sampleMethod.curFile;
    CODEC_NULLPTR_CHECK(curFile);
    auto retTy = StaticCast<FuncTy*>(sampleMethod.ty)->retTy;

    std::vector<OwnedPtr<FuncParam>> params;
    PushEnvParams(params);
    // jobject or jclass
    PushObjParams(params, "_");
    CODEC_ASSERT_WITH_MSG(!params.empty(), "jniEnvPtrParam is absent");
    auto& jniEnvPtrParam = *params[0];
    if (!sampleMethod.TestAttr(Attribute::STATIC)) {
        PushSelfParams(params);
    }
    auto& selfParam = *params.back();

    std::vector<OwnedPtr<FuncArg>> methodCallArgs;
    CODEC_NULLPTR_CHECK(sampleMethod.funcBody);
    CODEC_ASSERT_WITH_MSG(!sampleMethod.funcBody->paramLists.empty(), "paramLists cannot be empty");
    for (auto& arg : sampleMethod.funcBody->paramLists[0]->params) {
        if (!FillMethodParamsByArg(params, methodCallArgs, sampleMethod, arg, jniEnvPtrParam)) {
            return nullptr;
        }
    }
    OwnedPtr<MemberAccess> methodAccess;
    if (sampleMethod.TestAttr(Attribute::STATIC)) {
        methodAccess = CreateMemberAccess(WithinFile(CreateRefExpr(decl), curFile), sampleMethod);
    } else {
        auto reg = lib.CreateGetFromRegistryCall(
            WithinFile(CreateRefExpr(jniEnvPtrParam), curFile), WithinFile(CreateRefExpr(selfParam), curFile), decl.ty);
        methodAccess = CreateMemberAccess(std::move(reg), sampleMethod);
    }
    methodAccess->curFile = curFile;

    auto methodCall = CreateCallExpr(std::move(methodAccess), std::move(methodCallArgs), Ptr(&sampleMethod), retTy,
        CallKind::CALL_DECLARED_FUNCTION);

    auto methodCallRes = CreateTmpVarDecl(nullptr, std::move(methodCall));
    methodCallRes->ty = retTy;
    OwnedPtr<Expr> retExpr;
    auto createCODEMappingCall = [&library = this->lib, &jniEnvPtrParam, &curFile, &methodCallRes, &retExpr](
                                   std::string& clazzName, bool needCtorArgs) {
        std::replace(clazzName.begin(), clazzName.end(), '.', '/');
        auto entity = library.CreateCFFINewJavaCFFINewJavaProxyObjectForCODEMappingCall(
            WithinFile(CreateRefExpr(jniEnvPtrParam), curFile), WithinFile(CreateRefExpr(*methodCallRes), curFile),
            clazzName, needCtorArgs);
        retExpr = WithinFile(std::move(entity), curFile);
    };
    if (retTy->IsPrimitive()) {
        retExpr = WithinFile(CreateRefExpr(*methodCallRes), curFile);
    } else if (IsCODEMapping(*retTy)) {
        if (auto retStructTy = DynamicCast<StructTy*>(retTy)) {
            std::string clazzName = retStructTy->decl->fullPackageName + "." + retTy->name;
            createCODEMappingCall(clazzName, true);
        } else if (auto retEnumTy = DynamicCast<EnumTy*>(retTy)) {
            std::string clazzName = retEnumTy->decl->fullPackageName + "." + retTy->name;
            createCODEMappingCall(clazzName, false);
        }
    } else {
        OwnedPtr<Expr> methodResRef = WithinFile(CreateRefExpr(*methodCallRes), curFile);
        auto entity = lib.WrapJavaEntity(std::move(methodResRef));
        retExpr = lib.UnwrapJavaEntity(std::move(entity), retTy, *(sampleMethod.outerDecl), true);
    }

    auto wrappedNodesLambda = WrapReturningLambdaExpr(typeManager, Nodes(std::move(methodCallRes), std::move(retExpr)));
    std::string funcName = GetJniMethodName(sampleMethod);
    std::vector<OwnedPtr<FuncParamList>> paramLists;
    paramLists.push_back(CreateFuncParamList(std::move(params)));

    return GenerateNativeFuncDeclBylambda(decl, wrappedNodesLambda, paramLists, jniEnvPtrParam, retTy, funcName);
}

void JavaDesugarManager::GenerateFuncParamsForNativeDeleteCodeObject(
    Decl& decl, std::vector<OwnedPtr<FuncParam>>& params, FuncParam*& jniEnv, OwnedPtr<Expr>& selfRef)
{
    PushEnvParams(params);
    PushObjParams(params);
    PushSelfParams(params);
    CODEC_ASSERT_WITH_MSG(!params.empty(), "jniEnv is absent");
    jniEnv = &(*params[0]);
    CODEC_NULLPTR_CHECK(decl.curFile);
    constexpr int SELF_REF_INDEX = 2;
    CODEC_ASSERT_WITH_MSG(params.size() > SELF_REF_INDEX, "selfRef is absent");
    selfRef = WithinFile(CreateRefExpr(*params[SELF_REF_INDEX]), decl.curFile);
}

OwnedPtr<Decl> JavaDesugarManager::GenerateNativeFuncDeclBylambda(Decl& decl, OwnedPtr<LambdaExpr>& wrappedNodesLambda,
    std::vector<OwnedPtr<FuncParamList>>& paramLists, FuncParam& jniEnvPtrParam, Ptr<Ty>& retTy, std::string funcName)
{
    CODEC_NULLPTR_CHECK(decl.curFile);
    auto catchingCall = lib.WrapExceptionHandling(
        WithinFile(CreateRefExpr(jniEnvPtrParam), decl.curFile), std::move(wrappedNodesLambda));
    //  For ty is CODEMapping:
    //  when ty is ArgsTy, we could use the Java_CFFI_getFromRegistry with [id: jlong] to get the codira side
    //  struct/class. when ty is RetTy, just use [jobjectTy] for we need JNI to construct the ret object.
    auto jniRetTy = IsCODEMapping(*retTy) ? lib.GetJobjectTy() : GetJNITy(retTy);
    auto block = CreateBlock(Nodes(std::move(catchingCall)), jniRetTy);
    auto funcBody = CreateFuncBody(std::move(paramLists), nullptr, std::move(block), jniRetTy);
    std::vector<Ptr<Ty>> funcTyParams;
    CODEC_ASSERT_WITH_MSG(!funcBody->paramLists.empty(), "paramLists cannot be empty");
    for (auto& param : funcBody->paramLists[0]->params) {
        funcTyParams.push_back(param->ty);
    }
    auto funcTy = typeManager.GetFunctionTy(funcTyParams, jniRetTy, {.isC = true});
    auto fdecl = CreateFuncDecl(funcName, std::move(funcBody), funcTy);
    fdecl->funcBody->funcDecl = fdecl.get();
    fdecl->EnableAttr(Attribute::C);
    fdecl->EnableAttr(Attribute::GLOBAL);
    fdecl->EnableAttr(Attribute::PUBLIC);
    fdecl->EnableAttr(Attribute::NO_MANGLE);
    fdecl->EnableAttr(Attribute::UNSAFE);
    fdecl->curFile = decl.curFile;
    fdecl->moduleName = decl.moduleName;
    fdecl->fullPackageName = decl.fullPackageName;

    return std::move(fdecl);
}

/**
 * when isClassLikeDecl is true: argument ctor: generated constructor mapped with Java_ClassName_initCODEObject func
 * when isClassLikeDecl is false: argument ctor: origin constructor mapped with Java_ClassName_initCODEObject func
 */
OwnedPtr<Decl> JavaDesugarManager::GenerateNativeInitCodeObjectFunc(FuncDecl& ctor, bool isClassLikeDecl)
{
    if (isClassLikeDecl) {
        CODEC_ASSERT_WITH_MSG(!ctor.funcBody->paramLists.empty(), "paramLists cannot be empty");
        CODEC_ASSERT(!ctor.funcBody->paramLists[0]->params.empty()); // it contains obj: JavaEntity as minimum
    }

    // func decl arguments construction
    std::vector<OwnedPtr<FuncParam>> params;
    std::vector<OwnedPtr<FuncArg>> ctorCallArgs;
    PushEnvParams(params);
    PushObjParams(params);
    auto curFile = ctor.curFile;
    CODEC_NULLPTR_CHECK(curFile);
    CODEC_ASSERT_WITH_MSG(params.size() > 1, "expected at least 2 params");
    auto& jniEnvPtrParam = *(params[0]);
    auto objParamRef = WithinFile(CreateRefExpr(*params[1]), curFile);
    if (isClassLikeDecl) {
        auto objAsEntity = lib.CreateJavaEntityJobjectCall(std::move(objParamRef));
        auto objWeakRef = lib.CreateNewGlobalRefCall(
            WithinFile(CreateRefExpr(jniEnvPtrParam), curFile), std::move(objAsEntity), true);
        ctorCallArgs.push_back(CreateFuncArg(std::move(objWeakRef)));
    }

    CODEC_ASSERT_WITH_MSG(!ctor.funcBody->paramLists.empty(), "paramLists cannot be empty");
    for (size_t argIdx = 0; argIdx < ctor.funcBody->paramLists[0]->params.size(); ++argIdx) {
        auto& arg = ctor.funcBody->paramLists[0]->params[argIdx];

        if (isClassLikeDecl && argIdx == 0) {
            continue;
        }

        if (!FillMethodParamsByArg(params, ctorCallArgs, ctor, arg, jniEnvPtrParam)) {
            return nullptr;
        }
    }

    std::vector<OwnedPtr<FuncParamList>> paramLists;
    paramLists.push_back(CreateFuncParamList(std::move(params)));

    auto ctorRef = WithinFile(CreateRefExpr(ctor), curFile);
    auto objectCtorCall = CreateCallExpr(
        std::move(ctorRef), std::move(ctorCallArgs), Ptr(&ctor), ctor.outerDecl->ty, CallKind::CALL_OBJECT_CREATION);

    auto putToRegistryCall = lib.CreatePutToRegistryCall(std::move(objectCtorCall));
    auto bodyLambda = WrapReturningLambdaExpr(typeManager, Nodes(std::move(putToRegistryCall)));
    auto jlongTy = lib.GetJlongTy();
    auto funcName = GetJniInitCodeObjectFuncName(ctor, isClassLikeDecl);
    return GenerateNativeFuncDeclBylambda(ctor, bodyLambda, paramLists, jniEnvPtrParam, jlongTy, funcName);
}

} // namespace Codira::Interop::Java
