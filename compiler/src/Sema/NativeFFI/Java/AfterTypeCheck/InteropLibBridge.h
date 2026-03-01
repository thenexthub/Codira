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

#ifndef CODIRA_SEMA_NATIVE_FFI_JAVA_INTEROPLIB_BRIDGE
#define CODIRA_SEMA_NATIVE_FFI_JAVA_INTEROPLIB_BRIDGE

#include "Utils.h"
#include "NativeFFI/Utils.h"

#include "Codira/Modules/ImportManager.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/AST/Match.h"

namespace Codira::Interop::Java {
using namespace AST;


/**
 * method or field jni signature
 */
struct MemberJNISignature {
    std::string classTypeSignature;
    std::string name;
    std::string signature;

    MemberJNISignature(std::string classTypeSignature, std::string name, std::string signature)
        : classTypeSignature(classTypeSignature), name(name), signature(signature) {}

    MemberJNISignature(Utils& utils, FuncDecl& member)
        : MemberJNISignature(utils, member, StaticAs<ASTKind::CLASS_LIKE_DECL>(member.outerDecl))
    {
            auto& retTy = *member.funcBody->retType->ty;
            std::vector<Ptr<Ty>> paramTys = Native::FFI::GetParamTys(*member.funcBody->paramLists[0]);
            signature = utils.GetJavaTypeSignature(retTy, paramTys);
    }

    MemberJNISignature(Utils& utils, PropDecl& member)
        : MemberJNISignature(utils, member, StaticAs<ASTKind::CLASS_LIKE_DECL>(member.outerDecl))
    {
            signature = utils.GetJavaTypeSignature(*member.ty);
    }

    MemberJNISignature(Utils& utils, Decl& member, Ptr<ClassLikeDecl> jobject)
    {
        CODEC_ASSERT(jobject);
        Ptr<Ty> ty = jobject->ty;

        if (IsSynthetic(*jobject)) {
            if (jobject->inheritedTypes.size() > 1) {
                ty = jobject->inheritedTypes[1]->ty; // take interface ty
            } else {
                CODEC_ASSERT_WITH_MSG(!jobject->inheritedTypes.empty(), "JObject must inherit Codira Object");
                ty = jobject->inheritedTypes[0]->ty; // take superclass ty
            }
        }
        classTypeSignature = utils.GetJavaClassNormalizeSignature(*ty);
        name = GetJavaMemberName(member);
        CODEC_ASSERT(member.astKind == ASTKind::FUNC_DECL || member.astKind == ASTKind::PROP_DECL);
        signature = utils.GetJavaTypeSignature(*member.ty);
    }
};

class InteropLibBridge {
public:
    InteropLibBridge(
        ImportManager& importManager, TypeManager& typeManager, DiagnosticEngine& diag, Utils& utils)
        : importManager(importManager), typeManager(typeManager), diag(diag), utils(utils) { }

    /**
     * jobject
     */
    Ptr<TypeAliasDecl> GetJobjectDecl();

    /**
     * Java_CFFI_JavaEntity
     */
    Ptr<StructDecl> GetJavaEntityDecl();

    /**
     * Java_CFFI_JavaEntityKind
     */
    Ptr<EnumDecl> GetJavaEntityKindDecl();

    /**
     * Java_CFFI_JavaEntityKind.JOBJECT
     */
    Decl& GetJavaEntityKindJObject();

    /**
     * Java_CFFI_newGlobalReference
     */
    Ptr<FuncDecl> GetNewGlobalRefDecl();

    /**
     * Java_CFFI_deleteGlobalReference
     */
    Ptr<FuncDecl> GetDeleteGlobalRefDecl();

    /**
     * JNIEnv_ptr
     */
    Ptr<TypeAliasDecl> GetJniEnvPtrDecl();

    /**
     * Java_CFFI_get_env
     */
    Ptr<FuncDecl> GetGetJniEnvDecl();

    /**
     * Java_CFFI_JavaEntityJobject
     */
    Ptr<FuncDecl> GetCreateJavaEntityJobjectDecl();

    /**
     * Java_CFFI_JavaEntityJobjectNull
     */
    Ptr<FuncDecl> GetCreateJavaEntityNullDecl();

    /**
     * Java_CFFI_newJavaObject
     */
    Ptr<FuncDecl> GetNewJavaObjectDecl();

    /**
     * Java_CFFI_newJavaArray
     */
    Ptr<FuncDecl> GetNewJavaArrayDecl();

    /**
     * Java_CFFI_newJavaProxyObjectForCODEMapping
     */
    Ptr<FuncDecl> GetNewJavaProxyObjectForCODEMappingDecl();

    /**
     * Java_CFFI_arrayGet
     */
    Ptr<FuncDecl> GetJavaArrayGetDecl();

    /**
     * Java_CFFI_arraySet
     */
    Ptr<FuncDecl> GetJavaArraySetDecl();

    /**
     * Java_CFFI_arrayGetLength
     */
    Ptr<FuncDecl> GetJavaArrayGetLengthDecl();

    /**
     * Java_CFFI_callVirtualMethod_raw
     */
    Ptr<FuncDecl> GetCallMethodDecl();

    /**
     * Java_CFFI_callNonVirtualMethod_raw
     */
    Ptr<FuncDecl> GetNonVirtualCallMethodDecl();

    /**
     * Java_CFFI_callStaticMethod_raw
     */
    Ptr<FuncDecl> GetCallStaticMethodDecl();

    /**
     * Java_CFFI_deleteCODEObject
     */
    Ptr<FuncDecl> GetDeleteCODEObjectDecl();

    /**
     * Java_CFFI_removeFromRegistry
     */
    Ptr<FuncDecl> GetRemoveFromRegistryDecl();

    /**
     * Java_CFFI_put_to_registry_1
     */
    Ptr<FuncDecl> GetPutToRegistryDecl();

    /**
     * Java_CFFI_put_to_registry
     */
    Ptr<FuncDecl> GetPutToRegistrySelfInitDecl();

    /**
     * Java_CFFI_unwrapJavaEntityAsValue
     */
    Ptr<FuncDecl> GetUnwrapJavaEntityDecl();

    /**
     * Java_CFFI_unwrapJavaMirror
     */
    Ptr<FuncDecl> GetUnwrapJavaMirrorDecl();

    /**
     * Java_CFFI_getFromRegistryByEntityOption<T>
     */
    Ptr<FuncDecl> GetGetFromRegistryByEntityOptionDecl();

    /**
     * Java_CFFI_getFromRegistryByEntity<T>
     */
    Ptr<FuncDecl> GetGetFromRegistryByEntityDecl();

    /**
     * Java_CFFI_JavaStringToCodira
     */
    Ptr<FuncDecl> GetJavaStringToCodira();

    /**
     * Java_CFFI_CodiraStringToJava
     */
    Ptr<FuncDecl> GetCodiraStringToJava();

    /**
     * Java_CFFI_getFromRegistry<T>
     */
    Ptr<FuncDecl> GetGetFromRegistryDecl();

    /**
     * Java_CFFI_getFromRegistryOption<T>
     */
    Ptr<FuncDecl> GetGetFromRegistryOptionDecl();

    /**
     * Java_CFFI_getField_raw
     */
    Ptr<FuncDecl> GetGetFieldDecl();

    /**
     * Java_CFFI_setField_raw
     */
    Ptr<FuncDecl> GetSetFieldDecl();

    /**
     * Java_CFFI_getStaticField_raw
     */
    Ptr<FuncDecl> GetGetStaticFieldDecl();

    /**
     * Java_CFFI_setStaticField_raw
     */
    Ptr<FuncDecl> GetSetStaticFieldDecl();

    /**
     * INTEROPLIB_VERSION
     */
    Ptr<VarDecl> GetInteropLibVersionVarDecl();

    /**
     * Java_CFFI_ensure_not_null
     */
    Ptr<FuncDecl> GetEnsureNotNullDecl();

    /**
     * Java_CFFI_getOrNull
     */
    Ptr<FuncDecl> GetGetJavaEntityOrNullDecl();

    /**
     * Java_CFFI_isInstanceOf
     */
    Ptr<FuncDecl> GetIsInstanceOf();

    /**
     * withExceptionHandling
     */
    Ptr<FuncDecl> GetWithExceptionHandlingDecl();

    Ptr<FuncDecl> GetJClassDecl();
    Ptr<FuncDecl> GetParseMethodSignatureDecl();
    Ptr<FuncDecl> GetParseComponentSignatureDecl();
    Ptr<FuncDecl> GetCallNestDecl();
    Ptr<FuncDecl> GetMethodIdConstr();
    Ptr<FuncDecl> GetMethodIdConstrStatic();
    Ptr<FuncDecl> GetFieldIdConstr();
    Ptr<FuncDecl> GetFieldIdConstrStatic();

    /**
     * JNIEnv_ptr ty
     */
    Ptr<Ty> GetJNIEnvPtrTy();

    /**
     * Java_CFFI_JavaEntity ty
     */
    Ptr<Ty> GetJavaEntityTy();

    /**
     * jobject ty
     */
    Ptr<Ty> GetJobjectTy();

    /**
     * jlong
     */
    Ptr<Ty> GetJlongTy();

    /**
     * jobject type
     */
    OwnedPtr<Type> CreateJobjectType();

    /**
     * jlong
     */
    OwnedPtr<Type> CreateJlongType();

    /**
     * Returns codeExpr wrapped into java entity:
     *
     * Java_CFFI_JavaEntity(codeExpr)
     */
    OwnedPtr<Expr> WrapJavaEntity(OwnedPtr<Expr> codeExpr);

    /**
     * interoplib.interop.Java_CFFI_JavaEntityJobject(jobject: CPointer<Unit>)
     */
    OwnedPtr<Expr> CreateJavaEntityJobjectCall(OwnedPtr<Expr> arg);

    /**
     * Java_CFFI_JavaEntityJobjectNull()
     */
    OwnedPtr<Expr> CreateJavaEntityNullCall(Ptr<File> curFile);

    /**
     * match (arg) {
     *     case Some(argv) => argv.javaref
     *     case None => Java_CFFI_JavaEntityJobjectNull()
     * }
     */
    OwnedPtr<Expr> CreateJavaEntityFromOptionMirror(OwnedPtr<Expr> option, ClassLikeDecl& mirror);

    /**
     * Java_CFFI_JavaEntity() // Unit
     */
    OwnedPtr<CallExpr> CreateJavaEntityCall(Ptr<File> file);

    /**
     * Java_CFFI_JavaEntity(arg)
     */
    OwnedPtr<Expr> CreateJavaEntityCall(OwnedPtr<Expr> arg);

    /**
     * Java_CFFI_get_env()
     */
    OwnedPtr<CallExpr> CreateGetJniEnvCall(Ptr<File> curFile);

    /** CFFI object creation call:
     * Java_CFFI_newJavaObject(jniEnv, classTypeSignature, "(<argsSignature>)V", cffiCtorFuncArgs)
     */
    OwnedPtr<CallExpr> CreateCFFINewJavaObjectCall(OwnedPtr<Expr> jniEnv, std::string classTypeSignature,
                                                   FuncParamList& params, bool isMirror, File& curFile);

    /**
     * Java_CFFI_newJavaObject(env, classTypeSignature, constructorSignature, [args])
     */
    OwnedPtr<CallExpr> CreateNewJavaObjectCall(
        OwnedPtr<Expr> env,
        const std::string& classTypeSignature,
        const std::string& constructorSignature,
        std::vector<OwnedPtr<Expr>> args);

    /**
     * Java_CFFI_newJavaArray(env, signature, [args])
     */
    OwnedPtr<CallExpr> CreateCFFINewJavaArrayCall(
        OwnedPtr<Expr> jniEnv, FuncParamList& params, const Ptr<GenericParamDecl> genericParam);

    /**
     * Java_CFFI_newJavaProxyObjectForCODEMapping(env, entity, name, withMarkerParam)
     * For StrcutTy, withMarkerParam is true; for EnumTy, withMarkerParam is false.
     */
    OwnedPtr<CallExpr> CreateCFFINewJavaCFFINewJavaProxyObjectForCODEMappingCall(
        OwnedPtr<Expr> jniEnv, OwnedPtr<Expr> entity, std::string name, bool withMarkerParam);

    /**
     * Java_CFFI_newGlobalReference(env, obj, isWeak)
     */
    OwnedPtr<CallExpr> CreateNewGlobalRefCall(OwnedPtr<Expr> env, OwnedPtr<Expr> obj, bool isWeak);

    /**
     * Java_CFFI_deleteGlobalReference(env, obj)
     */
    OwnedPtr<CallExpr> CreateDeleteGlobalRefCall(OwnedPtr<Expr> env, OwnedPtr<Expr> obj);

    /**
     * CFFI method call:
     * Java_CFFI_callVirtualMethod_raw(jniEnv, obj, typeSignature, methodName, "(<argsSignature>)ReTy", cffiMethodArgs)
     */
    OwnedPtr<CallExpr> CreateCFFICallMethodCall(OwnedPtr<Expr> jniEnv, OwnedPtr<Expr> obj,
                                                const MemberJNISignature& signature,
                                                FuncParamList& params, File& curFile);

    /**
     * Java_CFFI_arrayGetLength(env)
     */
    OwnedPtr<CallExpr> CreateCFFIArrayLengthGetCall(OwnedPtr<Expr> javarefExpr, Ptr<File> curFile);

    /**
     * CFFI method call:
     * Java_CFFI_arrayGet(jniEnv, obj, typeSignature, cffiMethodArgs)
     * Java_CFFI_arraySet(jniEnv, obj, typeSignature, cffiMethodArgs)
     */
    OwnedPtr<AST::Expr> CreateCFFICallArrayMethodCall(OwnedPtr<AST::Expr> jniEnv, OwnedPtr<AST::Expr> obj,
        AST::FuncParamList& params, const Ptr<AST::GenericParamDecl> genericParam, ArrayOperationKind kind);

    /**
     * CFFI static method call:
     * Java_CFFI_callStaticMethod_raw(
     *     jniEnv, signature.classTypeSignature, signature.name, "(<argsSignature>)ReTy", cffiMethodArgs)
     */
    OwnedPtr<CallExpr> CreateCFFICallStaticMethodCall(
        OwnedPtr<Expr> jniEnv, const MemberJNISignature& signature, FuncParamList& params, File& curFile);

    /**
     * <callMethod>(env, obj, signature.classTypeSignature, signature.name, signature.signature, [args])
     * where <callMethod> is Java_CFFI_callVirtualMethod_raw or Java_CFFI_callNonVirtualMethod_raw
     *
     * @param virt should the call be virtual or not
     */
    OwnedPtr<CallExpr> CreateCallMethodCall(OwnedPtr<Expr> env, OwnedPtr<Expr> obj,
                                            const MemberJNISignature& signature,
                                            std::vector<OwnedPtr<Expr>> args, File& curFile,
                                            bool virt = true);

    /**
     * Java_CFFI_callStaticMethod_raw(env, signature.classTypeSignature, signature.name, signature.signature, [args])
     */
    OwnedPtr<CallExpr> CreateCallStaticMethodCall(OwnedPtr<Expr> env, const MemberJNISignature& signature,
                                                  std::vector<OwnedPtr<Expr>> args, File& curFile);

    /**
     * Java_CFFI_deleteCODEObject(env, obj, self, getWeakRef)
     */
    OwnedPtr<CallExpr> CreateDeleteCODEObjectCall(OwnedPtr<Expr> env, OwnedPtr<Expr> self,
                                                OwnedPtr<Expr> getWeakRef, Ptr<Ty> codeTy /* generic param ty */);

    /**
     * Java_CFFI_removeFromRegistry(self)
     */
    OwnedPtr<CallExpr> CreateRemoveFromRegistryCall(OwnedPtr<Expr> self);

    /**
     * Java_CFFI_put_to_registry_1(obj)
     */
    OwnedPtr<CallExpr> CreatePutToRegistryCall(OwnedPtr<Expr> obj);

    OwnedPtr<CallExpr> CreatePutToRegistrySelfInitCall(OwnedPtr<Expr> env, OwnedPtr<Expr> entity, OwnedPtr<Expr> obj);

    /**
     * Java_CFFI_getFromRegistryByObj{Option}<ty>(env, obj)
     */
    OwnedPtr<CallExpr> CreateGetFromRegistryByEntityCall(OwnedPtr<Expr> env, OwnedPtr<Expr> obj, Ptr<Ty> ty,
                                                      bool retAsOption);

    /**
     * Java_CFFI_JavaStringToCodira(env, jstring)
     */
    OwnedPtr<CallExpr> CreateJavaStringToCodiraCall(OwnedPtr<Expr> env, OwnedPtr<Expr> jstring);

    /**
     * Java_CFFI_getFromRegistry<ty>(env, self)
     */
    OwnedPtr<CallExpr> CreateGetFromRegistryCall(OwnedPtr<Expr> env, OwnedPtr<Expr> self, Ptr<Ty> ty);

    /**
     * Java_CFFI_getFromRegistryOption<ty>(self)
     */
    OwnedPtr<CallExpr> CreateGetFromRegistryOptionCall(OwnedPtr<Expr> self, Ptr<Ty> ty);

    /**
     * Java_CFFI_getField_raw(env, obj, typeSignature, fieldName, fieldSignature)
     */
    OwnedPtr<CallExpr> CreateGetFieldCall(OwnedPtr<Expr> env, OwnedPtr<Expr> obj, std::string typeSignature,
                                          std::string fieldName, std::string fieldSignature);

    /**
     * Java_CFFI_getStaticField_raw(env, typeSignature, fieldName, fieldSignature)
     */
    OwnedPtr<CallExpr> CreateGetStaticFieldCall(OwnedPtr<Expr> env, std::string typeSignature,
                                          std::string fieldName, std::string fieldSignature);

    /**
     * Java_CFFI_setField_raw(env, obj, signature.classTypeSignature,
     *                        signature.name, signature.signature, Java_CFFI_JavaEntity(value))
     */
    OwnedPtr<CallExpr> CreateSetFieldCall(OwnedPtr<Expr> env, OwnedPtr<Expr> obj,
                                          const MemberJNISignature& signature,
                                          OwnedPtr<Expr> value);

    /**
     * Java_CFFI_setStaticField_raw(env, typeSignature, fieldName, fieldSignature, Java_CFFI_JavaEntity(value))
     */
    OwnedPtr<CallExpr> CreateSetStaticFieldCall(OwnedPtr<Expr> env, std::string typeSignature, std::string fieldName,
                                          std::string fieldSignature, OwnedPtr<Expr> value);

    /**
     * Java_CFFI_ensure_not_null(entity)
     */
    OwnedPtr<Expr> CreateEnsureNotNullCall(OwnedPtr<Expr> entity);

    /**
     * Java_CFFI_getJavaEntityOrNull(entity)
     */
    OwnedPtr<Expr> CreateGetJavaEntityOrNullCall(OwnedPtr<Expr> entity);

    OwnedPtr<Expr> WrapExceptionHandling(OwnedPtr<Expr> env, OwnedPtr<LambdaExpr> action);

    /**
     * CPointer<Unit>()
     */
    OwnedPtr<PointerExpr> CreateJobjectNull();

    /**
     * env: JNIEnv_Ptr
     */
    OwnedPtr<FuncParam> CreateEnvFuncParam();

    /**
     * _: jobject or jclass, default name is '_'
     */
    OwnedPtr<FuncParam> CreateJClassOrJObjectFuncParam(const std::string& name = "_");

    /**
     * self: jlong
     */
    OwnedPtr<FuncParam> CreateSelfFuncParam();

    /**
     * // entityOption: Java_CFFI_JavaEntity
     *   match (entityOption.isNull) {
     *       case true => None<ty>
     *       case false => Some<ty>(ty(entityOption))
     *   }
     * If [toRaw] = `true`, then it returns java reference as CPointer<Unit> instead of an instance
     */
    OwnedPtr<Expr> UnwrapJavaMirrorOption(
        OwnedPtr<Expr> entityOption, Ptr<Ty> ty, const ClassLikeDecl& mirror, bool toRaw = false);

    OwnedPtr<Expr> UnwrapJavaImplOption(OwnedPtr<Expr> env, OwnedPtr<Expr> entityOption, Ptr<Ty> ty,
        const ClassLikeDecl& mirror, bool toRaw = false);

    /**
     * Creates unwrap call Java_CFFI_JavaEntity [entity] and returns the value it stores as [ty] if [toRaw] is `false`:
     * Java_CFFI_unwrapJavaEntityAsValue<ty>(entity)
     *
     * If ty of decl T is @JavaMirror:
     * T(entity)
     *
     * If ty of decl T is @JavaImpl:
     * Java_CFFI_getFromRegistry<T>(entity).
     * Else, if [toRaw] is `true` and T is @JavaMirror or @JavaImpl class, then it returns its CPointer<Unit> java ref
     */
    OwnedPtr<Expr> UnwrapJavaEntity(OwnedPtr<Expr> entity, Ptr<Ty> ty, const Decl& outerDecl, bool toRaw = false);

    OwnedPtr<AST::MatchExpr> CreateMatchByTypeArgument(
        const Ptr<AST::GenericParamDecl> genericParam,
        std::map<std::string, OwnedPtr<Expr>> typeToCaseMap, Ptr<Ty> retTy, OwnedPtr<Expr> defaultCase);
    OwnedPtr<AST::MatchExpr> CreateMatchWithTypeCast(OwnedPtr<Expr> exprToCast, Ptr<Ty> castTy);
    OwnedPtr<Expr> CreateGetTypeForTypeParameterCall(const Ptr<GenericParamDecl> genericParam) const;

    Ptr<FuncDecl> FindGetTypeForTypeParamDecl(File& file) const;
    Ptr<FuncDecl> FindCStringToStringDecl();
    Ptr<FuncDecl> FindStringEqualsDecl();
    Ptr<FuncDecl> FindStringStartsWithDecl();
    Ptr<FuncDecl> FindArrayJavaEntityGetDecl(ClassDecl& jArrayDecl) const;
    Ptr<FuncDecl> FindArrayJavaEntitySetDecl(ClassDecl& jArrayDecl) const;

    OwnedPtr<Expr> SelectJSigByTypeKind(TypeKind kind, Ptr<Ty> ty);
    OwnedPtr<Expr> SelectJPrimitiveNameByTypeKind(TypeKind kind, Ptr<Ty> ty);
    OwnedPtr<Expr> SelectEntityWrapperByTypeKind(TypeKind kind, Ptr<Ty> ty, Ptr<FuncParam> param, Ptr<File> file);
    OwnedPtr<Expr> SelectEntityUnwrapperByTypeKind(
        TypeKind kind, Ptr<Ty> ty, Ptr<Expr> entity, const ClassLikeDecl& mirror);
    std::map<std::string, OwnedPtr<Expr>> GenerateTypeMappingWithSelector(
        std::function<OwnedPtr<Expr>(TypeKind, Ptr<Ty>)> selector
    );

    void CheckInteropLibVersion();

private:
    static constexpr auto INTEROPLIB_VERSION = 9;
    static constexpr auto INTEROPLIB_PACKAGE_NAME = "interoplib.interop";

    const std::vector<TypeKind> supportedArrayPrimitiveElementType = {
        TypeKind::TYPE_BOOLEAN,
        TypeKind::TYPE_INT8, TypeKind::TYPE_UINT16, TypeKind::TYPE_INT16, TypeKind::TYPE_INT32, TypeKind::TYPE_INT64,
        TypeKind::TYPE_FLOAT32, TypeKind::TYPE_FLOAT64
    };

    OwnedPtr<Expr> CreateMirrorContructorCall(
        Ptr<ClassLikeDecl> mirror, OwnedPtr<Expr> javaEntity, Ptr<Ty> expectedTy) const;

    OwnedPtr<Expr> UnwrapJavaArrayEntity(OwnedPtr<Expr> entity, Ptr<Ty> ty, const ClassLikeDecl& mirror);
    OwnedPtr<Expr> UnwrapJavaPrimitiveEntity(OwnedPtr<Expr> entity, Ptr<Ty> ty);

    template <ASTKind K = ASTKind::DECL>
    inline auto ImportDecl(const std::string& package, const std::string& declname, bool silent = false)
    {
        auto decl = importManager.GetImportedDecl(package, declname);
        if (!decl) {
            if (!silent) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_member_not_imported,
                                      DEFAULT_POSITION, package + "." + declname);
            }
            return Ptr(As<K>(nullptr));
        }

        CODEC_ASSERT(decl && decl->astKind == K);
        return Ptr(StaticAs<K>(decl));
    }

    template <ASTKind K = ASTKind::DECL>
    inline auto GetInteropLibDecl(const std::string& declname, bool silent = false)
    {
        return ImportDecl<K>(INTEROPLIB_PACKAGE_NAME, declname, silent);
    }

    template <ASTKind K = ASTKind::DECL>
    inline auto GetJavaLangDecl(const std::string& declname)
    {
        return ImportDecl<K>(INTEROP_JAVA_LANG_PACKAGE, declname);
    }

    ImportManager& importManager;
    TypeManager& typeManager;
    DiagnosticEngine& diag;
    Utils& utils;
};
}

#endif // CODIRA_SEMA_NATIVE_FFI_JAVA_INTEROPLIB_BRIDGE
