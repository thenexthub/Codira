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

#include "ui_decorator.h"

#include "function.h"
#include "graph_analyzer.h"
#include "program.h"
#include "configs/guard_context.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
using OpcodeList = std::vector<panda::pandasm::Opcode>;

constexpr std::string_view TAG = "[UI_Decorator]";

constexpr std::string_view UI_DECORATOR_PREFIX = "__";
constexpr std::string_view MONITOR_UI_DECORATOR_DELIMITER = ".";

constexpr std::string_view UI_DECORATOR_FUNC_NAME_UPDATE_STATE_VARS = "updateStateVars";
constexpr std::string_view UI_DECORATOR_FUNC_NAME_RESET_STATE_VARS_ON_REUSE = "resetStateVarsOnReuse";
constexpr std::string_view UI_DECORATOR_FUNC_NAME_GET_REUSE_ID = "getReuseId";

// defined in constructor
std::unordered_map<panda::guard::UiDecoratorType, std::vector<std::string>> g_uiDecoratorByNewTable {
    {panda::guard::UiDecoratorType::LINK, {"SynchedPropertySimpleTwoWayPU", "SynchedPropertyObjectTwoWayPU"}},
    {panda::guard::UiDecoratorType::PROP, {"SynchedPropertySimpleOneWayPU", "SynchedPropertyObjectOneWayPU"}},
    {panda::guard::UiDecoratorType::OBJECT_LINK, {"SynchedPropertyNesedObjectPU"}},
    {panda::guard::UiDecoratorType::PROVIDE, {"ObservedPropertySimplePU", "ObservedPropertyObjectPU"}},
};

// defined in constructor
std::unordered_map<panda::guard::UiDecoratorType, std::vector<std::string>> g_uiDecoratorByMethodMemberTable {
    {panda::guard::UiDecoratorType::LOCAL_STORAGE_LINK, {"createLocalStorageLink"}},
    {panda::guard::UiDecoratorType::LOCAL_STORAGE_PROP, {"createLocalStorageProp"}},
    {panda::guard::UiDecoratorType::STORAGE_LINK, {"createStorageLink"}},
    {panda::guard::UiDecoratorType::STORAGE_PROP, {"createStorageProp"}},
    {panda::guard::UiDecoratorType::CONSUME, {"initializeConsume"}},
};

// defined in class constructor: PARAM(initParam)
// defined in ui class method or in toplevel function: BUILDER
// defined in ui class method updateStateVars: PARAM(updateParam)
// defined in ui class method resetStateVarsOnReuse: COMPUTED, CONSUMER, PARAM(resetParam)
// defined in toplevel function: ANIMATABLE_EXTEND
std::unordered_map<panda::guard::UiDecoratorType, std::vector<std::string>> g_uiDecoratorByMethodFieldTable {
    {panda::guard::UiDecoratorType::PARAM, {"initParam", "updateParam", "resetParam"}},
    {panda::guard::UiDecoratorType::COMPUTED, {"resetComputed"}},
    {panda::guard::UiDecoratorType::CONSUMER, {"resetConsumer"}},
    {panda::guard::UiDecoratorType::BUILDER, {"makeBuilderParameterProxy"}},
    {panda::guard::UiDecoratorType::EXTERNAL_API, {"addProvidedVar", "declareWatch"}},
    {panda::guard::UiDecoratorType::ANIMATABLE_EXTEND, {"createAnimatableProperty", "updateAnimatableProperty"}},
    {panda::guard::UiDecoratorType::REUSABLE_V2, {"reuseOrCreateNewComponent"}},
};

// defined in func_main0
std::unordered_map<panda::guard::UiDecoratorType, std::vector<std::string>> g_uiDecoratorInFuncMain0Table {
    {panda::guard::UiDecoratorType::LOCAL, {"Local"}},       {panda::guard::UiDecoratorType::EVENT, {"Event"}},
    {panda::guard::UiDecoratorType::ONCE, {"Once"}},         {panda::guard::UiDecoratorType::PARAM, {"Param"}},
    {panda::guard::UiDecoratorType::MONITOR, {"Monitor"}},   {panda::guard::UiDecoratorType::TRACK, {"Track"}},
    {panda::guard::UiDecoratorType::TRACE, {"Trace"}},       {panda::guard::UiDecoratorType::COMPUTED, {"Computed"}},
    {panda::guard::UiDecoratorType::PROVIDER, {"Provider"}}, {panda::guard::UiDecoratorType::CONSUMER, {"Consumer"}},
};

panda::guard::UiDecoratorType GetUiDecoratorType(
    const std::string &name, const std::unordered_map<panda::guard::UiDecoratorType, std::vector<std::string>> &table)
{
    if (name.empty()) {
        return panda::guard::UiDecoratorType::NONE;
    }

    for (const auto &[type, methodList] : table) {
        for (const auto &methodName : methodList) {
            if (methodName == name) {
                return type;
            }
        }
    }

    return panda::guard::UiDecoratorType::NONE;
}

panda::guard::UiDecoratorType GetNewUiDecoratorType(const std::string &name)
{
    return GetUiDecoratorType(name, g_uiDecoratorByNewTable);
}

panda::guard::UiDecoratorType GetMemberUiDecoratorType(const std::string &name)
{
    return GetUiDecoratorType(name, g_uiDecoratorByMethodMemberTable);
}

panda::guard::UiDecoratorType GetFieldUiDecoratorType(const std::string &name)
{
    return GetUiDecoratorType(name, g_uiDecoratorByMethodFieldTable);
}

panda::guard::UiDecoratorType GetFuncMain0UiDecoratorType(const std::string &name)
{
    return GetUiDecoratorType(name, g_uiDecoratorInFuncMain0Table);
}

bool IsParamUiDecoratorValid(const panda::guard::InstructionInfo &info)
{
    if (info.ins_->opcode != panda::pandasm::Opcode::CALLTHIS2) {
        return false;
    }

    if (!info.function_->component_ && (info.function_->name_ != UI_DECORATOR_FUNC_NAME_UPDATE_STATE_VARS) &&
        (info.function_->name_ != UI_DECORATOR_FUNC_NAME_RESET_STATE_VARS_ON_REUSE)) {
        return false;
    }
    return true;
}

const OpcodeList UI_DECORATOR_LIST_IN_FUNCTION = {
    panda::pandasm::Opcode::STOBJBYNAME, panda::pandasm::Opcode::CALLTHIS1, panda::pandasm::Opcode::CALLTHIS2,
    panda::pandasm::Opcode::CALLTHIS3,   panda::pandasm::Opcode::CALLARGS2, panda::pandasm::Opcode::ISIN};

const OpcodeList UI_DECORATOR_LIST_IN_FUNC_MAIN0 = {
    panda::pandasm::Opcode::CALLARG1,  panda::pandasm::Opcode::CALLARGS2, panda::pandasm::Opcode::CALLARGS3,
    panda::pandasm::Opcode::CALLRANGE, panda::pandasm::Opcode::CALLTHIS2, panda::pandasm::Opcode::CALLTHIS3,
};

}  // namespace

bool panda::guard::UiDecorator::IsUiDecoratorIns(const InstructionInfo &info, Scope scope)
{
    const auto &uiDecoratorInsList =
        (scope == TOP_LEVEL) ? UI_DECORATOR_LIST_IN_FUNC_MAIN0 : UI_DECORATOR_LIST_IN_FUNCTION;
    return std::any_of(uiDecoratorInsList.begin(), uiDecoratorInsList.end(),
                       [&](const panda::pandasm::Opcode opcode) -> bool { return info.ins_->opcode == opcode; });
}

void panda::guard::UiDecorator::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
}

void panda::guard::UiDecorator::Build()
{
    if (scope_ == TOP_LEVEL) {
        HandleInstInFuncMain0();
        return;
    }
    HandleInstInFunction();
}

void panda::guard::UiDecorator::AddDefineInsList(const std::vector<InstructionInfo> &instLIst)
{
    for (const auto &inst : instLIst) {
        AddDefineInsList(inst);
    }
}

bool panda::guard::UiDecorator::IsValidUiDecoratorType() const
{
    return (this->type_ != UiDecoratorType::NONE);
}

void panda::guard::UiDecorator::Update()
{
    if (!IsValidUiDecoratorType()) {
        return;
    }

    if (IsMonitorUiDecoratorType()) {
        UpdateMonitorDecorator();
        return;
    }

    this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
    for (auto &inst : defineInsList_) {
        inst.ins_->GetId(INDEX_0) = this->obfName_;
    }
    this->program_->prog_->strings.emplace(this->obfName_);
}

void panda::guard::UiDecorator::HandleInstInFuncMain0()
{
    std::string callName = GraphAnalyzer::GetCallName(baseInst_);
    this->type_ = GetFuncMain0UiDecoratorType(callName);
    if (!IsValidUiDecoratorType()) {
        return;
    }

    if (IsMonitorUiDecoratorType()) {
        BuildMonitorDecorator();  // CALLARG1 CALLARGS2 CALLARGS3 CALLRANGE
        return;
    }

    uint32_t paramIndex;
    if (baseInst_.equalToOpcode(pandasm::Opcode::CALLARG1)) {
        paramIndex = INDEX_0;
    } else if (baseInst_.equalToOpcode(pandasm::Opcode::CALLARGS2) ||
               baseInst_.equalToOpcode(pandasm::Opcode::CALLARGS3)) {  // CALLARGS3 COMPUTED
        paramIndex = INDEX_1;
    } else {
        PANDA_GUARD_ABORT_PRINT(TAG, ErrorCode::GENERIC_ERROR, "unsupported ui decorator call scene");
    }

    InstructionInfo param;
    GraphAnalyzer::GetCallLdaStrParam(baseInst_, paramIndex, param);
    if (!param.IsValid()) {
        this->type_ = UiDecoratorType::NONE;
        return;
    }

    this->name_ = param.ins_->GetId(INDEX_0);
    this->AddDefineInsList(param);

    LOG(INFO, PANDAGUARD) << TAG << "func_main0 found ui decorator property name:" << this->name_
                          << ", type:" << static_cast<int>(this->type_);
}

void panda::guard::UiDecorator::HandleInstInFunction()
{
    switch (baseInst_.ins_->opcode) {
        case pandasm::Opcode::STOBJBYNAME:
            HandleStObjByNameIns();
            break;
        case pandasm::Opcode::CALLTHIS1:
        case pandasm::Opcode::CALLTHIS2:
        case pandasm::Opcode::CALLTHIS3:
        case pandasm::Opcode::CALLARGS2:
            BuildCreatedByMemberFieldDecorator();
            break;
        case pandasm::Opcode::ISIN:
            BuildEventDecorator();
            break;
        default:
            break;
    }

    if (IsValidUiDecoratorType()) {
        LOG(INFO, PANDAGUARD) << TAG << "function found ui decorator property name:" << this->name_
                              << ", type:" << static_cast<int>(this->type_);
    }
}

void panda::guard::UiDecorator::HandleStObjByNameIns()
{
    if (!baseInst_.function_->component_) {
        return;
    }
    if (!StringUtil::IsPrefixMatched(baseInst_.ins_->GetId(INDEX_0), UI_DECORATOR_PREFIX.data())) {
        return;
    }
    this->name_ = baseInst_.ins_->GetId(INDEX_0).substr(UI_DECORATOR_PREFIX.size());

    InstructionInfo input;
    GraphAnalyzer::GetStObjByNameInput(baseInst_, input);
    if (!input.IsValid()) {
        return;
    }
    switch (input.ins_->opcode) {
        case pandasm::Opcode::NEWOBJRANGE:
            HandleNewObjRangeIns(input);
            break;
        case pandasm::Opcode::CALLTHIS2:
            BuildCreatedByMemberMethodDecorator(input, INDEX_2);
            break;
        case pandasm::Opcode::CALLTHIS3:
            BuildCreatedByMemberMethodDecorator(input, INDEX_3);
            break;
        default:
            break;
    }
}

void panda::guard::UiDecorator::HandleNewObjRangeIns(const InstructionInfo &info)
{
    std::string callName;
    InstructionInfo out;
    GraphAnalyzer::GetNewObjRangeInfo(info, callName, out);
    if (callName.empty() || !out.IsValid()) {
        return;
    }

    this->type_ = GetNewUiDecoratorType(callName);
    if (!IsValidUiDecoratorType()) {
        this->type_ = UiDecoratorType::NONE;
        return;
    }

    this->AddDefineInsList(out);
}

void panda::guard::UiDecorator::BuildCreatedByMemberMethodDecorator(const InstructionInfo &info, uint32_t paramIndex)
{
    std::string callName = GraphAnalyzer::GetCallName(info);
    this->type_ = GetMemberUiDecoratorType(callName);
    if (!IsValidUiDecoratorType()) {
        return;
    }

    InstructionInfo param;
    GraphAnalyzer::GetCallLdaStrParam(info, paramIndex, param);
    if (!param.IsValid()) {
        this->type_ = UiDecoratorType::NONE;
        return;
    }
    this->AddDefineInsList(param);

    if (this->type_ == UiDecoratorType::CONSUME) {
        InstructionInfo aliasParam;
        GraphAnalyzer::GetCallLdaStrParam(info, INDEX_1, aliasParam);
        if (!aliasParam.IsValid()) {
            this->type_ = UiDecoratorType::NONE;
            return;
        }
        this->AddDefineInsList(aliasParam);
    }
}

void panda::guard::UiDecorator::BuildMonitorDecorator()
{
    uint32_t maxParamCnt = baseInst_.ins_->Regs().size();
    if (baseInst_.ins_->opcode == pandasm::Opcode::CALLRANGE) {
        maxParamCnt = std::get<int64_t>(baseInst_.ins_->GetImm(INDEX_1));
    }
    for (uint32_t index = 0; index < maxParamCnt; index++) {
        InstructionInfo param;
        GraphAnalyzer::GetCallLdaStrParam(baseInst_, index, param);
        if (!param.IsValid()) {
            this->type_ = UiDecoratorType::NONE;
            return;
        }
        LOG(INFO, PANDAGUARD) << TAG << "found monitor ui decorator: " << param.ins_->GetId(INDEX_0);
        this->AddDefineInsList(param);
    }
}

void panda::guard::UiDecorator::BuildCreatedByMemberFieldDecorator()
{
    if (baseInst_.equalToOpcode(pandasm::Opcode::CALLTHIS1) && !baseInst_.function_->component_) {
        return;
    }

    std::string callName = GraphAnalyzer::GetCallName(baseInst_);
    this->type_ = GetFieldUiDecoratorType(callName);
    if (!IsValidUiDecoratorType()) {
        return;
    }

    if ((this->type_ == UiDecoratorType::PARAM) && !IsParamUiDecoratorValid(baseInst_)) {
        this->type_ = UiDecoratorType::NONE;
        return;
    }

    if (IsReusableV2UiDecoratorType()) {
        BuildReusableV2Decorator();
        return;
    }

    uint32_t paramIndex = INDEX_1;
    InstructionInfo param;
    if (this->type_ == UiDecoratorType::BUILDER) {
        paramIndex = INDEX_0;
    }
    GraphAnalyzer::GetCallLdaStrParam(baseInst_, paramIndex, param);
    if (!param.IsValid()) {
        this->type_ = UiDecoratorType::NONE;
        return;
    }

    this->name_ = param.ins_->GetId(INDEX_0);
    this->AddDefineInsList(param);
}

void panda::guard::UiDecorator::BuildEventDecorator()
{
    if (!baseInst_.function_->component_) {
        return;
    }

    std::vector<InstructionInfo> out;
    GraphAnalyzer::GetIsInInfo(baseInst_, out);
    if (out.empty()) {
        return;
    }

    this->name_ = out[INDEX_0].ins_->GetId(INDEX_0);
    this->type_ = UiDecoratorType::EVENT;
    this->AddDefineInsList(out);
}

void panda::guard::UiDecorator::UpdateMonitorDecorator()
{
    for (auto &inst : defineInsList_) {
        auto nameList = StringUtil::Split(inst.ins_->GetId(INDEX_0), MONITOR_UI_DECORATOR_DELIMITER.data());
        if (nameList.empty()) {
            continue;
        }
        std::string obfName;
        for (const auto &name : nameList) {
            obfName += GuardContext::GetInstance()->GetNameMapping()->GetName(name);
            obfName += MONITOR_UI_DECORATOR_DELIMITER;
        }
        obfName = obfName.substr(0, obfName.length() - 1);
        LOG(INFO, PANDAGUARD) << TAG << "update monitor ui decorator: " << inst.ins_->GetId(INDEX_0) << " --> "
                              << obfName;
        inst.ins_->GetId(INDEX_0) = obfName;
        this->program_->prog_->strings.emplace(obfName);
    }
}

bool panda::guard::UiDecorator::IsMonitorUiDecoratorType() const
{
    return this->type_ == UiDecoratorType::MONITOR;
}

void panda::guard::UiDecorator::AddDefineInsList(const InstructionInfo &ins)
{
    LOG(INFO, PANDAGUARD) << TAG << "ui decorator add instList:" << ins.ins_->ToString();
    this->defineInsList_.emplace_back(ins);
}

void panda::guard::UiDecorator::BuildReusableV2Decorator()
{
    this->type_ = UiDecoratorType::NONE;
    if (baseInst_.notEqualToOpcode(pandasm::Opcode::CALLTHIS1)) {
        return;
    }

    InstructionInfo createObjectInst;
    GraphAnalyzer::GetCallCreateObjectWithBufferParam(baseInst_, INDEX_1, createObjectInst);
    if (!createObjectInst.IsValid()) {
        LOG(DEBUG, PANDAGUARD) << TAG << "callthi1 inst index 1 not match createobjectwithbuffer inst";
        return;
    }

    const std::string &literalArrayIdx = createObjectInst.ins_->GetId(0);
    auto outerProperty = GetObjectOuterProperty(literalArrayIdx);
    if (!outerProperty) {
        LOG(DEBUG, PANDAGUARD) << TAG << "object not find outer property:" << literalArrayIdx;
        return;
    }

    InstructionInfo defineFuncInst;
    GraphAnalyzer::GetDefinePropertyByNameFunction(outerProperty->defineInsList_[0], defineFuncInst);
    if (!defineFuncInst.IsValid()) {
        LOG(DEBUG, PANDAGUARD) << TAG << "definepropertybyname inst index 1 not match definefunc inst";
        return;
    }

    auto func = Function(this->program_, defineFuncInst.ins_->GetId(0));
    func.EnumerateIns([&](const InstructionInfo &inst) -> void { EnumerateIns(inst); });
}

bool panda::guard::UiDecorator::IsReusableV2UiDecoratorType() const
{
    return this->type_ == UiDecoratorType::REUSABLE_V2;
}

std::shared_ptr<panda::guard::Property> panda::guard::UiDecorator::GetObjectOuterProperty(
    const std::string &literalArrayIdx) const
{
    auto it = this->objectTableRef_.find(literalArrayIdx);
    if (it == this->objectTableRef_.end()) {
        return nullptr;
    }

    for (const auto &property : it->second->outerProperties_) {
        if (property->name_ == UI_DECORATOR_FUNC_NAME_GET_REUSE_ID) {
            return property;
        }
    }
    return nullptr;
}

void panda::guard::UiDecorator::EnumerateIns(const panda::guard::InstructionInfo &inst)
{
    if (inst.notEqualToOpcode(pandasm::Opcode::LDA_STR)) {
        return;
    }

    this->name_ = inst.ins_->GetId(INDEX_0);
    this->type_ = UiDecoratorType::REUSABLE_V2;
    this->AddDefineInsList(inst);
}