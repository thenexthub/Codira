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
 * This file declares class for java code generation.
 */
#ifndef CODIRA_SEMA_JAVA_CODE_GENERATOR
#define CODIRA_SEMA_JAVA_CODE_GENERATOR

#include <set>

#include "AbstractSourceCodeGenerator.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Types.h"
#include "Codira/Mangle/BaseMangler.h"

namespace Codira::Interop::Java {
using namespace AST;

class JavaSourceCodeGenerator : public AbstractSourceCodeGenerator {
public:
    JavaSourceCodeGenerator(
        Decl* decl, const BaseMangler& mangler, const std::string& outputFilePath, std::string codeLibName);
    JavaSourceCodeGenerator(Decl* decl, const BaseMangler& mangler, const std::optional<std::string>& folderPath,
        const std::string& outputFileName, std::string codeLibName);
    static bool IsDeclAppropriateForGeneration(const Decl& declArg);

private:
    static const std::string DEFAULT_OUTPUT_DIR;
    static const std::string IGNORE_IMPORT;
    static std::string AddImport(Ptr<Ty> ty, std::set<std::string>* javaImports, const std::string* curPackageName);
    static std::string MapCODETypeToJavaType(const Ptr<Ty> ty, std::set<std::string>* javaImports,
        const std::string* curPackageName, bool isNativeMethod = false);
    static std::string MapCODETypeToJavaType(const OwnedPtr<Type>& type, std::set<std::string>* javaImports,
        const std::string* curPackageName, bool isNativeMethod = false);
    static std::string MapCODETypeToJavaType(const OwnedPtr<FuncParam>& param, std::set<std::string>* javaImports,
        const std::string* curPackageName, bool isNativeMethod = false);
    static std::string GenerateParams(const std::vector<OwnedPtr<FuncParam>>& params,
        const std::function<std::string(const OwnedPtr<FuncParam>& ptr)>& transform);
    static std::string GenerateParamLists(const std::vector<OwnedPtr<FuncParamList>>& paramLists,
        const std::function<std::string(const OwnedPtr<FuncParam>& ptr)>& transform);

    Decl* decl;
    std::set<std::string> imports;
    const std::string codeLibName;
    const BaseMangler& mangler;

    std::string GenerateFuncParams(const std::vector<OwnedPtr<FuncParam>>& params, bool isNativeMethod = false);
    std::string GenerateFuncParamLists(
        const std::vector<OwnedPtr<FuncParamList>>& paramLists, bool isNativeMethod = false);
    void ConstructResult() override;
    void AddClassDeclaration();
    void AddInterfaceDeclaration();
    void AddLoadLibrary();
    void AddSelfIdField();
    void AddProperties();
    std::string GenerateConstructorDecl(const FuncDecl& func, bool isForCodira);
    // Generate all constructors for each ctor in Enum.
    std::string GenerateConstructorForEnumDecl(const OwnedPtr<Decl>& ctor);
    // Generate super call argument and native declaration.
    std::pair<std::string, std::string> GenNativeSuperArgCall(
        const FuncArg& arg, const std::vector<OwnedPtr<FuncParam>>& params);
    // Generate super call and collection native func declaration.
    std::string GenerateSuperCall(
        const CallExpr& call, const std::vector<OwnedPtr<FuncParam>>& params, std::vector<std::string>& nativeArgs);
    std::string GenerateConstructorSuperCall(const FuncBody& body, std::vector<std::string>& nativeArgs);
    void AddConstructor(const FuncDecl& ctor, const std::string& superCall, bool isForCodira);
    // Generate constructors and native funcs.
    void AddConstructor(const FuncDecl& ctor);
    void AddConstructors();
    void AddAllCtorsForCODEMappingEnum(const EnumDecl& enumDecl);
    void AddInstanceMethod(const FuncDecl& funcDecl);
    void AddStaticMethod(const FuncDecl& funcDecl);
    void AddMethods();
    void AddInterfaceMethods();
    void AddEndClassParenthesis();
    void AddNativeInitCODEObject(const std::vector<OwnedPtr<Codira::AST::FuncParam>> &params);
    void AddNativeDeleteCODEObject();
    void AddFinalize();
    void AddHeader();
    void AddPrivateCtorForCODEMappring();
    void AddPrivateCtorForCODEMappringEnum();
    void AddEqualOrIdentityMethod(bool hasHascodeMethod, bool hasEqualsMethod, bool hasToStringMethod);
};
} // namespace Codira::Interop::Java

#endif // CODIRA_SEMA_JAVA_CODE_GENERATOR
