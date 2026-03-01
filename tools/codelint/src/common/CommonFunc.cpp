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

#include "CommonFunc.h"
#include "Codira/AST/Node.h"

namespace Codira::CodeCheck {
using namespace Codira::CodeCheck;

using namespace Codira::CHIR;

void CommonFunc::getAbsPath(const std::string& path, std::optional<std::string>& absPath)
{
    if (path.empty()) {
        absPath = path;
        return;
    }
    absPath = FileUtil::GetAbsPath(path);
    if (!absPath.has_value()) {
        absPath = path;
    }
}

int CommonFunc::ReadJsonFileToJsonInfo(
    const std::string& jsonPath, ConfigContext& configContext, nlohmann::json& jsonInfo)
{
    std::string path = configContext.GetConfigFileDir();
    if (path.empty()) {
        path = configContext.GetCodelintConfigPath();
    }
    std::ifstream inputFile(FileUtil::JoinPath(path, jsonPath).c_str());
    if (!inputFile.is_open()) {
        if (jsonPath != "/config/exclude_lists.json") {
            Errorln(jsonPath, " open json file failed!");
        }
        return ERR;
    }
#ifndef CODIRA_ENABLE_GCOV
    try {
#endif

        inputFile >> jsonInfo;
#ifndef CODIRA_ENABLE_GCOV
        if (inputFile.fail() || !jsonInfo.is_object()) {
            throw std::runtime_error("read json data failed!");
        }
    } catch (...) {
        Errorln(jsonPath, " read json data failed!");
        inputFile.close();
        return ERR;
    }
#endif
    inputFile.close();
    return OK;
}

std::tuple<std::string, CHIR::CustomType*, CHIR::FuncType*> CommonFunc::GetCHIRFuncInfoKindFunc(
    const CHIR::Value* value)
{
    std::string packageName;
    CHIR::CustomType* parentTy = nullptr;
    CHIR::FuncType* funcTy = nullptr;
    auto func = VirtualCast<CHIR::Func*>(value);
    const CHIR::CustomTypeDef* parentCustomTypeDef = func->GetParentCustomTypeDef();
    if (parentCustomTypeDef) {
        // For ExtendDef
        if (parentCustomTypeDef->IsExtend()) {
            auto customType = DynamicCast<CHIR::CustomType>(
                StaticCast<CHIR::ExtendDef*>(parentCustomTypeDef)->GetExtendedType());
            if (customType) {
                parentCustomTypeDef = customType->GetCustomTypeDef();
            }
        }
        packageName = parentCustomTypeDef->GetPackageName();
        if (parentCustomTypeDef->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
            auto genericDecl = parentCustomTypeDef->GetGenericDecl();
            if (genericDecl) {
                packageName = genericDecl->GetPackageName();
            }
        }
        parentTy = static_cast<CHIR::CustomType*>(parentCustomTypeDef->GetType());
    } else {
        packageName = func->GetPackageName();
        parentTy = nullptr;
    }
    funcTy = const_cast<CHIR::FuncType*>(func->GetFuncType());

    return {packageName, parentTy, funcTy};
}

std::tuple<std::string, CHIR::CustomType*, CHIR::FuncType*> CommonFunc::GetCHIRFuncInfoKindImpFunc(
    const CHIR::Value* value)
{
    std::string packageName;
    CHIR::CustomType* parentTy = nullptr;
    CHIR::FuncType* funcTy = nullptr;
    auto imported = VirtualCast<CHIR::ImportedFunc*>(value);
    auto parentCustomTypeDef = imported->GetParentCustomTypeDef();
    if (parentCustomTypeDef) {
        packageName = parentCustomTypeDef->GetPackageName();
        parentTy = static_cast<CHIR::CustomType*>(parentCustomTypeDef->GetType());
    } else {
        packageName = imported->GetSourcePackageName();
        parentTy = nullptr;
    }
    funcTy = StaticCast<CHIR::FuncType*>(imported->GetType());
    return {packageName, parentTy, funcTy};
}

CHIRFuncInfo CommonFunc::GetCHIRFuncInfo(const CHIR::Value* value)
{
    std::string packageName;
    CHIR::CustomType* parentTy = nullptr;
    CHIR::FuncType* funcTy = nullptr;
    if (value->IsFuncWithBody()) {
        std::tie(packageName, parentTy, funcTy) = GetCHIRFuncInfoKindFunc(value);
    } else if (value->IsImportedFunc()) {
        std::tie(packageName, parentTy, funcTy) = GetCHIRFuncInfoKindImpFunc(value);
    }
    return CHIRFuncInfo{value->GetSrcCodeIdentifier(), Ptr(parentTy), Ptr(funcTy), packageName};
}

bool CommonFunc::CheckPkgName(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo)
{
    return funcInfo.pkgName == NOT_CARE || funcInfo.pkgName == chirFuncInfo.pkgName;
}

bool CommonFunc::CheckFuncName(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo)
{
    return funcInfo.funcName == chirFuncInfo.funcName;
}

bool CommonFunc::CheckParentTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo)
{
    if (funcInfo.parentTy != NOT_CARE) {
        if (funcInfo.parentTy == ANY_TYPE && chirFuncInfo.parentTy) {
            return true;
        }
        if (chirFuncInfo.parentTy) {
            auto def = chirFuncInfo.parentTy->GetCustomTypeDef();
            if (def && def->GetSrcCodeIdentifier() != funcInfo.parentTy) {
                return false;
            }
        }
    }
    return true;
}

CommonFunc::PositionPair CommonFunc::GetCodePosition(const CHIR::Expression* expr)
{
    auto loc = expr->GetDebugLocation();
    auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
    auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
    return std::make_pair(begPos, endPos);
}

CommonFunc::PositionPair CommonFunc::GetCodePosition(const CHIR::DebugLocation loc)
{
    auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
    auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
    return std::make_pair(begPos, endPos);
}

CommonFunc::PositionPair CommonFunc::GetCodePosition(const CHIR::Value* value)
{
    auto loc = value->GetDebugLocation();
    auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
    auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
    if (begPos == Codira::Position(0, 0, 0) && value->IsLocalVar()) {
        auto localVar = StaticCast<CHIR::LocalVar*>(value);
        loc = localVar->GetExpr()->GetDebugLocation();
        begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
        endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
    }
    return std::make_pair(begPos, endPos);
}

std::string CommonFunc::PrintType(const CHIR::Type* ty)
{
    if (ty->IsClass()) {
        auto classTy = RawStaticCast<const CHIR::ClassType*>(ty);
        return classTy->GetClassDef()->GetSrcCodeIdentifier();
    }
    if (ty->IsStruct()) {
        auto structTy = RawStaticCast<const CHIR::StructType*>(ty);
        return structTy->GetStructDef()->GetSrcCodeIdentifier();
    }
    if (ty->IsEnum()) {
        auto enumTy = RawStaticCast<const CHIR::EnumType*>(ty);
        return enumTy->GetEnumDef()->GetSrcCodeIdentifier();
    }
    if (ty->IsFunc()) {
        auto funcTy = RawStaticCast<const CHIR::FuncType*>(ty);
        std::stringstream ss;
        ss << "(";
        auto argTys = funcTy->GetParamTypes();
        for (unsigned i = 0; i < argTys.size(); i++) {
            if (i > 0) {
                ss << ", ";
            }
            ss << PrintType(argTys[i]);
        }
        ss << ")";
        ss << "->" << PrintType(funcTy->GetReturnType());
        return ss.str();
    }
    if (ty->IsRef()) {
        auto refType = RawStaticCast<const CHIR::RefType*>(ty);
        return PrintType(refType->GetBaseType());
    }
    return ty->ToString();
}

std::string CommonFunc::PrintTypeWithArgs(const CHIR::Type* ty)
{
    if (ty->IsRef()) {
        auto refType = RawStaticCast<const CHIR::RefType*>(ty);
        return PrintTypeWithArgs(refType->GetBaseType());
    }
    std::string str = "";
    std::vector<CHIR::Type*> argTys;
    if (ty->IsClass()) {
        str = "Class-";
        auto classTy = RawStaticCast<const CHIR::ClassType*>(ty);
        str += classTy->GetClassDef()->GetSrcCodeIdentifier();
        argTys = classTy->GetTypeArgs();
    }
    if (ty->IsStruct()) {
        str = "Struct-";
        auto structTy = RawStaticCast<const CHIR::StructType*>(ty);
        str += structTy->GetStructDef()->GetSrcCodeIdentifier();
        argTys = structTy->GetTypeArgs();
    }
    if (ty->IsEnum()) {
        str = "Enum-";
        auto enumTy = RawStaticCast<const CHIR::EnumType*>(ty);
        str += enumTy->GetEnumDef()->GetSrcCodeIdentifier();
        argTys = enumTy->GetTypeArgs();
    }
    if (str.empty()) {
        return PrintType(ty);
    } else if (argTys.size() == 0) {
        return str;
    }
    str += ("<" + PrintTypeWithArgs(argTys[0]));
    for (size_t i = 1; i < argTys.size(); i++) {
        str += (", " + PrintTypeWithArgs(argTys[i]));
    }
    str += ">";
    return str;
}

bool CommonFunc::CheckReturnTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo)
{
    CODEC_NULLPTR_CHECK(chirFuncInfo.funcTy);
    auto returnTy = chirFuncInfo.funcTy->GetReturnType();
    if (funcInfo.returnTy != NOT_CARE) {
        if (funcInfo.returnTy != ANY_TYPE) {
            return PrintType(returnTy) == funcInfo.returnTy;
        } else {
            return !PrintType(returnTy).empty();
        }
    }
    return true;
}

bool CommonFunc::CheckArgumentsTy(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo, bool inClass)
{
    CODEC_NULLPTR_CHECK(chirFuncInfo.funcTy);
    auto args = chirFuncInfo.funcTy->GetParamTypes();
    size_t loopCnt = funcInfo.params.size() < args.size() ? funcInfo.params.size() : args.size();
    bool needToMatchArgsNum = true;
    for (size_t i = 0; i < loopCnt; ++i) {
        if (funcInfo.params[i] == ANY_TYPE) {
            continue;
        }
        if (funcInfo.params[i] == NOT_CARE) {
            needToMatchArgsNum = false;
            break;
        }
        if (inClass && i == 0) {
            continue;
        }
        if (funcInfo.params[i] != PrintType(args[i])) {
            return false;
        }
    }
    if (needToMatchArgsNum && funcInfo.params.size() != args.size()) {
        return false;
    }
    return true;
}

bool CommonFunc::FindCHIRFunction(const CHIRFuncInfo& chirFuncInfo, const AstFuncInfo& funcInfo)
{
    return CommonFunc::CheckPkgName(chirFuncInfo, funcInfo) && CommonFunc::CheckFuncName(chirFuncInfo, funcInfo) &&
        CommonFunc::CheckParentTy(chirFuncInfo, funcInfo) && CommonFunc::CheckReturnTy(chirFuncInfo, funcInfo) &&
        CommonFunc::CheckArgumentsTy(chirFuncInfo, funcInfo);
}

bool CommonFunc::FindCHIRFunction(const CHIR::Value* value, const AstFuncInfo& funcInfo)
{
    if (!value) {
        return false;
    }
    auto chirFuncInfo = GetCHIRFuncInfo(value);
    if (!chirFuncInfo.funcTy) {
        return false;
    }
    return FindCHIRFunction(chirFuncInfo, funcInfo);
}

bool CommonFunc::IsGenericInstantated(const CHIR::Func* func)
{
    if (func->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
        return true;
    }
    auto parentDef = func->GetParentCustomTypeDef();
    if (parentDef && parentDef->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
        return true;
    }
    return false;
}

std::string CommonFunc::GetChainMemberPathName(const CHIR::GetElementRef* getElementRef)
{
    std::string str;
    auto path = getElementRef->GetPath();
    auto type = getElementRef->GetLocation()->GetType();
    while (type->GetTypeKind() == CHIR::Type::TypeKind::TYPE_REFTYPE) {
        type = StaticCast<CHIR::RefType*>(type)->GetBaseType();
    }
    if (type->GetTypeKind() != CHIR::Type::TypeKind::TYPE_CLASS) {
        return "";
    }
    auto classType = StaticCast<CHIR::ClassType*>(type);
    CHIR::CustomTypeDef* parentDef = classType->GetClassDef();
    // Skip Closure's Class
    if (parentDef->GetIdentifier().find("@$Auto_Env") == 0) {
        return str;
    }
    for (auto index : path) {
        str += ".";
        str += parentDef->GetInstanceVar(index).name;
        auto parentType = DynamicCast<CHIR::StructType*>(parentDef->GetInstanceVar(index).type);
        if (parentType) {
            parentDef = parentType->GetStructDef();
        } else {
            break;
        }
    }
    return str;
}

/**
 * @brief Retrieve names of data types in the form of a.b.c
 * '''
 * struct SA {
 *  var a = 1
 * }
 * class CA {
 *  var a = SA()
 * }
 * var a = CA()
 * a.a.a
 * '''
 */
std::string CommonFunc::GetChainMemberName(const CHIR::GetElementRef* getElementRef)
{
    std::string pathStr = GetChainMemberPathName(getElementRef);
    if (pathStr.empty()) {
        return pathStr;
    }
    /**
     * In Member Functions, there is a defaulted parameter 'this'
     * when use member variables' identifier, we need to ignore 'this.' in the member access chain
     * Here is a example
     * '''
     * class A {
     *  var a = 1
     *  func foo() {
     *      a = 2 // 'a' but not 'this.a'
     *  }
     * }
     * '''
     */
    if (getElementRef->GetLocation()->IsParameter() &&
        getElementRef->GetLocation()->GetSrcCodeIdentifier() == "this") {
        return pathStr.erase(0, (pathStr.front() == '.') ? 1 : 0);
    }
    /**
     * Here is a exmaple
     * '''
     * class A {
     *  var a = 1
     * }
     * var a = A()
     * a.a = 1
     * '''
     */
    auto localVar = DynamicCast<CHIR::LocalVar*>(getElementRef->GetLocation());
    if (!localVar || !localVar->GetExpr() || localVar->GetExpr()->GetExprKind() != CHIR::ExprKind::LOAD) {
        return pathStr;
    }
    auto load = StaticCast<CHIR::Load*>(localVar->GetExpr());
    localVar = DynamicCast<CHIR::LocalVar*>(load->GetLocation());
    if (!localVar || !localVar->GetExpr()) {
        return pathStr;
    }
    auto expr = localVar->GetExpr();
    switch (expr->GetExprKind()) {
        case CHIR::ExprKind::GET_ELEMENT_REF: {
            auto newGetElementRef = StaticCast<CHIR::GetElementRef*>(expr);
            auto baseStr = GetChainMemberName(newGetElementRef);
            if (baseStr.empty()) {
                return pathStr.erase(0, (pathStr.front() == '.') ? 1 : 0);
            }
            return pathStr.insert(0, baseStr);
        }
        case CHIR::ExprKind::ALLOCATE: {
            pathStr.insert(0, ".");
            return pathStr.insert(0, localVar->GetDebugExpr()->GetSrcCodeIdentifier());
        }
        default:
            return pathStr;
    }
}

std::string CommonFunc::GetFuncDeclParentTyName(Ptr<const Codira::AST::FuncDecl> pFuncDecl)
{
    if (!pFuncDecl || !pFuncDecl->funcBody) {
        return "";
    }
    if (pFuncDecl->funcBody->parentClassLike) {
        return pFuncDecl->funcBody->parentClassLike->ty->name;
    }
    if (pFuncDecl->funcBody->parentStruct) {
        return pFuncDecl->funcBody->parentStruct->ty->name;
    }
    if (pFuncDecl->funcBody->parentEnum) {
        return pFuncDecl->funcBody->parentEnum->ty->name;
    }
    return "";
}

std::vector<uint64_t> CommonFunc::emptyPath = {};

bool CommonFunc::HasEnding(const std::string& fullString, const std::string& ending)
{
    if (fullString.length() < ending.length()) {
        return false;
    }
    return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
}

static std::set<std::string> MacroInStd = {"@Derive", "@DeriveExclude", "@DeriveInclude", "@DeriveOrder"};

bool CommonFunc::IsStdDerivedMacro(CodeCheckDiagnosticEngine* engine, const Codira::Position& pos)
{
    auto begin = Codira::Position(pos.fileID, pos.line, 1);
    auto end = Codira::Position(pos.fileID, pos.line + 1, 1);
    auto str = engine->GetSourceManager().GetContentBetween(begin, end);
    size_t firstNotSpace = str.find_first_not_of(" \t");
    if (firstNotSpace == std::string::npos) {
        return false;
    }
    for (auto macro: MacroInStd) {
        auto macroSize = macro.size();
        if (str.compare(firstNotSpace, macroSize, macro) == 0) {
            return true;
        }
    }
    return false;
}
} // namespace Codira::CodeCheck
