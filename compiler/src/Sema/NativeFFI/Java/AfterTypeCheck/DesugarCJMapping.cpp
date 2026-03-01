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

// Support Struct decl and Enum decl for now.
OwnedPtr<Decl> JavaDesugarManager::GenerateCODEMappingNativeDeleteCodeObjectFunc(Decl& decl)
{
    std::vector<OwnedPtr<FuncParam>> params;
    FuncParam* jniEnvPtrParam = nullptr;
    OwnedPtr<Expr> selfParamRef;
    GenerateFuncParamsForNativeDeleteCodeObject(decl, params, jniEnvPtrParam, selfParamRef);
    CODEC_NULLPTR_CHECK(jniEnvPtrParam);

    auto removeFromRegistryCall = lib.CreateRemoveFromRegistryCall(std::move(selfParamRef));
    auto wrappedNodesLambda = WrapReturningLambdaExpr(typeManager, Nodes(std::move(removeFromRegistryCall)));
    Ptr<Ty> unitTy = typeManager.GetPrimitiveTy(TypeKind::TYPE_UNIT).get();
    auto funcName = GetJniDeleteCodeObjectFuncName(decl);
    std::vector<OwnedPtr<FuncParamList>> paramLists;
    paramLists.push_back(CreateFuncParamList(std::move(params)));

    return GenerateNativeFuncDeclBylambda(decl, wrappedNodesLambda, paramLists, *jniEnvPtrParam, unitTy, funcName);
}

void JavaDesugarManager::GenerateForCODEStructMapping(AST::StructDecl* structDecl)
{
    CODEC_ASSERT(structDecl && IsCODEMapping(*structDecl));
    std::vector<FuncDecl*> generatedCtors;
    for (auto& member : structDecl->GetMemberDecls()) {
        if (member->TestAnyAttr(Attribute::IS_BROKEN, Attribute::PRIVATE, Attribute::PROTECTED, Attribute::INTERNAL)) {
            continue;
        }
        if (auto fd = As<ASTKind::FUNC_DECL>(member.get())) {
            if (fd->TestAttr(Attribute::CONSTRUCTOR)) {
                generatedCtors.push_back(fd);
            } else {
                auto nativeMethod = GenerateNativeMethod(*fd, *structDecl);
                if (nativeMethod != nullptr) {
                    generatedDecls.push_back(std::move(nativeMethod));
                }
            }
        }
    }
    if (!generatedCtors.empty()) {
        generatedDecls.push_back(GenerateCODEMappingNativeDeleteCodeObjectFunc(*structDecl));
        for (auto generatedCtor : generatedCtors) {
            generatedDecls.push_back(GenerateNativeInitCodeObjectFunc(*generatedCtor, false));
        }
    }
}

OwnedPtr<Decl> JavaDesugarManager::GenerateNativeInitCodeObjectFuncForEnumCtorNoParams(
    AST::EnumDecl& enumDecl, AST::VarDecl& ctor)
{
    // Empty params to build constructor from VarDecl.
    std::vector<OwnedPtr<FuncParam>> params;
    std::vector<OwnedPtr<FuncArg>> ctorCallArgs;
    PushEnvParams(params, "env");
    auto curFile = ctor.curFile;
    CODEC_NULLPTR_CHECK(curFile);
    CODEC_ASSERT_WITH_MSG(!params.empty(), "jniEnvPtrParam is absent");
    auto& jniEnvPtrParam = *(params[0]);

    std::vector<OwnedPtr<FuncParamList>> paramLists;
    paramLists.push_back(CreateFuncParamList(std::move(params)));

    auto enumRef = WithinFile(CreateRefExpr(enumDecl), curFile);
    auto objectCtorCall = CreateMemberAccess(std::move(enumRef), ctor.identifier);

    auto putToRegistryCall = lib.CreatePutToRegistryCall(std::move(objectCtorCall));
    auto bodyLambda = WrapReturningLambdaExpr(typeManager, Nodes(std::move(putToRegistryCall)));
    auto jlongTy = lib.GetJlongTy();
    auto funcName = GetJniInitCodeObjectFuncNameForVarDecl(ctor);
    return GenerateNativeFuncDeclBylambda(ctor, bodyLambda, paramLists, jniEnvPtrParam, jlongTy, funcName);
}

void JavaDesugarManager::GenerateNativeInitCODEObjectEnumCtor(AST::EnumDecl& enumDecl)
{
    auto nativeMethod = MakeOwned<Decl>();
    for (auto& ctor : enumDecl.constructors) {
        if (ctor->astKind == ASTKind::FUNC_DECL) {
            auto fd = As<ASTKind::FUNC_DECL>(ctor.get());
            CODEC_NULLPTR_CHECK(fd);
            nativeMethod = GenerateNativeInitCodeObjectFunc(*fd, false);
        } else if (ctor->astKind == ASTKind::VAR_DECL) {
            auto varDecl = As<ASTKind::VAR_DECL>(ctor.get());
            CODEC_NULLPTR_CHECK(varDecl);
            nativeMethod = GenerateNativeInitCodeObjectFuncForEnumCtorNoParams(enumDecl, *varDecl);
        }
        generatedDecls.push_back(std::move(nativeMethod));
    }
}

void JavaDesugarManager::GenerateForCODEEnumMapping(AST::EnumDecl& enumDecl)
{
    CODEC_ASSERT(IsCODEMapping(enumDecl));

    GenerateNativeInitCODEObjectEnumCtor(enumDecl);

    for (auto& member : enumDecl.GetMemberDecls()) {
        if (member->TestAttr(Attribute::IS_BROKEN) || !member->TestAttr(Attribute::PUBLIC)) {
            continue;
        }
        if (auto fd = As<ASTKind::FUNC_DECL>(member.get())) {
            generatedDecls.push_back(GenerateNativeMethod(*fd, enumDecl));
        } else if (member->astKind == ASTKind::PROP_DECL && !member->TestAttr(Attribute::COMPILER_ADD)) {
            const PropDecl& propDecl = *StaticAs<ASTKind::PROP_DECL>(member.get());
            CODEC_ASSERT_WITH_MSG(!propDecl.getters.empty(), "property must have a getter");
            const OwnedPtr<FuncDecl>& funcDecl = propDecl.getters[0];
            auto getSignature = GetJniMethodNameForProp(propDecl, false);
            auto nativeMethod = GenerateNativeMethod(*funcDecl.get(), enumDecl);
            if (nativeMethod != nullptr) {
                nativeMethod->identifier = getSignature;
                generatedDecls.push_back(std::move(nativeMethod));
            }
        }
    }

    generatedDecls.push_back(GenerateCODEMappingNativeDeleteCodeObjectFunc(enumDecl));
}

void JavaDesugarManager::GenerateInCODEMapping(File& file)
{
    for (auto& decl : file.decls) {
        if (!decl.get()->TestAttr(Attribute::PUBLIC) || decl.get()->TestAttr(Attribute::IS_BROKEN)) {
            continue;
        }
        auto astDecl = As<ASTKind::DECL>(decl.get());
        if (astDecl && astDecl->TestAttr(Attribute::IS_BROKEN)) {
            continue;
        }
        auto structDecl = As<ASTKind::STRUCT_DECL>(decl.get());
        if (structDecl && IsCODEMapping(*structDecl)) {
            GenerateForCODEStructMapping(structDecl);
        }
        auto enumDecl = As<ASTKind::ENUM_DECL>(decl.get());
        if (enumDecl && IsCODEMapping(*enumDecl)) {
            GenerateForCODEEnumMapping(*enumDecl);
        }
    }
}

void JavaDesugarManager::DesugarInCODEMapping(File& file)
{
    for (auto& decl : file.decls) {
        if (!decl.get()->TestAttr(Attribute::PUBLIC) || decl.get()->TestAttr(Attribute::IS_BROKEN) ||
            !JavaSourceCodeGenerator::IsDeclAppropriateForGeneration(*decl.get()) || !IsCODEMapping(*decl.get())) {
            continue;
        }

        const std::string fileJ = decl.get()->identifier.Val() + ".java";
        auto codegen = JavaSourceCodeGenerator(decl.get(), mangler, javaCodeGenPath, fileJ,
            GetCodiraLibName(outputLibPath, decl.get()->GetFullPackageName()));
        codegen.Generate();
    }
}

} // namespace Codira::Interop::Java
