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
 * This file declares auxiliary methods for java interop implementation
 */
#ifndef CODIRA_SEMA_NATIVE_FFI_JAVA_AFTER_TYPE_CHECK_UTILS
#define CODIRA_SEMA_NATIVE_FFI_JAVA_AFTER_TYPE_CHECK_UTILS

#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Utils.h"
#include "../../../InheritanceChecker/StructInheritanceChecker.h"
#include "NativeFFI/Utils.h"

namespace Codira::Interop::Java {
using namespace AST;
using namespace Codira::Native::FFI;

enum class ArrayOperationKind: uint8_t {
    CREATE,
    GET,
    SET,
    GET_LENGTH
};

class Utils final {
public:
    Utils(ImportManager& importManager, TypeManager& typeManager);

    // Ty of `Option<ty>`
    Ptr<Ty> GetOptionTy(Ptr<Ty> ty);
    Ptr<EnumDecl> GetOptionDecl();

    // `Option<ty>.None`
    OwnedPtr<Expr> CreateOptionNoneRef(Ptr<Ty> ty);

    // `Option<ty>.Some(expr)`
    OwnedPtr<Expr> CreateOptionSomeCall(OwnedPtr<Expr> expr, Ptr<Ty> ty);

    // `Option<ty>.Some`
    OwnedPtr<Expr> CreateOptionSomeRef(Ptr<Ty> ty);

    // Decl of `java.lang.JObject`
    Ptr<ClassLikeDecl> GetJObjectDecl();

    // Decl of `java.lang.JString`
    Ptr<ClassLikeDecl> GetJStringDecl();

    // Decl of String
    StructDecl& GetStringDecl();

    std::string GetJavaClassNormalizeSignature(const Ty& codetype) const;
    std::string GetJavaTypeSignature(const Ty& codetype);
    std::string GetJavaTypeSignature(Ty& retTy, const std::vector<Ptr<Ty>>& args);
    std::string GetJavaObjectTypeName(const Ty& ty);

    OwnedPtr<Expr> CreateOptionMatch(
        OwnedPtr<Expr> selector,
        std::function<OwnedPtr<Expr>(VarDecl&)> someBranch,
        std::function<OwnedPtr<Expr>()> noneBranch,
        Ptr<Ty> ty);

private:
    Ptr<Decl> GetOptionSomeDecl();
    Ptr<Decl> GetOptionNoneDecl();

    Ptr<ClassLikeDecl> GetJavaLangDecl(const std::string& identifier);

private:
    ImportManager& importManager;
    TypeManager& typeManager;
};

/**
 * Returns javaref field of java mirror for the passed @JavaMirror/@JavaImpl declaration
 */
Ptr<VarDecl> GetJavaRefField(ClassLikeDecl& mirror);

/**
 * expr.javaRefGetter()
 */
OwnedPtr<Expr> CreateJavaRefCall(OwnedPtr<Expr> expr, FuncDecl& javaRefGetter);

/**
 * expr.javaref
 */
OwnedPtr<Expr> CreateJavaRefCall(OwnedPtr<Expr> expr, VarDecl& javaref);

/**
 * this.javaRefGetter(), where this is expr on mirrorLike
 */
OwnedPtr<Expr> CreateJavaRefCall(ClassLikeDecl& mirrorLike, FuncDecl& javaRefGetter, Ptr<File> curFile);

/**
 * this.javaref, where this is expr on mirrorLike
 */
OwnedPtr<Expr> CreateJavaRefCall(ClassLikeDecl& mirrorLike, VarDecl& javaref, Ptr<File> curFile);

/**
 * if mirrorLike is abstract class or interface, then:
 *   expr.getJavaRef()
 * else:
 *   expr.javaref
 */
OwnedPtr<Expr> CreateJavaRefCall(OwnedPtr<Expr> expr, ClassLikeDecl& mirrorLike);

/**
 * if mirrorLike is abstract class or interface, then:
 *   this.getJavaRef()
 * else:
 *   this.javaref,
 * where this is on mirrorLike
 */
OwnedPtr<Expr> CreateJavaRefCall(ClassLikeDecl& mirrorLike, Ptr<File> curFile);

/**
 * Acts like `CreateJavaRefCall(expr, *StaticCast<ClassLikeTy*>(expr->ty))->commonDecl`
 */
OwnedPtr<Expr> CreateJavaRefCall(OwnedPtr<Expr> expr);

/**
 * Is generated constructor of java mirror of kind: init(Java_CFFI_JavaEntity)
 */
bool IsGeneratedJavaMirrorConstructor(const FuncDecl& ctor);

bool IsGeneratedJavaImplConstructor(const FuncDecl& ctor);

/**
 * Recursively searches generated constructor of @JavaMirror in passed @JavaMirror/@JavaImpl `mirror`
 */
Ptr<FuncDecl> GetGeneratedJavaMirrorConstructor(ClassLikeDecl& mirror);

/**
 * Searches generated constructor of current @JavaMirror
 */
Ptr<FuncDecl> GetGeneratedConstructorInMirror(ClassDecl& mirror);

/**
 * Returns name of corresponding Java method or field with respect to @ForeignName annotation
 */
std::string GetJavaMemberName(const Decl& decl);

/**
 * Returns true if has java fully-qualified name defined in annotation as a string literal
 */
bool HasPredefinedJavaName(const ClassLikeDecl& decl);

/**
 * Returns fully-qualified name of the decl or fq-name specified in @JavaMirror as attribute,
 * which is suitable for specifying in JNI calls
 */
std::string GetJavaFQName(const Decl& decl);

/**
 * Returns fully-qualified name of the decl or fq-name specified in @JavaMirror as attribute,
 * which is suitable for using Java source code:
 * - For specifying nested class '.' is used
 */
std::string GetJavaFQSourceCodeName(const ClassLikeDecl& decl);


struct DestructedJavaClassName {
    /// Full package name
    std::optional<std::string> packageName;

    /// Name of top level class
    std::string topLevelClassName;

    /// Full name of the class, starting from top level
    std::string fullClassName;
};

/**
 * Returns parts of java class name
 */
DestructedJavaClassName DestructJavaClassName(const ClassLikeDecl& decl);

/**
 * Returns package of the decl or package specified in @JavaMirror as attribute (omitting class name)
 */
std::string GetJavaPackage(const Decl& decl);

/**
 * Mangles java name with JNI correspondence
 */
void MangleJNIName(std::string& name);

/**
 * Performs mangling of `javaTy` with `mangler`. If `javaTy` is a mirrror or impl, then it returns `jobjectTy`
 */
std::string GetMangledJniInitCodeObjectFuncName(const BaseMangler& mangler,
    const std::vector<OwnedPtr<FuncParam>>& params,
    bool isGeneratedCtor);

std::string GetMangledJniInitCodeObjectFuncNameForEnum(
    const BaseMangler& mangler, const std::vector<OwnedPtr<FuncParam>>& params, const std::string funcName);

const Ptr<ClassDecl> GetSyntheticClass(const ImportManager& importManager, const ClassLikeDecl& cld);

/**
 * Creates call of generated constructor (accepting java entity)
 * mirrorTy(entity)
 */
OwnedPtr<Expr> CreateMirrorConstructorCall(
    const ImportManager& importManager, OwnedPtr<Expr> entity, Ptr<Ty> mirrorTy);

bool IsJArray(const Decl& decl);
bool IsJArray(const Ty& ty);

bool IsMirror(const Ty& ty);

bool IsImpl(const Decl& decl);
bool IsImpl(const Ty& ty);
bool IsCODEMappingInterface(const Ty& ty);
bool IsCODEMapping(const Ty& ty);

ArrayOperationKind GetArrayOperationKind(Decl& decl);

template <typename Ret = Node, typename... Args>
std::vector<OwnedPtr<Ret>> Nodes(OwnedPtr<Args>&&... args)
{
    std::vector<OwnedPtr<Ret>> nodes;
    (nodes.push_back(std::forward<OwnedPtr<Args>>(args)), ...);
    return nodes;
}

namespace {

template <typename T>
void WrapArg(std::vector<OwnedPtr<FuncArg>>* funcArgs, OwnedPtr<T>&& e)
{
    CODEC_ASSERT(e);
    if (auto ptr = As<ASTKind::FUNC_ARG>(e.get())) {
        funcArgs->emplace_back(ptr);
    } else {
        funcArgs->push_back(CreateFuncArg(std::forward<OwnedPtr<T>>(e)));
    }
}

}

template <typename... Args>
OwnedPtr<CallExpr> CreateCall(Ptr<FuncDecl> fd, Ptr<File> curFile, OwnedPtr<Args>&&... args)
{
    if (!fd) {
        return nullptr;
    }

    std::vector<OwnedPtr<FuncArg>> funcArgs;

    (WrapArg(&funcArgs, std::forward<OwnedPtr<Args>>(args)), ...);

    auto funcTy = StaticCast<FuncTy*>(fd->ty);

    return CreateCallExpr(WithinFile(CreateRefExpr(*fd), curFile), std::move(funcArgs), fd, funcTy->retTy,
                          CallKind::CALL_DECLARED_FUNCTION);
}

Ptr<VarDecl> GetJavaRefField(ClassDecl& mirrorLike);

/**
 * for interfaces and abstract classes
 */
Ptr<FuncDecl> GetJavaRefGetter(ClassLikeDecl& mirror);

/**
 * For interface and abstract class mirror the synthetic class is generated to store $javaref field.
 */
bool IsSynthetic(const Node& node);

bool IsJavaRefGetter(const Decl& fd);

template<typename MemberDecl, ASTKind MemberKind>
Ptr<MemberDecl> FindFirstMemberDecl(
    const Decl& sourceDecl,
    const std::function<bool(const Decl&)>& memberPredicate
)
{
    for (auto& member : sourceDecl.GetMemberDecls()) {
        if (memberPredicate(*member)) {
            auto ptr = As<MemberKind>(member);
            CODEC_ASSERT(ptr);
            return Ptr(ptr);
        }
    }
    CODEC_ABORT();
    return nullptr;
}

void GenerateSyntheticClassMemberStubs(
    ClassDecl& synthetic,
    const MemberMap& interfaceMembers,
    const MemberMap& instanceMembers);


/**
 * Returns FQ name of marker class for Codira side constructor of Java class
 */
std::string GetConstructorMarkerFQName();
std::string GetConstructorMarkerClassName();
OwnedPtr<ClassDecl> CreateConstructorMarkerClassDecl();

} // namespace Codira::Interop::Java

#endif // CODIRA_SEMA_NATIVE_FFI_JAVA_AFTER_TYPE_CHECK_UTILS
