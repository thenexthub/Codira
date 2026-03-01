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

#include "DataflowRuleGOTH01Check.h"
#include "Codira/AST/Types.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {

using namespace Codira::CHIR;
using namespace Codira::AST;
using Json = nlohmann::json;

DataflowRuleGOTH01Check::DataflowRuleGOTH01Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}

static bool IsLoggerSubTy(const Codira::CHIR::ClassType* classType)
{
    auto classDef = classType->GetClassDef();
    if (classDef && classDef->GetSrcCodeIdentifier() == "Logger" && classDef->GetPackageName() == "stdx.log") {
        return true;
    }
    for (auto interface : classDef->GetImplementedInterfaceTys()) {
        if (IsLoggerSubTy(interface)) {
            return true;
        }
    }
    const auto& extendDefs = classDef->GetExtends();
    for (auto extendDef : extendDefs) {
        for (auto interfaceTy : extendDef->GetImplementedInterfaceTys()) {
            if (IsLoggerSubTy(interfaceTy)) {
                return true;
            }
        }
    }
    
    auto superClass = classDef->GetSuperClassTy();
    if (superClass && IsLoggerSubTy(superClass)) {
        return true;
    }
    return false;
}

static bool IsLogger(const CHIRFuncInfo& chirFuncInfo, std::string funcName)
{
    AstFuncInfo func = AstFuncInfo(funcName, "Logger", {"Logger", "String", "Array"}, "Unit", "stdx.log");
    if (funcName == "log") {
        func = AstFuncInfo(funcName, "Logger", {"Logger", "LogLevel", "String", "Array"}, "Unit", "stdx.log");
    }
    if (!CommonFunc::CheckFuncName(chirFuncInfo, func) ||
        !CommonFunc::CheckReturnTy(chirFuncInfo, func)) {
        return false;
    }

    if (CommonFunc::CheckPkgName(chirFuncInfo, func) && CommonFunc::CheckParentTy(chirFuncInfo, func)) {
        return true;
    }

    if (!chirFuncInfo.parentTy || !chirFuncInfo.parentTy->IsClass()) {
        return false;
    }
    auto classTy = RawStaticCast<const CHIR::ClassType*>(chirFuncInfo.parentTy);
    return IsLoggerSubTy(classTy);
}
static std::string GetLogMsg(const CHIR::Expression* expr);
static std::string GetSubLogMsg(const CHIR::Value* value)
{
    auto localVar = dynamic_cast<const CHIR::LocalVar*>(value);
    if (!localVar) {
        return "";
    }
    auto subExpr = localVar->GetExpr();
    return GetLogMsg(subExpr);
}

// Retrieve string information from different expressions.
static std::string GetLogMsg(const CHIR::Expression* expr)
{
    if (!expr) {
        return "";
    }
    switch (expr->GetExprKind()) {
        case CHIR::ExprKind::CONSTANT: {
            auto constant = static_cast<const CHIR::Constant*>(expr);
            return constant->IsStringLit() ? constant->GetStringLitVal() : "";
        }
        case CHIR::ExprKind::APPLY: {
            auto applyAdd = static_cast<const CHIR::Apply*>(expr);
            auto add = applyAdd->GetCallee();
            if (add->IsImportedFunc()) {
                auto importedVal = VirtualCast<CHIR::ImportedFunc*>(add);
                if (importedVal->GetSrcCodeIdentifier() == "+" && importedVal->GetSourcePackageName() == "std.core") {
                    return GetSubLogMsg(applyAdd->GetArgs()[0]) + GetSubLogMsg(applyAdd->GetArgs()[1]);
                }
            }
            return "";
        }
        default:
            return "";
    }
}
// Log msg is obtained from args[1]
template <typename T> static std::string GetSpecialLogMsg(Ptr<T> apply, const CHIR::ConstDomain& state)
{
    auto arg = static_cast<CHIR::LocalVar*>(apply->GetArgs()[1]);
    auto absVal = state.CheckAbstractValue(arg);
    if (absVal) {
        return StaticCast<CHIR::ConstStrVal*>(absVal)->GetVal();
    }
    auto expr = arg->GetExpr();
    return GetLogMsg(expr);
}

static std::string GetSpecialLogMsg(Ptr<CHIR::Apply> apply, const CHIR::ConstDomain& state)
{
    return GetSpecialLogMsg<CHIR::Apply>(apply, state);
}

static std::string GetSpecialLogMsg(Ptr<CHIR::ApplyWithException> applyWithExcept, const CHIR::ConstDomain& state)
{
    return GetSpecialLogMsg<CHIR::ApplyWithException>(applyWithExcept, state);
}
// For 'SimpleLogger.log()', require the log level to be not equal to 0, where 0 indicates off.
template <typename T> static bool GetLogLevel(Ptr<T> apply, int index)
{
    auto arg = static_cast<CHIR::LocalVar*>(apply->GetArgs()[index - 1]);
    auto expr = arg->GetExpr();
    auto load = dynamic_cast<const CHIR::Load*>(expr);
    if (load) {
        auto value = load->GetLocation();
        if ((!value->IsImportedVar() && !value->IsGlobalVarInCurPackage()) ||
            value->GetSrcCodeIdentifier() == "OFF") {
            return false;
        }
        return true;
    }
    return false;
}
// Log msg is obtained from args[2] when log level is not 0.
template <typename T> static std::string GetGeneralLogMsg(Ptr<T> apply, const CHIR::ConstDomain& state, int index = 2)
{
    if (GetLogLevel(apply, index)) {
        auto arg = static_cast<CHIR::LocalVar*>(apply->GetArgs()[index]);
        auto absVal = state.CheckAbstractValue(arg);
        if (absVal) {
            return StaticCast<CHIR::ConstStrVal*>(absVal)->GetVal();
        }
        auto expr = arg->GetExpr();
        return GetLogMsg(expr);
    }
    return "";
}

static std::string GetGeneralLogMsg(Ptr<CHIR::Apply> apply, const CHIR::ConstDomain& state)
{
    return GetGeneralLogMsg<CHIR::Apply>(apply, state);
}

static std::string GetGeneralLogMsg(Ptr<CHIR::ApplyWithException> apply, const CHIR::ConstDomain& state)
{
    return GetGeneralLogMsg<CHIR::ApplyWithException>(apply, state);
}

static std::string GetGeneralLogMsg(Ptr<CHIR::Invoke> invoke, const CHIR::ConstDomain& state)
{
    return GetGeneralLogMsg<CHIR::Invoke>(invoke, state);
}

std::set<std::string> DataflowRuleGOTH01Check::logMsgGetter = {"trace", "debug", "info", "warn", "error", "log"};

static std::set<std::string> ReturnSensitiveWords(const std::string& content, const Json& sensitiveKeys)
{
    std::set<std::string> keySet{};
    for (const auto& item : sensitiveKeys) {
        if (content.find(item.get<std::string>()) == std::string::npos) {
            continue;
        }
        keySet.insert(item.get<std::string>());
    }
    return keySet;
}

static Ptr<CHIR::Type> GetBaseType(Ptr<CHIR::Type> ty)
{
    if (!ty->IsRef()) {
        return ty;
    }
    auto ref = StaticCast<CHIR::RefType*>(ty);
    return GetBaseType(ref->GetBaseType());
}

template <typename T> static CHIRFuncInfo GetFuncInfo(Ptr<T> apply)
{
    auto call = apply->GetCallee();
    return CommonFunc::GetCHIRFuncInfo(call);
}

static CHIRFuncInfo GetFuncInfo(Ptr<CHIR::Invoke> invoke)
{
    auto funcName = invoke->GetMethodName();
    auto funcType = invoke->GetMethodType();
    auto parentTy = GetBaseType(invoke->GetObject()->GetType());
    if (!parentTy->IsClass()) {
        return CHIRFuncInfo();
    }
    auto classTy = StaticCast<CHIR::ClassType*>(parentTy);
    auto pkgName = classTy->GetClassDef()->GetPackageName();
    return CHIRFuncInfo(funcName, classTy, funcType, pkgName);
}

// Identify whether passwords, keys, and other sensitive data are stored in logs by checking the Apply node.
template <typename T> void DataflowRuleGOTH01Check::CheckApplyOrInvoke(Ptr<T> apply, const CHIR::ConstDomain& state)
{
    std::string content{};
    auto funcInfo = GetFuncInfo(apply);
    for (auto& it : logMsgGetter) {
        // Determine if it is a function related to logging.
        if (!IsLogger(funcInfo, it)) {
            continue;
        }
        if (it != "log") {
            content = GetSpecialLogMsg(apply, state);
        } else {
            content = GetGeneralLogMsg(apply, state);
        }

        // Determine if it contains sensitive words described in the configuration file.
        auto keySet = ReturnSensitiveWords(content, sensitiveKeys);
        for (auto value : keySet) {
            if (value.empty()) {
                continue;
            }
            auto loc = apply->GetDebugLocation();
            auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
            auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
            diagEngine->Diagnose(begPos, endPos, CodeCheckDiagKind::G_OTH_01_log_sensitive_information, value);
        }
        break;
    }
}

void DataflowRuleGOTH01Check::CheckBasedOnCHIR(CHIR::Package &package)
{
    Json sensitiveInfo;
    if (sensitiveKeys.empty()) {
        if (CommonFunc::ReadJsonFileToJsonInfo(
        "/config/dataflow_rule_G_OTH_01.json", ConfigContext::GetInstance(), sensitiveInfo) == ERR) {
            return;
        }
        if (!sensitiveInfo.contains("SensitiveKeyword")) {
            Errorln("/config/dataflow_rule_G_OTH_01.json", " read json data failed!");
            return;
        } else {
            sensitiveKeys = sensitiveInfo["SensitiveKeyword"];
        }
    }
    const auto actionBeforeVisitExpr = [](const CHIR::ConstDomain&, CHIR::Expression*, size_t) {};
    const auto actionAfterVisitExpr = [this](const CHIR::ConstDomain& state, CHIR::Expression* expr, size_t index) {
        if (expr->IsApply()) {
            auto apply = StaticCast<Codira::CHIR::Apply*>(expr);
            CheckApplyOrInvoke<CHIR::Apply>(apply, state);
        }
        if (expr->IsInvoke()) {
            auto invoke = StaticCast<Codira::CHIR::Invoke*>(expr);
            CheckApplyOrInvoke<CHIR::Invoke>(invoke, state);
        }
    };
    const auto actionOnTerminator = [this](const CHIR::ConstDomain& state, CHIR::Terminator* expr,
                                        std::optional<CHIR::Block*>) {
        if (expr->IsApplyWithException()) {
            auto applyWithExcept = StaticCast<Codira::CHIR::ApplyWithException*>(expr);
            CheckApplyOrInvoke<CHIR::ApplyWithException>(applyWithExcept, state);
        }
    };
    auto funcs = package.GetGlobalFuncs();
    for (auto& func : funcs) {
        if (func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        auto result = analysisWrapper->CheckFuncResult(func);
        if (!result) {
            continue;
        }
        result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
    }
}

} // namespace Codira::CodeCheck
