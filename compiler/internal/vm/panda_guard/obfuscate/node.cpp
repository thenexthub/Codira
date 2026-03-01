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

#include "node.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "graph_analyzer.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
using OpcodeList = std::vector<panda::pandasm::Opcode>;

constexpr std::string_view TAG = "[Node]";
constexpr std::string_view ENTRY_FUNC_NAME = ".func_main_0";
constexpr std::string_view NORMALIZED_OHM_DELIMITER = "&";
constexpr std::string_view PATH_DELIMITER = "/";
constexpr std::string_view PACKAGE_MODULES_PREFIX = "pkg_modules";
constexpr std::string_view PKG_NAME_PREFIX = "pkgName@";
constexpr std::string_view SCOPE_NAMES_FIELD = "scopeNames";
constexpr std::string_view MODULE_RECORD_IDX_FIELD = "moduleRecordIdx";
constexpr std::string_view JSON_FILE_FIELD = "jsonFileContent";
constexpr size_t MAX_EXPORT_ITEM_LEN = 10000;
constexpr size_t STOBJBYVALUE_ACC_INDEX = 2;

constexpr std::string_view OBJECT_PROPERTY_OBJECT = "Object";
constexpr std::string_view OBJECT_PROPERTY_PROTOTYPE = "prototype";
constexpr std::string_view OBJECT_PROPERTY_GETOWNPROPERTYDESCRIPTOR = "getOwnPropertyDescriptor";
constexpr std::string_view OBJECT_PROPERTY_DEFINEPROPERTY = "defineProperty";
constexpr size_t NAMESPACE_OWN_PARAM_REG_INDEX = 3;  // a3(this)

const OpcodeList METHOD_NAME_DIRECT_LIST = {
    panda::pandasm::Opcode::STOWNBYNAME,
    panda::pandasm::Opcode::STOWNBYNAMEWITHNAMESET,
};

const OpcodeList METHOD_NAME_INDIRECT_LIST = {
    panda::pandasm::Opcode::DEFINEGETTERSETTERBYVALUE,
    panda::pandasm::Opcode::STOWNBYVALUE,
    panda::pandasm::Opcode::STOWNBYVALUEWITHNAMESET,
};

bool InOpcodeList(const panda::guard::InstructionInfo &info, const OpcodeList &list)
{
    return std::any_of(list.begin(), list.end(), [&](const auto &elem) { return elem == info.ins_->opcode; });
}

void UpdateScopeNamesLiteralArray(panda::pandasm::LiteralArray &literalArray)
{
    for (auto &literal : literalArray.literals_) {
        if (!literal.IsStringValue()) {
            continue;
        }

        const auto &value = std::get<std::string>(literal.value_);
        literal.value_ = panda::guard::GuardContext::GetInstance()->GetNameMapping()->GetName(value);
    }
}

template <typename T>
void UpdateEntityNamespaceMemberExport(const std::string &entityIdx,
                                       const std::unordered_map<std::string, std::shared_ptr<T>> &map)
{
    PANDA_GUARD_ASSERT_PRINT(map.find(entityIdx) == map.end(), TAG, panda::guard::ErrorCode::GENERIC_ERROR,
                             "invalid entityIdx:" << entityIdx);
    const auto &entity = map.at(entityIdx);
    LOG(INFO, PANDAGUARD) << TAG << "update namespace export for entity:" << entityIdx;
    entity->SetExportAndRefreshNeedUpdate(true);
}

bool IsRemoteHar(const std::string &name)
{
    return panda::guard::StringUtil::IsPrefixMatched(name, PACKAGE_MODULES_PREFIX.data());
}
}  // namespace

bool panda::guard::Node::IsJsonFile(const pandasm::Record &record)
{
    return std::any_of(record.field_list.begin(), record.field_list.end(),
                       [](const auto &field) { return field.name == JSON_FILE_FIELD; });
}

bool panda::guard::Node::FindPkgName(const pandasm::Record &record, std::string &pkgName)
{
    return std::any_of(
        record.field_list.begin(), record.field_list.end(), [&](const panda::pandasm::Field &field) -> bool {
            const bool found = field.name.rfind(PKG_NAME_PREFIX, 0) == 0;
            if (found) {
                pkgName = field.name.substr(PKG_NAME_PREFIX.size(), field.name.size() - PKG_NAME_PREFIX.size());
            }
            return found;
        });
}

void panda::guard::Node::InitWithRecord(const pandasm::Record &record)
{
    if (IsJsonFile(record)) {
        this->type_ = NodeType::JSON_FILE;
        LOG(INFO, PANDAGUARD) << TAG << "json file:" << this->name_;
        return;
    }

    std::string pkgName;
    if (FindPkgName(record, pkgName)) {
        this->type_ = NodeType::SOURCE_FILE;
        this->pkgName_ = pkgName;
        LOG(INFO, PANDAGUARD) << TAG << "source file:" << this->name_;
        return;
    }

    if (record.metadata->IsAnnotation()) {
        this->type_ = NodeType::ANNOTATION;
        LOG(INFO, PANDAGUARD) << TAG << "annotation:" << this->name_;
        return;
    }
}

void panda::guard::Node::Build()
{
    LOG(INFO, PANDAGUARD) << TAG << "node create for " << this->name_ << " start";

    this->sourceName_ = GuardContext::GetInstance()->GetGuardOptions()->GetSourceName(this->name_);
    this->obfSourceName_ = this->sourceName_;
    LOG(INFO, PANDAGUARD) << TAG << "node sourceName_ " << this->sourceName_;
    this->isNormalizedOhmUrl_ = GuardContext::GetInstance()->GetGuardOptions()->IsUseNormalizedOhmUrl();
    CreateFilePath();
    this->filepath_.obfName = this->filepath_.name;
    LOG(INFO, PANDAGUARD) << TAG << "pre part: " << this->filepath_.prePart;
    LOG(INFO, PANDAGUARD) << TAG << "file path: " << this->filepath_.name;
    LOG(INFO, PANDAGUARD) << TAG << "post part: " << this->filepath_.postPart;

    if (this->type_ == NodeType::SOURCE_FILE) {
        moduleRecord_.Create();

        auto entryFunc = std::make_shared<Function>(this->program_, this->name_ + ENTRY_FUNC_NAME.data(), false);
        entryFunc->Init();
        entryFunc->Create();

        const auto &function = entryFunc->GetOriginFunction();
        this->sourceFile_ = function.source_file;
        this->obfSourceFile_ = function.source_file;

        entryFunc->EnumerateIns([&](const InstructionInfo &info) -> void { EnumerateIns(info, TOP_LEVEL); });

        this->functionTable_.emplace(entryFunc->idx_, entryFunc);
    }

    this->ExtractNames();

    LOG(INFO, PANDAGUARD) << TAG << "node create for " << this->name_ << " end";
}

panda::pandasm::Record &panda::guard::Node::GetRecord() const
{
    return this->program_->prog_->record_table.at(this->obfName_);
}

void panda::guard::Node::EnumerateIns(const InstructionInfo &info, Scope scope)
{
    CreateFunction(info, scope);
    CreateProperty(info);
    CreateClass(info, scope);
    CreateOuterMethod(info);
    CreateObject(info, scope);
    CreateObjectOuterProperty(info);
    FindStLexVarName(info);
    AddNameForExportObject(info);
    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        CreateUiDecorator(info, scope);
        CreateObjectDecoratorProperty(info);
    }
    UpdateExportForNamespaceMember(info);
    CreateArray(info);
}

void panda::guard::Node::CreateFunction(const InstructionInfo &info, Scope scope)
{
    if (info.notEqualToOpcode(pandasm::Opcode::DEFINEFUNC)) {
        return;
    }

    const std::string idx = info.ins_->GetId(0);
    if (this->functionTable_.find(idx) != this->functionTable_.end()) {
        this->functionTable_.at(idx)->defineInsList_.push_back(info);
        return;
    }

    auto function = std::make_shared<Function>(this->program_, idx);
    function->node_ = this;
    function->scope_ = scope;
    function->defineInsList_.push_back(info);
    function->component_ = info.function_->component_;
    function->Init();

    function->export_ = this->moduleRecord_.IsExportVar(function->name_);
    function->Create();

    function->EnumerateIns([&](const InstructionInfo &insInfo) -> void { EnumerateIns(insInfo, FUNCTION); });

    this->functionTable_.emplace(function->idx_, function);
}

/**
 * e.g. class A {
 *  constructor {
 *    this.v1 = 1;
 *
 *    let obj = {};
 *    obj.v2 = 2;
 *  }
 * }
 * v1: property, bind function
 * v2: variable property, bind object
 */
void panda::guard::Node::CreateProperty(const InstructionInfo &info) const
{
    if (!Property::IsPropertyIns(info)) {
        return;
    }

    InstructionInfo nameInfo;
    Property::GetPropertyNameInfo(info, nameInfo);
    // if function is enum function, try to find property in acc
    if (!nameInfo.IsValid() && info.function_->type_ == FunctionType::ENUM_FUNCTION) {
        // e.g. this[0] = 'property'
        LOG(INFO, PANDAGUARD) << TAG << "try to find property in acc";
        GraphAnalyzer::GetLdaStr(info, nameInfo, STOBJBYVALUE_ACC_INDEX);
    }

    if (!nameInfo.IsValid()) {
        LOG(INFO, PANDAGUARD) << TAG << "invalid nameInfo:" << info.index_ << " " << info.ins_->ToString();
        return;
    }

    const std::string name = StringUtil::UnicodeEscape(nameInfo.ins_->GetId(0));
    bool innerReg = info.IsInnerReg();
    if (!innerReg && (info.function_->propertyTable_.find(name) != info.function_->propertyTable_.end())) {
        info.function_->propertyTable_.at(name)->defineInsList_.emplace_back(info);
        return;
    }

    auto property = std::make_shared<Property>(this->program_, name);
    property->defineInsList_.push_back(info);
    property->nameInfo_ = nameInfo;
    if (!innerReg) {
        property->scope_ = this->scope_;
        property->export_ = this->export_;
        property->Create();

        LOG(INFO, PANDAGUARD) << TAG << "find property:" << property->name_;

        info.function_->propertyTable_.emplace(name, property);
    } else {
        property->scope_ = FUNCTION;
        property->export_ = false;
        property->Create();

        LOG(INFO, PANDAGUARD) << TAG << "find variable property:" << property->name_;

        info.function_->variableProperties_.push_back(property);
    }
}

void panda::guard::Node::CreateObjectDecoratorProperty(const InstructionInfo &info)
{
    if (info.notEqualToOpcode(pandasm::Opcode::CALLTHIS2) && info.notEqualToOpcode(pandasm::Opcode::CALLTHIS3)) {
        return;
    }

    std::string callName = GraphAnalyzer::GetCallName(info);
    if (callName != OBJECT_PROPERTY_GETOWNPROPERTYDESCRIPTOR && callName != OBJECT_PROPERTY_DEFINEPROPERTY) {
        return;
    }

    InstructionInfo objectParam;
    GraphAnalyzer::GetCallTryLdGlobalByNameParam(info, INDEX_0, objectParam);
    if (!objectParam.IsValid() || objectParam.ins_->GetId(0) != OBJECT_PROPERTY_OBJECT) {
        return;
    }

    InstructionInfo param1;
    GraphAnalyzer::GetCallLdObjByNameParam(info, INDEX_1, param1);
    if (!param1.IsValid() || param1.ins_->GetId(0) != OBJECT_PROPERTY_PROTOTYPE) {
        return;
    }

    InstructionInfo param2;
    GraphAnalyzer::GetCallLdaStrParam(info, INDEX_2, param2);
    if (!param2.IsValid()) {
        return;
    }

    const std::string name = StringUtil::UnicodeEscape(param2.ins_->GetId(0));
    auto property = std::make_shared<Property>(this->program_, name);
    property->scope_ = this->scope_;
    property->export_ = this->export_;
    property->defineInsList_.push_back(param2);
    property->nameInfo_ = param2;
    property->Create();
    LOG(INFO, PANDAGUARD) << TAG << "find object decorator property:" << property->name_;
    info.function_->objectDecoratorProperties_.push_back(property);
}

void panda::guard::Node::CreateClass(const InstructionInfo &info, Scope scope)
{
    if (info.notEqualToOpcode(pandasm::Opcode::DEFINECLASSWITHBUFFER) &&
        info.notEqualToOpcode(pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS)) {
        return;
    }

    const std::string idx = info.ins_->GetId(1);
    if (this->classTable_.find(idx) != this->classTable_.end()) {
        this->classTable_.at(idx)->defineInsList_.push_back(info);
        return;
    }

    auto clazz = std::make_shared<Class>(this->program_, info.ins_->GetId(0));
    clazz->node_ = this;
    clazz->moduleRecord_ = &this->moduleRecord_;
    clazz->literalArrayIdx_ = idx;
    clazz->defineInsList_.push_back(info);
    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        clazz->component_ = GraphAnalyzer::IsComponentClass(info);
    }
    clazz->scope_ = scope;
    if (info.ins_->opcode == pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS) {
        clazz->callRunTimeInst_ = true;
    }
    clazz->Create();

    clazz->EnumerateMethodIns([&](const InstructionInfo &insInfo) -> void { EnumerateIns(insInfo, FUNCTION); });

    this->classTable_.emplace(clazz->literalArrayIdx_, clazz);
}

void panda::guard::Node::CreateOuterMethod(const InstructionInfo &info)
{
    if (info.notEqualToOpcode(pandasm::Opcode::DEFINEMETHOD)) {
        return;
    }

    InstructionInfo defineInsInfo;
    InstructionInfo nameInsInfo;
    GraphAnalyzer::HandleDefineMethod(info, defineInsInfo, nameInsInfo);
    PANDA_GUARD_ASSERT_PRINT(!defineInsInfo.IsValid(), TAG, ErrorCode::GENERIC_ERROR, "defineInsInfo is invalid");
    // nameInsInfo maybe empty, therefore, there not check nameInsInfo. Instead, it is checked in actual use

    const std::string methodIdx = info.ins_->GetId(0);
    std::string literalArrayIdx;
    if (defineInsInfo.equalToOpcode(pandasm::Opcode::DEFINECLASSWITHBUFFER) ||
        defineInsInfo.equalToOpcode(pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS)) {
        literalArrayIdx = defineInsInfo.ins_->GetId(1);
    } else {  // createobjectwithbuffer
        literalArrayIdx = defineInsInfo.ins_->GetId(0);
    }

    const auto outerMethod = std::make_shared<OuterMethod>(this->program_, methodIdx);
    outerMethod->node_ = this;
    outerMethod->defineInsList_.push_back(info);
    GetMethodNameInfo(nameInsInfo, outerMethod->nameInfo_);
    outerMethod->Init();

    if (this->classTable_.find(literalArrayIdx) != this->classTable_.end()) {
        const auto &clazz = this->classTable_.at(literalArrayIdx);
        outerMethod->className_ = clazz->name_;
        outerMethod->export_ = clazz->export_;
        outerMethod->scope_ = clazz->scope_;
        outerMethod->component_ = clazz->component_;

        outerMethod->Create();

        LOG(INFO, PANDAGUARD) << TAG << "found method:" << methodIdx << " for class:" << clazz->name_;
        clazz->outerMethods_.push_back(outerMethod);
    } else if (this->objectTable_.find(literalArrayIdx) != this->objectTable_.end()) {
        const auto &obj = this->objectTable_.at(literalArrayIdx);
        outerMethod->export_ = obj->export_;
        outerMethod->scope_ = obj->scope_;

        outerMethod->Create();

        LOG(INFO, PANDAGUARD) << TAG << "found method:" << methodIdx << " for obj:" << obj->literalArrayIdx_;
        obj->outerMethods_.push_back(outerMethod);
    } else {
        PANDA_GUARD_ABORT_PRINT(TAG, ErrorCode::GENERIC_ERROR, "unexpect outer method for:" << literalArrayIdx);
    }

    outerMethod->EnumerateIns([&](const InstructionInfo &insInfo) -> void { EnumerateIns(insInfo, FUNCTION); });
}

void panda::guard::Node::CreateObject(const InstructionInfo &info, Scope scope)
{
    if (info.notEqualToOpcode(pandasm::Opcode::CREATEOBJECTWITHBUFFER)) {
        return;
    }

    const std::string idx = info.ins_->GetId(0);
    if (this->objectTable_.find(idx) != this->objectTable_.end()) {
        this->objectTable_.at(idx)->defineInsList_.push_back(info);
        return;
    }

    auto object = std::make_shared<Object>(this->program_, idx, this->name_);
    LOG(INFO, PANDAGUARD) << TAG << "found record object:" << object->literalArrayIdx_;
    object->node_ = this;
    object->defineInsList_.push_back(info);
    object->scope_ = scope;
    object->Create();

    object->EnumerateMethods([&](Function &function) -> void {
        function.EnumerateIns([&](const InstructionInfo &insInfo) -> void { EnumerateIns(insInfo, FUNCTION); });
    });

    this->objectTable_.emplace(object->literalArrayIdx_, object);
}

void panda::guard::Node::CreateObjectOuterProperty(const panda::guard::InstructionInfo &info)
{
    if (info.notEqualToOpcode(pandasm::Opcode::DEFINEPROPERTYBYNAME)) {
        return;
    }

    InstructionInfo defineIns;
    GraphAnalyzer::HandleDefineProperty(info, defineIns);
    if (!defineIns.IsValid()) {
        return;
    }

    PANDA_GUARD_ASSERT_PRINT(defineIns.notEqualToOpcode(pandasm::Opcode::CREATEOBJECTWITHBUFFER), TAG,
                             ErrorCode::GENERIC_ERROR, "unexpect related define ins");

    const std::string literalArrayIdx = defineIns.ins_->GetId(0);
    PANDA_GUARD_ASSERT_PRINT(this->objectTable_.find(literalArrayIdx) == this->objectTable_.end(), TAG,
                             ErrorCode::GENERIC_ERROR, "no record object for literalArrayIdx:" << literalArrayIdx);

    const auto &object = this->objectTable_.at(literalArrayIdx);
    const auto property = std::make_shared<Property>(this->program_, info.ins_->GetId(0));
    property->defineInsList_.push_back(info);
    property->nameInfo_ = info;
    property->export_ = object->export_;
    property->scope_ = object->scope_;

    property->Create();

    object->outerProperties_.push_back(property);
    LOG(INFO, PANDAGUARD) << TAG << "found object outer property:" << property->name_;
}

void panda::guard::Node::AddNameForExportObject(const InstructionInfo &info)
{
    if (info.notEqualToOpcode(pandasm::Opcode::STMODULEVAR) &&
        info.notEqualToOpcode(pandasm::Opcode::WIDE_STMODULEVAR)) {
        return;
    }

    InstructionInfo defineIns;
    GraphAnalyzer::GetStModuleVarDefineIns(info, defineIns);
    if (!defineIns.IsValid()) {
        return;
    }

    if (defineIns.notEqualToOpcode(pandasm::Opcode::CREATEOBJECTWITHBUFFER)) {
        return;
    }

    const int64_t index = std::get<int64_t>(info.ins_->GetImm(0));
    PANDA_GUARD_ASSERT_PRINT(index < 0 || index > MAX_EXPORT_ITEM_LEN, TAG, ErrorCode::GENERIC_ERROR,
                             "unexpect export item index:" << index);
    const auto &exportName = this->moduleRecord_.GetLocalExportName(index);

    const auto &objectIdx = defineIns.ins_->GetId(0);
    PANDA_GUARD_ASSERT_PRINT(this->objectTable_.find(objectIdx) == this->objectTable_.end(), TAG,
                             ErrorCode::GENERIC_ERROR, "invalid objectIdx:" << objectIdx);
    const auto &object = this->objectTable_.at(objectIdx);

    object->SetExportName(exportName);
    object->SetExportAndRefreshNeedUpdate(true);

    LOG(INFO, PANDAGUARD) << TAG << "add export name:" << exportName << " for objectIdx:" << objectIdx;
}

void panda::guard::Node::UpdateExportForNamespaceMember(const InstructionInfo &info) const
{
    if (info.notEqualToOpcode(pandasm::Opcode::STOBJBYNAME) ||
        (info.function_->type_ != FunctionType::NAMESPACE_FUNCTION) ||
        ((info.ins_->GetReg(0) - info.function_->regsNum_) != NAMESPACE_OWN_PARAM_REG_INDEX)) {
        return;
    }

    const auto propertyName = info.ins_->GetId(0);
    PANDA_GUARD_ASSERT_PRINT(info.function_->propertyTable_.find(propertyName) == info.function_->propertyTable_.end(),
                             TAG, ErrorCode::GENERIC_ERROR, "invalid propertyName:" << propertyName);
    const auto &property = info.function_->propertyTable_.at(propertyName);
    property->SetExportAndRefreshNeedUpdate(true);

    InstructionInfo defineIns;
    GraphAnalyzer::GetStObjByNameDefineIns(info, defineIns);

    if (!defineIns.IsValid()) {
        return;
    }

    if (defineIns.equalToOpcode(pandasm::Opcode::DEFINEFUNC)) {
        UpdateEntityNamespaceMemberExport(defineIns.ins_->GetId(0), this->functionTable_);
    }

    if (defineIns.equalToOpcode(pandasm::Opcode::DEFINECLASSWITHBUFFER) ||
        defineIns.equalToOpcode(pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS)) {
        UpdateEntityNamespaceMemberExport(defineIns.ins_->GetId(1), this->classTable_);
    }

    if (defineIns.equalToOpcode(pandasm::Opcode::CREATEOBJECTWITHBUFFER)) {
        UpdateEntityNamespaceMemberExport(defineIns.ins_->GetId(0), this->objectTable_);
    }
}

void panda::guard::Node::CreateArray(const InstructionInfo &info)
{
    if (info.notEqualToOpcode(pandasm::Opcode::CREATEARRAYWITHBUFFER)) {
        return;
    }

    LOG(INFO, PANDAGUARD) << TAG << "found array:" << info.ins_->GetId(0);
    auto array = std::make_shared<Array>(this->program_);
    array->node_ = this;
    array->nameInfo_ = info;
    array->Create();

    this->arrays_.emplace_back(array);
}

void panda::guard::Node::FindStLexVarName(const InstructionInfo &info)
{
    if (info.notEqualToOpcode(pandasm::Opcode::STLEXVAR)) {
        return;
    }

    InstructionInfo outInfo;
    GraphAnalyzer::GetLdaStr(info, outInfo);
    if (!outInfo.IsValid()) {
        return;
    }

    LOG(INFO, PANDAGUARD) << TAG << "found stlexvar name:" << outInfo.ins_->GetId(0);
    GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(outInfo.ins_->GetId(0));
}

void panda::guard::Node::CreateUiDecorator(const InstructionInfo &info, Scope scope)
{
    if (!UiDecorator::IsUiDecoratorIns(info, scope)) {
        return;
    }

    auto decorator = std::make_shared<UiDecorator>(this->program_, this->objectTable_);
    decorator->scope_ = scope;
    decorator->export_ = false;
    decorator->baseInst_ = info;
    decorator->Create();
    if (!decorator->IsValidUiDecoratorType()) {
        return;
    }
    this->uiDecorator_.emplace_back(decorator);
}

void panda::guard::Node::CreateFilePath()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->IsFileNameObfEnabled() || options->IsReservedRemoteHarPkgNames(this->pkgName_) ||
        options->IsInRecordNameWhiteList(this->name_)) {
        this->filepath_.name = this->name_;
        return;
    }

    if (this->isNormalizedOhmUrl_) {
        CreateFilePathForNormalizedMode();
    } else {
        CreateFilePathForDefaultMode();
    }
}

void panda::guard::Node::CreateFilePathForDefaultMode()
{
    if (IsRemoteHar(this->name_)) {
        if (this->type_ == NodeType::JSON_FILE) {
            this->filepath_.name = this->name_;
            return;
        }
        std::string prefix = pkgName_ + PATH_DELIMITER.data();
        PANDA_GUARD_ASSERT_PRINT(!StringUtil::IsPrefixMatched(name_, prefix), TAG, ErrorCode::GENERIC_ERROR,
                                 "invalid remote har prefix");
        filepath_.prePart = std::move(prefix);

        filepath_.name = name_.substr(filepath_.prePart.size(), name_.size() - filepath_.prePart.size());
        return;
    }

    // format: bundleName/hapPkgName@pkgName/filepath
    size_t startPos = name_.find_first_of(PATH_DELIMITER.data(), 0);
    if (startPos == std::string::npos) {
        filepath_.name = name_;
        return;
    }

    std::string toFound = pkgName_ + PATH_DELIMITER.data();
    size_t foundPos = name_.find(toFound, startPos);
    if (foundPos == std::string::npos) {
        filepath_.name = name_;
        return;
    }

    foundPos += toFound.size();
    filepath_.prePart = name_.substr(0, foundPos);
    filepath_.name = name_.substr(foundPos, name_.size() - foundPos);
}

void panda::guard::Node::CreateFilePathForNormalizedMode()
{
    // [<bundle name>?]&<package name>/<file path>&[<version>?]
    const size_t startPos = name_.find_first_of(NORMALIZED_OHM_DELIMITER.data(), 0);
    const std::string prefix = this->type_ == NodeType::SOURCE_FILE
                                   ? NORMALIZED_OHM_DELIMITER.data() + pkgName_ + PATH_DELIMITER.data()
                                   : NORMALIZED_OHM_DELIMITER.data();

    PANDA_GUARD_ASSERT_PRINT(!StringUtil::IsPrefixMatched(name_, prefix, startPos), TAG, ErrorCode::GENERIC_ERROR,
                             "invalid normalizedOhmUrl prefix");
    size_t prefixEnd = startPos + prefix.size();
    filepath_.prePart = name_.substr(0, prefixEnd);

    size_t filePathEnd = name_.find_first_of(NORMALIZED_OHM_DELIMITER.data(), prefixEnd);
    PANDA_GUARD_ASSERT_PRINT(filePathEnd == std::string::npos, TAG, ErrorCode::GENERIC_ERROR,
                             "invalid normalizedOhmUrl format");
    filepath_.name = name_.substr(prefixEnd, filePathEnd - prefixEnd);

    filepath_.postPart = name_.substr(filePathEnd, name_.size() - filePathEnd);
}

void panda::guard::Node::ExtractNames()
{
    moduleRecord_.ExtractNames(this->strings_);

    for (const auto &[_, function] : this->functionTable_) {
        function->ExtractNames(this->strings_);
    }

    for (const auto &[_, clazz] : this->classTable_) {
        clazz->ExtractNames(this->strings_);
    }

    for (const auto &[_, object] : this->objectTable_) {
        object->ExtractNames(this->strings_);
    }

    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        for (const auto &decorator : this->uiDecorator_) {
            decorator->ExtractNames(this->strings_);
        }
    }

    auto parts = StringUtil::Split(filepath_.name, PATH_DELIMITER.data());
    for (const auto &part : parts) {
        this->strings_.emplace(part);
    }

    GuardContext::GetInstance()->GetNameMapping()->AddReservedNames(this->strings_);

    LOG(INFO, PANDAGUARD) << TAG << "strings:";
    for (const auto &str : this->strings_) {
        LOG(INFO, PANDAGUARD) << TAG << str;
    }
}

void panda::guard::Node::RefreshNeedUpdate()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    this->fileNameNeedUpdate_ = options->IsFileNameObfEnabled() && !options->IsInRecordNameWhiteList(this->name_);
    if (this->fileNameNeedUpdate_) {
        this->fileNameNeedUpdate_ = options->IsUseNormalizedOhmUrl() ?
            !options->IsReservedRemoteHarPkgNames(pkgName_) :
            !IsRemoteHar(this->name_);
    }

    if (options->IsKeepPath(this->name_)) {
        LOG(INFO, PANDAGUARD) << TAG << "found keep rule for:" << this->name_;
        this->contentNeedUpdate_ = false;
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->strings_);
    }

    for (auto &[_, object] : this->objectTable_) {
        object->SetContentNeedUpdate(this->contentNeedUpdate_);
    }

    this->needUpdate_ = this->fileNameNeedUpdate_ || this->contentNeedUpdate_;
}

void panda::guard::Node::EnumerateFunctions(const std::function<FunctionTraver> &callback)
{
    for (auto &[_, function] : this->functionTable_) {
        callback(*function);
    }

    for (auto &[_, clazz] : this->classTable_) {
        clazz->EnumerateFunctions(callback);
    }

    for (auto &[_, object] : this->objectTable_) {
        object->EnumerateMethods(callback);
    }
}

void panda::guard::Node::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "node update for " << this->name_ << " start";

    if (this->fileNameNeedUpdate_) {
        this->UpdateFileNameDefine();
    }

    // fileNameNeedUpdate_ || contentNeedUpdate_
    for (auto &[_, function] : this->functionTable_) {
        function->Obfuscate();
        function->WriteNameCache(this->sourceName_);
    }

    for (auto &[_, clazz] : this->classTable_) {
        clazz->Obfuscate();
        clazz->WriteNameCache(this->sourceName_);
    }

    for (auto &[_, object] : this->objectTable_) {
        object->Obfuscate();
        object->WriteNameCache(this->sourceName_);
    }

    for (const auto &annotation : this->annotations_) {
        annotation->Obfuscate();
        annotation->WriteNameCache(this->sourceName_);
    }

    if (GuardContext::GetInstance()->GetGuardOptions()->IsDecoratorObfEnabled()) {
        for (const auto &decorator : this->uiDecorator_) {
            decorator->Obfuscate();
            decorator->WriteNameCache(this->sourceName_);
        }
    }

    for (const auto &array : this->arrays_) {
        array->Obfuscate();
    }

    if (this->contentNeedUpdate_) {
        moduleRecord_.Obfuscate();
        moduleRecord_.WriteNameCache(this->sourceName_);
    }

    this->UpdateScopeNames();
    this->UpdateFieldsLiteralArrayIdx();

    this->WriteFileCache(this->sourceName_);

    LOG(INFO, PANDAGUARD) << TAG << "node update for " << this->name_ << " end";
}

void panda::guard::Node::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfName(filePath, this->obfSourceName_);
    GuardContext::GetInstance()->GetNameCache()->AddSourceFile(filePath, this->sourceFile_, this->obfSourceFile_);
}

void panda::guard::Node::UpdateRecordTable()
{
    auto entry = this->program_->prog_->record_table.extract(this->name_);
    entry.key() = this->obfName_;
    entry.mapped().name = this->obfName_;
    if (!entry.mapped().source_file.empty()) {
        this->UpdateSourceFile(entry.mapped().source_file);
        entry.mapped().source_file = this->obfSourceFile_;
    }
    this->program_->prog_->record_table.insert(std::move(entry));
}

void panda::guard::Node::UpdateFileNameDefine()
{
    filepath_.obfName = GuardContext::GetInstance()->GetNameMapping()->GetFilePath(filepath_.name);
    obfName_ = filepath_.prePart + filepath_.obfName + filepath_.postPart;
    UpdateRecordTable();

    /* e.g.
     *  sourceName_: entry/src/main/ets/entryability/EntryAbility.ets
     *  filepath_.name: src/main/ets/entryability/EntryAbility
     *  sourceNamePrePart: entry/
     *  sourceNamePostPart: .ets
     */
    const auto &[sourceNamePrePart, sourceNamePostPart] = StringUtil::RSplitOnce(this->sourceName_, filepath_.name);
    if (sourceNamePrePart.empty() && sourceNamePostPart.empty()) {
        this->obfSourceName_ = obfName_;
    } else {
        this->obfSourceName_ = sourceNamePrePart + filepath_.obfName + sourceNamePostPart;
    }
}

void panda::guard::Node::UpdateFileNameReferences()
{
    moduleRecord_.UpdateFileNameReferences();
}

void panda::guard::Node::UpdateSourceFile(const std::string &file)
{
    PANDA_GUARD_ASSERT_PRINT(file != this->sourceFile_, TAG, ErrorCode::GENERIC_ERROR, "duplicate source file" << file);
    if (this->sourceFileUpdated_) {
        return;
    }

    this->sourceFile_ = file;

    /* e.g.
     *  file: entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ets
     *  filepath_.name: src/main/ets/entryability/EntryAbility
     *  prefix: entry|entry|1.0.0|
     *  suffix: .ets
     */
    const auto &[prefix, suffix] = StringUtil::RSplitOnce(file, filepath_.name);
    PANDA_GUARD_ASSERT_PRINT(file != filepath_.name && prefix.empty() && suffix.empty(), TAG, ErrorCode::GENERIC_ERROR,
                             "invalid source file" << file << ",record: " << this->name_);
    this->obfSourceFile_ = prefix + filepath_.obfName + suffix;
    this->sourceFileUpdated_ = true;

    LOG(INFO, PANDAGUARD) << TAG << "source_file: " << this->sourceFile_;
}

void panda::guard::Node::UpdateScopeNames() const
{
    LOG(INFO, PANDAGUARD) << TAG << "update scopeNames for:" << this->name_;
    auto &record = this->GetRecord();
    for (auto &it : record.field_list) {
        if (it.name == SCOPE_NAMES_FIELD) {
            const auto &literalArrayIdx = it.metadata->GetValue()->GetValue<std::string>();
            LOG(INFO, PANDAGUARD) << TAG << "scopeNames literalArrayIdx:" << literalArrayIdx;
            auto &literalArray = this->program_->prog_->literalarray_table.at(literalArrayIdx);
            UpdateScopeNamesLiteralArray(literalArray);
            break;
        }
    }
}

void panda::guard::Node::UpdateFieldsLiteralArrayIdx()
{
    if (this->name_ == this->obfName_) {
        return;
    }

    LOG(INFO, PANDAGUARD) << "update fields literalArrayIdx for:" << this->name_;
    auto &record = this->GetRecord();
    for (auto &it : record.field_list) {
        if (it.name == SCOPE_NAMES_FIELD || it.name == MODULE_RECORD_IDX_FIELD) {
            const auto &literalArrayIdx = it.metadata->GetValue()->GetValue<std::string>();
            LOG(INFO, PANDAGUARD) << TAG << "literalArrayIdx:" << literalArrayIdx;

            std::string updatedLiteralArrayIdx = literalArrayIdx;
            updatedLiteralArrayIdx.replace(updatedLiteralArrayIdx.find(this->name_), this->name_.size(),
                                           this->obfName_);
            LOG(INFO, PANDAGUARD) << TAG << "updated literalArrayIdx:" << updatedLiteralArrayIdx;

            UpdateLiteralArrayTableIdx(literalArrayIdx, updatedLiteralArrayIdx);

            it.metadata->SetValue(
                pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(updatedLiteralArrayIdx));

            if (it.name == MODULE_RECORD_IDX_FIELD) {
                this->moduleRecord_.UpdateLiteralArrayIdx(updatedLiteralArrayIdx);
            }
        }
    }
}

void panda::guard::Node::WriteNameCache()
{
    this->WriteFileCache(this->sourceName_);
}

void panda::guard::Node::GetMethodNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo)
{
    if (!info.IsValid()) {
        return;
    }

    if (InOpcodeList(info, METHOD_NAME_DIRECT_LIST)) {
        nameInfo = info;
    } else if (InOpcodeList(info, METHOD_NAME_INDIRECT_LIST)) {
        GraphAnalyzer::GetLdaStr(info, nameInfo);
    }
}
