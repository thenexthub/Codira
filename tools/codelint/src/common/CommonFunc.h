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

#ifndef CODIRACODECHECK_COMMONFUNC_H
#define CODIRACODECHECK_COMMONFUNC_H

#include <string>
#include <vector>
#include "Codira/AST/Node.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/EnumDef.h"
#include "Codira/CHIR/Type/StructDef.h"
#include "Codira/CHIR/Type/Type.h"
#include "common/CODELintCompilerInvocation.h"
#include "common/CommonData.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "nlohmann/json.hpp"

namespace Codira::CodeCheck {
const int OK = 0;
const int ERR = 255;
const int JSON_READ_ERR = 254;

class CommonFunc {
public:
    CommonFunc() = default;
    ~CommonFunc() = default;
    using PositionPair = std::pair<Codira::Position, Codira::Position>;
    static PositionPair GetCodePosition(const CHIR::Expression* expr);
    static PositionPair GetCodePosition(const CHIR::Value* value);
    static PositionPair GetCodePosition(const CHIR::DebugLocation loc);
    static void getAbsPath(const std::string& path, std::optional<std::string>& absPath);
    static int ReadJsonFileToJsonInfo(
        const std::string& jsonPath, ConfigContext& configContext, nlohmann::json& jsonInfo);
    static bool FindCHIRFunction(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool FindCHIRFunction(const CHIR::Value* value, const AstFuncInfo& funcInfo);
    static std::string GetChainMemberPathName(const CHIR::GetElementRef* getElementRef);
    static std::string GetChainMemberName(const CHIR::GetElementRef* getElementRef);
    static std::string GetFuncDeclParentTyName(Ptr<const Codira::AST::FuncDecl> pFuncDecl);
    static bool HasEnding(std::string const& fullString, std::string const& ending);
    static std::string PrintType(const CHIR::Type* ty);
    static std::string PrintTypeWithArgs(const CHIR::Type* ty);
    static CHIRFuncInfo GetCHIRFuncInfo(const CHIR::Value* value);
    static std::tuple<std::string, CHIR::CustomType*, CHIR::FuncType*> GetCHIRFuncInfoKindFunc(
        const CHIR::Value* value);
    static std::tuple<std::string, CHIR::CustomType*, CHIR::FuncType*> GetCHIRFuncInfoKindImpFunc(
        const CHIR::Value* value);
    static bool CheckFuncName(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool CheckArgumentsTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo, bool inClass = false);
    static bool CheckReturnTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool CheckParentTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool CheckPkgName(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo);
    static bool IsGenericInstantated(const CHIR::Func* func);
    static bool IsStdDerivedMacro(CodeCheckDiagnosticEngine* engine, const Codira::Position& pos);
private:
    static std::vector<uint64_t> emptyPath;
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_COMMONFUNC_H
