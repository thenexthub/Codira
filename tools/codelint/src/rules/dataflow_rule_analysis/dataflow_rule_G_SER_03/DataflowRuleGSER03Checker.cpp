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

#include "DataflowRuleGSER03Checker.h"
#include "Codira/AST/Types.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {

using namespace Codira::CHIR;

using namespace Codira::AST;

/**
 * In this file, we often analysis function calling which be translated to APPLY node in chir
 * APPLY node can be dynamic_cast to Ptr<Operation>, and then we analysis APPLY node via GetArgStricts()
 * GetArgStricts()[0] is function defination, we can get function name from it
 * GetArgStricts()[1] is 1st parameter of function
 * GetArgStricts()[2] is 2nd parameter of function
 * GetArgStricts()[3] ...
 *
 * If it's a member function of some class, then 1st param of APPLY node is the object of this class,
 * and 2nd param of APPLY node is the first input param of this function
 */

DataflowRuleGSER03Checker::DataflowRuleGSER03Checker(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine)
{
}

static DebugLocation GetlocalVarLoc(const CHIR::LocalVar* localVar)
{
    auto expr = localVar->GetExpr();
    if (expr->GetExprKind() == CHIR::ExprKind::LOAD) {
        auto load = StaticCast<CHIR::Load*>(expr);
        auto value = load->GetLocation();
        if (value->IsLocalVar()) {
            auto subLocalVar = StaticCast<CHIR::LocalVar*>(value);
            return subLocalVar->GetExpr()->GetDebugLocation();
        }
    }
    return expr->GetDebugLocation();
}

DataflowRuleGSER03Checker::TypeWithPosition DataflowRuleGSER03Checker::TypeInSer(
    const CHIR::Apply* apply, bool inDataStruct)
{
    AstFuncInfo funcInfo = AstFuncInfo("serialize", NOT_CARE, {NOT_CARE}, "DataModel", NOT_CARE);
    auto callee = apply->GetCallee();
    if (CommonFunc::FindCHIRFunction(callee, funcInfo)) {
        auto serType = CommonFunc::GetCHIRFuncInfo(callee).funcTy->GetParamTypes()[0];
        auto serTypeName = CommonFunc::PrintTypeWithArgs(serType);
        if (inDataStruct && apply->GetArgs()[0]->IsLocalVar()) {
            auto localVar = StaticCast<CHIR::LocalVar*>(apply->GetArgs()[0]);
            return std::make_pair(serTypeName, CommonFunc::GetCodePosition(GetlocalVarLoc(localVar)));
        } else {
            return std::make_pair(serTypeName, CommonFunc::GetCodePosition(apply));
        }
    }
    return std::make_pair("", std::make_pair(Codira::Position(), Codira::Position()));
}

DataflowRuleGSER03Checker::TypeWithPosition DataflowRuleGSER03Checker::TypeInDeser(const CHIR::Apply* apply)
{
    AstFuncInfo funcInfo = AstFuncInfo("deserialize", NOT_CARE, {"DataModel"}, NOT_CARE, NOT_CARE);
    auto callee = apply->GetCallee();
    if (CommonFunc::FindCHIRFunction(callee, funcInfo)) {
        // Adaptation to generic functions
        auto deserType = apply->GetThisType() ? apply->GetResultType()
                                              : CommonFunc::GetCHIRFuncInfo(callee).funcTy->GetReturnType();
        auto deserTypeName = CommonFunc::PrintTypeWithArgs(deserType);
        return std::make_pair(deserTypeName, CommonFunc::GetCodePosition(apply));
    }
    return std::make_pair("", std::make_pair(Codira::Position(), Codira::Position()));
}

static std::function<bool(CHIR::ClassDef*)> isSubClass = [](CHIR::ClassDef* classDef) -> bool {
    if (classDef->GetSrcCodeIdentifier() == "Serializable" &&
        classDef->GetPackageName() == "stdx.serialization.serialization") {
        return true;
    }
    auto superClass = classDef->GetSuperClassDef();
    if (superClass && isSubClass(superClass)) {
        return true;
    }
    for (auto interface : classDef->GetImplementedInterfaceDefs()) {
        if (isSubClass(interface)) {
            return true;
        }
    }
    const auto& extendDefs = classDef->GetExtends();
    for (auto extendDef : extendDefs) {
        for (auto interface : extendDef->GetImplementedInterfaceDefs()) {
            if (isSubClass(interface)) {
                return true;
            }
        }
    }
    return false;
};

template <typename T, typename = decltype(std::declval<T>().GetImplementedInterfaceDefs())>
static bool IsSubClass(const T* def)
{
    auto implementedInterfaces = def->GetImplementedInterfaceDefs();
    if (std::any_of(implementedInterfaces.begin(), implementedInterfaces.end(), isSubClass)) {
        return true;
    }
    const auto& extendDefs = def->GetExtends();
    for (auto extendDef : extendDefs) {
        auto extendInterfaces = extendDef->GetImplementedInterfaceDefs();
        if (std::any_of(extendInterfaces.begin(), extendInterfaces.end(), isSubClass)) {
            return true;
        }
    }
    return false;
}

void DataflowRuleGSER03Checker::CollectSerRelatedFuncsInDef(CHIR::CustomTypeDef* classDef)
{
    auto identifier = classDef->GetSrcCodeIdentifier();
    AstFuncInfo serializeFunc = AstFuncInfo("serialize", identifier, {identifier}, "DataModel", NOT_CARE);
    AstFuncInfo deserializeFunc = AstFuncInfo("deserialize", identifier, {"DataModel"}, identifier, NOT_CARE);
    bool withSerialize = false;
    bool withDeserialize = false;
    CHIR::Func* serialFunc{};
    CHIR::Func* deSerialFunc{};
    auto customType = StaticCast<CHIR::CustomType*>(classDef->GetType());
    for (auto method : classDef->GetMethods()) {
        auto funcType = static_cast<CHIR::FuncType*>(method->GetType());
        auto chirFuncInfo = CHIRFuncInfo(method->GetSrcCodeIdentifier(), Ptr(customType), Ptr(funcType), NOT_CARE);
        if (!withDeserialize && CommonFunc::FindCHIRFunction(chirFuncInfo, serializeFunc)) {
            withDeserialize = true;
            serialFunc = StaticCast<CHIR::Func*>(method);
        }
        if (!withSerialize && CommonFunc::FindCHIRFunction(chirFuncInfo, deserializeFunc)) {
            withSerialize = true;
            deSerialFunc = StaticCast<CHIR::Func*>(method);
        }
        if (withDeserialize && withSerialize) {
            break;
        }
    }
    if (serialFunc && deSerialFunc) {
        allSerialiseClass[classDef] = std::make_pair(serialFunc, deSerialFunc);
        typeToDataModel[classDef->GetSrcCodeIdentifier()] = "DataModel" + classDef->GetSrcCodeIdentifier();
    }
}

void DataflowRuleGSER03Checker::CollectClassDefWithSer(CHIR::Package& package)
{
    for (auto classDef : package.GetClasses()) {
        // Requirement: The class must be a subclass of the Serializable interface.
        if (IsSubClass<CHIR::ClassDef>(classDef)) {
            CollectSerRelatedFuncsInDef(classDef);
        }
    }
    for (auto structDef : package.GetStructs()) {
        // Requirement: The struct must be a subclass of the Serializable interface.
        if (IsSubClass<CHIR::StructDef>(structDef)) {
            CollectSerRelatedFuncsInDef(structDef);
        }
    }
}

static std::string GetStrLitVal(const CHIR::Value* value)
{
    auto localVar = DynamicCast<CHIR::LocalVar*>(value);
    if (!localVar) {
        return "";
    }
    auto constant = DynamicCast<CHIR::Constant*>(localVar->GetExpr());
    if (constant && constant->GetValue()->IsLiteral()) {
        return constant->GetStringLitVal();
    }
    return "";
}

std::string DataflowRuleGSER03Checker::GetValueIdentifier(const CHIR::Value* value)
{
    auto identifier = value->GetSrcCodeIdentifier();
    if (!identifier.empty()) {
        return identifier;
    }
    if (value->IsLocalVar()) {
        auto localVar = StaticCast<CHIR::LocalVar*>(value);
        if (localVar->GetExpr()->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
            auto getElementRef = StaticCast<const CHIR::GetElementRef*>(localVar->GetExpr());
            return CommonFunc::GetChainMemberName(getElementRef);
        }
    }
    return identifier;
}

void DataflowRuleGSER03Checker::SetSerInRetToFieldSerMapHelper(
    CHIR::Load* load, TypeWithPosition& typeStr, SerialiseMap& serTypeMap)
{
    auto var = load->GetLocation();
    auto identifier = GetValueIdentifier(var);
    if (serTypeMap.count(identifier) == 0) {
        serTypeMap[identifier] = {typeStr};
    } else {
        serTypeMap[identifier].emplace_back(typeStr);
    }
}

void DataflowRuleGSER03Checker::SetSerInRetToFieldSerMap(
    CHIR::Apply* apply, TypeWithPosition& typeStr, SerialiseMap& serTypeMap)
{
    auto localVar = DynamicCast<CHIR::LocalVar*>(apply->GetArgs()[0]);
    if (localVar && localVar->GetExpr()->GetExprKind() == CHIR::ExprKind::LOAD) {
        auto load = StaticCast<CHIR::Load*>(localVar->GetExpr());
        SetSerInRetToFieldSerMapHelper(load, typeStr, serTypeMap);
    }
    if (localVar && localVar->GetExpr()->GetExprKind() == CHIR::ExprKind::TYPECAST) {
        auto typecast = StaticCast<CHIR::TypeCast*>(localVar->GetExpr());
        localVar = DynamicCast<CHIR::LocalVar*>(typecast->GetSourceValue());
        if (!localVar) {
            return;
        }
        auto load = DynamicCast<CHIR::Load*>(localVar->GetExpr());
        if (load) {
            SetSerInRetToFieldSerMapHelper(load, typeStr, serTypeMap);
        }
    }
}

bool DataflowRuleGSER03Checker::CollectSerTypeInStore(
    const CHIR::Store* store, std::vector<TypeWithPosition>& serTypes, SerialiseMap& serMap, CHIR::LocalVar* ret)
{
    if (store->GetLocation() == ret) {
        auto localVar = StaticCast<CHIR::LocalVar*>(store->GetValue());
        auto apply = DynamicCast<CHIR::Apply*>(localVar->GetExpr());
        if (apply) {
            auto serType = TypeInSer(apply);
            if (!serType.first.empty()) {
                SetSerInRetToFieldSerMap(apply, serType, serMap);
                serTypes.emplace_back(serType);
            }
        }
        return true;
    }
    return false;
}

bool DataflowRuleGSER03Checker::CollectSerTypeInInitApply(const CHIR::Apply* apply, CHIR::ImportedValue* imported,
    std::vector<TypeWithPosition>& serTypes, SerialiseMap& fieldSerMap)
{
    AstFuncInfo funcInfo =
        AstFuncInfo("init", NOT_CARE, {"Field", "String", "DataModel"}, "Void", "stdx.serialization.serialization");
    if (CommonFunc::FindCHIRFunction(imported, funcInfo)) {
        auto litVal = GetStrLitVal(apply->GetArgs()[1]);
        if (!litVal.empty()) {
            auto ser = StaticCast<CHIR::LocalVar*>(apply->GetArgs()[2]);
            auto serTypeString = TypeInSer(StaticCast<CHIR::Apply*>(ser->GetExpr()), true);
            if (serTypeString.first.empty()) {
                return true;
            }
            if (fieldSerMap.count(litVal) == 0) {
                fieldSerMap[litVal] = {serTypeString};
            } else {
                fieldSerMap[litVal].emplace_back(serTypeString);
            }
        }
        return true;
    }
    return false;
}

bool DataflowRuleGSER03Checker::CollectSerTypeInAddApply(const CHIR::Apply* apply, CHIR::ImportedValue* imported,
    std::vector<TypeWithPosition>& serTypes, SerialiseMap& fieldSerMap)
{
    auto funcInfo = AstFuncInfo(
        "add", NOT_CARE, {"DataModelStruct", "Field"}, "DataModelStruct", "stdx.serialization.serialization");
    if (CommonFunc::FindCHIRFunction(imported, funcInfo)) {
        auto expr = StaticCast<CHIR::LocalVar*>(apply->GetArgs()[1])->GetExpr();
        if (expr->GetExprKind() != CHIR::ExprKind::APPLY) {
            return true;
        }
        auto newApply = StaticCast<CHIR::Apply*>(expr);
        auto litVal = GetStrLitVal(newApply->GetArgs()[0]);
        if (!litVal.empty()) {
            auto ser = StaticCast<CHIR::LocalVar*>(newApply->GetArgs()[1]);
            // Load
            auto load = StaticCast<CHIR::Load*>(ser->GetExpr());
            auto tyStr = CommonFunc::PrintTypeWithArgs(load->GetLocation()->GetType());
            auto serTypeString = std::make_pair(tyStr, CommonFunc::GetCodePosition(apply));
            if (serTypeString.first.empty()) {
                return true;
            }
            if (fieldSerMap.count(litVal) == 0) {
                fieldSerMap[litVal] = {serTypeString};
            } else {
                fieldSerMap[litVal].emplace_back(serTypeString);
            }
        }
        return true;
    }
    return false;
}

void DataflowRuleGSER03Checker::CollectSerType(const CHIR::Expression& expr, std::vector<TypeWithPosition>& serTypes,
    SerialiseMap& serMap, SerialiseMap& fieldSerMap, CHIR::LocalVar* ret)
{
    if (!ret || (expr.GetExprKind() != CHIR::ExprKind::STORE && expr.GetExprKind() != CHIR::ExprKind::APPLY)) {
        return;
    }
    if (expr.GetExprKind() == CHIR::ExprKind::STORE) {
        auto store = StaticCast<CHIR::Store*>(&expr);
        if (CollectSerTypeInStore(store, serTypes, serMap, ret)) {
            return;
        }
    }
    auto apply = DynamicCast<CHIR::Apply*>(&expr);
    if (apply && apply->GetCallee() && apply->GetCallee()->IsImportedFunc()) {
        auto imported = VirtualCast<CHIR::ImportedFunc>(apply->GetCallee());
        if (CollectSerTypeInInitApply(apply, imported, serTypes, fieldSerMap)) {
            return;
        }
        if (CollectSerTypeInAddApply(apply, imported, serTypes, fieldSerMap)) {
            return;
        }
    }
}

std::string DataflowRuleGSER03Checker::JointSerializeTypeAndLine(
    std::vector<TypeWithPosition>& serTypes, bool dataModel)
{
    const int32_t minReplaceSize = 2;
    std::vector<std::string> typePart;
    for (auto& info : serTypes) {
        std::string type = info.first;
        if (dataModel) {
            type = typeToDataModel[info.first];
        }
        typePart.emplace_back("'" + type + "'(line " + std::to_string(info.second.first.line) + ")");
        typePart.emplace_back(", ");
    }
    typePart.pop_back();
    if (typePart.size() > minReplaceSize) {
        typePart[typePart.size() - minReplaceSize] = " or ";
    }
    std::string res;
    for (auto& str : typePart) {
        res += str;
    }
    return res;
}

void DataflowRuleGSER03Checker::CheckSerTypeInCallee(
    const CHIR::Apply* apply, std::vector<TypeWithPosition>& serTypes, SerialiseMap& serMap, SerialiseMap& fieldSerMap)
{
    auto callee = apply->GetCallee();
    if (callee->IsFuncWithBody()) {
        auto func = VirtualCast<CHIR::Func*>(callee);
        Visitor::Visit(*func, [this, &serTypes, &serMap, &fieldSerMap](CHIR::Expression& expr) {
            CheckSerType(expr, serTypes, serMap, fieldSerMap);
            return CHIR::VisitResult::CONTINUE;
        });
    }
}

void DataflowRuleGSER03Checker::CheckSerTypeInArgs(const CHIR::Apply* apply,
    DataflowRuleGSER03Checker::TypeWithPosition& deserTypeStr, std::vector<TypeWithPosition>& serTypes,
    SerialiseMap& serMap, SerialiseMap& fieldSerMap)
{
    if (apply->GetArgs()[0]->IsLocalVar()) {
        auto arg = StaticCast<CHIR::LocalVar*>(apply->GetArgs()[0])->GetExpr();
        auto get = DynamicCast<CHIR::Apply*>(arg);
        if (!get) {
            return;
        }
        auto getFuncInfo = AstFuncInfo(
            "get", "DataModelStruct", {"DataModelStruct", "String"}, "DataModel", "stdx.serialization.serialization");
        if (CommonFunc::FindCHIRFunction(get->GetCallee(), getFuncInfo)) {
            auto litVal = GetStrLitVal(get->GetArgs()[1]);
            if (litVal.empty()) {
                return;
            }
            auto deserInfo =
                "'" + deserTypeStr.first + "'(line " + std::to_string(deserTypeStr.second.first.line) + ")";
            auto serTypes = serMap[litVal];
            auto fieldSerTypes = fieldSerMap[litVal];
            if (fieldSerTypes.empty() && !serTypes.empty()) {
                auto serInfo = JointSerializeTypeAndLine(serTypes, true);
                diagEngine->Diagnose(deSerFuncLocation.first, deSerFuncLocation.second,
                    CodeCheckDiagKind::G_SER_03_return_error, litVal, deserInfo, serInfo);
                return;
            }
            if (fieldSerTypes.empty() && serTypes.empty()) {
                diagEngine->Diagnose(deSerFuncLocation.first, deSerFuncLocation.second,
                    CodeCheckDiagKind::G_SER_03_key_lack, litVal, deserInfo, litVal);
                return;
            }
            bool hasDeserType = std::any_of(fieldSerTypes.begin(), fieldSerTypes.end(),
                [&deserTypeStr](const auto& element) { return element.first == deserTypeStr.first; });
            if (!fieldSerTypes.empty() && !hasDeserType) {
                auto serInfo = JointSerializeTypeAndLine(fieldSerTypes);
                diagEngine->Diagnose(deSerFuncLocation.first, deSerFuncLocation.second,
                    CodeCheckDiagKind::G_SER_03_type_error, litVal, deserInfo, serInfo);
            }
        }
    }
}

void DataflowRuleGSER03Checker::CheckSerType(const CHIR::Expression& expr, std::vector<TypeWithPosition>& serTypes,
    SerialiseMap& serMap, SerialiseMap& fieldSerMap)
{
    auto apply = DynamicCast<CHIR::Apply*>(&expr);
    if (!apply) {
        return;
    }
    auto deserTypeStr = TypeInDeser(apply);
    if (deserTypeStr.first.empty()) {
        CheckSerTypeInCallee(apply, serTypes, serMap, fieldSerMap);
        return;
    }
    CheckSerTypeInArgs(apply, deserTypeStr, serTypes, serMap, fieldSerMap);
    bool hasDeserType = std::any_of(serTypes.begin(), serTypes.end(),
        [&deserTypeStr](const auto& element) { return element.first == deserTypeStr.first; });
    if (!serTypes.empty() && !hasDeserType) {
        auto serInfo = JointSerializeTypeAndLine(serTypes);
        diagEngine->Diagnose(deSerFuncLocation.first, deSerFuncLocation.second, CodeCheckDiagKind::G_SER_03_type_lack,
            deserTypeStr.first, CommonFunc::GetCodePosition(apply).first.line, serInfo);
    }
}

void DataflowRuleGSER03Checker::CheckBasedOnCHIR(CHIR::Package& package)
{
    CollectClassDefWithSer(package);
    for (auto it : allSerialiseClass) {
        std::vector<TypeWithPosition> serTypes;
        SerialiseMap fieldSerMap;
        SerialiseMap serMap;
        auto serFunc = it.second.first;
        CHIR::LocalVar* ret = nullptr;
        Visitor::Visit(*serFunc, [this, &serTypes, &serMap, &fieldSerMap, &ret](CHIR::Expression& expr) {
            if (auto res = expr.GetResult(); res && res->IsRetValue()) {
                ret = expr.GetResult();
            }
            CollectSerType(expr, serTypes, serMap, fieldSerMap, ret);
            return CHIR::VisitResult::CONTINUE;
        });
        auto deSerFunc = it.second.second;
        deSerFuncLocation = CommonFunc::GetCodePosition(deSerFunc);
        Visitor::Visit(*deSerFunc, [this, &serTypes, &serMap, &fieldSerMap](CHIR::Expression& expr) {
            CheckSerType(expr, serTypes, serMap, fieldSerMap);
            return CHIR::VisitResult::CONTINUE;
        });
    }
}
} // namespace Codira::CodeCheck
