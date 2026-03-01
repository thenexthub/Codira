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
 * This file declares class for Objective-C code generation.
 */

#ifndef CODIRA_SEMA_OBJC_GENERATOR_H
#define CODIRA_SEMA_OBJC_GENERATOR_H

#include <fstream>
#include <string_view>

#include "NativeFFI/ObjC/AfterTypeCheck/Interop/Context.h"
#include "NativeFFI/ObjC/Utils/Handler.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Types.h"

namespace Codira::Interop::ObjC {

enum class ObjCFunctionType { STATIC, INSTANCE };
enum class GenerationTarget { HEADER, SOURCE, BOTH };
enum class FunctionListFormat { DECLARATION, STATIC_REF, CODIRA_DECL };
enum class OptionalBlockOp { OPEN, CLOSE, NONE };
using ArgsList = std::vector<std::pair<std::string, std::string>>;

class ObjCGenerator {
public:
    ObjCGenerator(InteropContext& ctx, Ptr<AST::Decl> declArg, const std::string& outputFilePath,
        const std::string& codeLibOutputPath);
    void Generate();

private:
    std::string res;
    std::string resSource;
    const std::string& outputFilePath;
    const std::string& codeLibOutputPath;
    size_t currentBlockIndent = 0;
    Ptr<AST::Decl> decl;
    InteropContext& ctx;

    void OpenBlock();
    void CloseBlock(bool newLineBefore, bool newLineAfter);
    void AddWithIndent(const std::string& s, const GenerationTarget target = GenerationTarget::HEADER,
        const OptionalBlockOp bOp = OptionalBlockOp::NONE);

    std::string GenerateReturn(const std::string& statement) const;
    std::string GenerateAssignment(const std::string& lhs, const std::string& rhs) const;
    std::string GenerateIfStatement(const std::string& lhs, const std::string& rhs, const std::string& op) const;
    std::string GenerateObjCCall(const std::string& lhs, const std::string& rhs) const;
    std::string GenerateCCall(
        const std::string& funcName, const std::vector<std::string> args = std::vector<std::string>()) const;
    std::string GenerateDefaultFunctionImplementation(const std::string& name, const AST::Ty& retTy,
        const ArgsList args = ArgsList(), const ObjCFunctionType = ObjCFunctionType::INSTANCE) const;
    std::string GenerateFunctionDeclaration(
        const ObjCFunctionType type, const std::string& returnType, const std::string& name) const;
    std::string GeneratePropertyDeclaration(const ObjCFunctionType staticType, const std::string& mode,
        const std::string& type, const std::string& name) const;
    std::string GenerateImport(const std::string& name);
    std::string GenerateStaticFunctionReference(
        const std::string& funcName, const std::string& retType, const std::string& argTypes) const;
    std::string GenerateStaticReference(const std::string& name, const std::string& type,
        const std::string& defaultValue) const;
    std::string GenerateFuncParamLists(const std::vector<OwnedPtr<AST::FuncParamList>>& paramLists,
        const std::vector<std::string>& selectorComponents,
        FunctionListFormat format = FunctionListFormat::DECLARATION,
        const ObjCFunctionType type = ObjCFunctionType::INSTANCE);
    std::string MapCODETypeToObjCType(const OwnedPtr<AST::Type>& type);
    std::string MapCODETypeToObjCType(const OwnedPtr<AST::FuncParam>& param);

    ArgsList ConvertParamsListToArgsList(
        const std::vector<OwnedPtr<AST::FuncParamList>>& paramLists, bool withRegistryId);
    std::vector<std::string> ConvertParamsListToCallableParamsString(
        std::vector<OwnedPtr<AST::FuncParamList>>& paramLists, bool withSelf) const;
    std::string GenerateSetterParamLists(const std::string& type) const;
    std::string WrapperCallByInitForCODEMappingReturn(const AST::Ty& retTy, const std::string& nativeCall) const;

    void GenerateForwardDeclarations();
    void GenerateStaticFunctionsReferences();
    void GenerateFunctionSymbolsInitialization();
    void GenerateFunctionSymInit(const std::string& fName);
    void GenerateInterfaceDecl();
    void AddProperties();
    void AddConstructors();
    void AddMethods();
    void WriteToFile();

    std::string GenerateArgumentCast(const AST::Ty& retTy, std::string value) const;
};
} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_OBJC_GENERATOR_H
