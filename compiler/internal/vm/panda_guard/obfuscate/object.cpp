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

#include "object.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view TAG = "[Object]";
constexpr size_t LITERAL_OBJECT_ITEM_LEN = 2;
constexpr size_t LITERAL_OBJECT_COMMON_ITEM_GROUP_LEN = 4;  // Every 4 elements represent a set of regular key values
constexpr size_t LITERAL_OBJECT_METHOD_ITEM_GROUP_LEN = 6;  // Every 6 elements represent a set of regular key values
}  // namespace

void panda::guard::ObjectProperty::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
}

void panda::guard::ObjectProperty::Build()
{
    if (this->method_ == nullptr) {
        return;
    }

    this->method_->nameNeedUpdate_ = this->needUpdate_;
    this->method_->Init();
    this->method_->Create();
}

void panda::guard::ObjectProperty::Update()
{
    auto &literalArrayTable = this->program_->prog_->literalarray_table;
    PANDA_GUARD_ASSERT_PRINT(literalArrayTable.find(this->literalArrayIdx_) == literalArrayTable.end(), TAG,
                             ErrorCode::GENERIC_ERROR, "get bad literalArrayIdx:" << this->literalArrayIdx_);

    auto &literalArray = literalArrayTable.at(this->literalArrayIdx_);
    if (this->contentNeedUpdate_) {
        this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << this->obfName_;
        literalArray.literals_[this->index_].value_ = this->obfName_;
    }

    if (this->method_) {
        this->method_->Obfuscate();
        literalArray.literals_[this->index_ + LITERAL_OBJECT_ITEM_LEN].value_ = this->method_->obfIdx_;
    }
}

void panda::guard::ObjectProperty::SetContentNeedUpdate(bool toUpdate)
{
    this->contentNeedUpdate_ = toUpdate && this->needUpdate_;
    this->needUpdate_ = true;

    if (this->method_) {
        this->method_->SetContentNeedUpdate(toUpdate);
    }
}

void panda::guard::ObjectProperty::SetExportAndRefreshNeedUpdate(bool isExport)
{
    if (this->method_) {
        this->method_->SetExportAndRefreshNeedUpdate(isExport);
    }

    Entity::SetExportAndRefreshNeedUpdate(isExport);
}

void panda::guard::Object::Build()
{
    LOG(INFO, PANDAGUARD) << TAG << "object create for " << this->literalArrayIdx_ << " start";

    const auto &literalArray = this->program_->prog_->literalarray_table.at(this->literalArrayIdx_);
    size_t keyIndex = 1;                                     // object item key index
    size_t valueIndex = keyIndex + LITERAL_OBJECT_ITEM_LEN;  // object item value index
    while (valueIndex < literalArray.literals_.size()) {
        auto &valueLiteral = literalArray.literals_[valueIndex];
        const bool isMethod = valueLiteral.tag_ == panda_file::LiteralTag::METHOD;
        CreateProperty(literalArray, keyIndex, isMethod);
        if (isMethod) {
            keyIndex += LITERAL_OBJECT_METHOD_ITEM_GROUP_LEN;
        } else {
            keyIndex += LITERAL_OBJECT_COMMON_ITEM_GROUP_LEN;
        }
        valueIndex = keyIndex + LITERAL_OBJECT_ITEM_LEN;
    }
    LOG(INFO, PANDAGUARD) << TAG << "object create for " << this->literalArrayIdx_ << " end";
}

void panda::guard::Object::CreateProperty(const pandasm::LiteralArray &literalArray, size_t index, bool isMethod)
{
    const auto &[keyTag, keyValue] = literalArray.literals_[index];
    PANDA_GUARD_ASSERT_PRINT(keyTag != panda_file::LiteralTag::STRING, TAG, ErrorCode::GENERIC_ERROR,
                             "bad keyTag literal tag");

    const auto property = std::make_shared<ObjectProperty>(this->program_, this->literalArrayIdx_);
    property->name_ = StringUtil::UnicodeEscape(std::get<std::string>(keyValue));
    property->scope_ = this->scope_;
    property->export_ = this->export_;
    property->index_ = index;

    if (isMethod) {
        const size_t valueLiteralIndex = index + 2;
        PANDA_GUARD_ASSERT_PRINT(valueLiteralIndex >= literalArray.literals_.size(), TAG, ErrorCode::GENERIC_ERROR,
                                 "bad valueLiteralIndex:" << valueLiteralIndex);
        const auto &[valueTag, valueValue] = literalArray.literals_[valueLiteralIndex];
        PANDA_GUARD_ASSERT_PRINT(valueTag != panda_file::LiteralTag::METHOD, TAG, ErrorCode::GENERIC_ERROR,
                                 "bad valueLiteral tag:" << (int)valueTag);
        property->method_ = std::make_shared<PropertyMethod>(this->program_, std::get<std::string>(valueValue));
        property->method_->node_ = this->node_;
        property->method_->export_ = this->export_;
        property->method_->scope_ = this->scope_;
    }

    property->Create();

    LOG(INFO, PANDAGUARD) << TAG << "find object property:" << property->name_;

    this->properties_.push_back(property);
}

void panda::guard::Object::UpdateLiteralArrayIdx()
{
    if (!GuardContext::GetInstance()->GetGuardOptions()->IsFileNameObfEnabled()) {
        return;
    }

    const auto &it = this->program_->nodeTable_.find(this->recordName_);
    PANDA_GUARD_ASSERT_PRINT(it == this->program_->nodeTable_.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "not find node: " + this->recordName_);
    const auto &node = it->second;
    if (node->name_ == node->obfName_) {
        return;
    }
    std::string updatedLiteralArrayIdx = this->literalArrayIdx_;
    updatedLiteralArrayIdx.replace(updatedLiteralArrayIdx.find(node->name_), node->name_.size(), node->obfName_);

    UpdateLiteralArrayTableIdx(this->literalArrayIdx_, updatedLiteralArrayIdx);

    this->literalArrayIdx_ = updatedLiteralArrayIdx;

    for (auto &inst : this->defineInsList_) {
        inst.ins_->GetId(INDEX_0) = updatedLiteralArrayIdx;
    }

    for (auto &property : this->properties_) {
        property->literalArrayIdx_ = updatedLiteralArrayIdx;
    }
}

void panda::guard::Object::EnumerateMethods(const std::function<FunctionTraver> &callback)
{
    for (const auto &property : this->properties_) {
        if (property->method_) {
            callback(property->method_.operator*());
        }
    }

    for (auto &method : this->outerMethods_) {
        callback(*method);
    }
}

void panda::guard::Object::ExtractNames(std::set<std::string> &strings) const
{
    for (const auto &property : this->properties_) {
        property->ExtractNames(strings);
    }

    for (const auto &property : this->outerProperties_) {
        property->ExtractNames(strings);
    }

    for (const auto &method : this->outerMethods_) {
        method->ExtractNames(strings);
    }
}

void panda::guard::Object::RefreshNeedUpdate()
{
    this->needUpdate_ = true;
    this->needUpdateName_ = TopLevelOptionEntity::NeedUpdate(*this);
}

void panda::guard::Object::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "object update for " << this->literalArrayIdx_ << " start";

    if (this->contentNeedUpdate_ && this->needUpdateName_ && !this->name_.empty()) {
        this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
    }

    UpdateLiteralArrayIdx();

    for (const auto &property : this->properties_) {
        property->Obfuscate();
    }

    for (const auto &property : this->outerProperties_) {
        property->Obfuscate();
    }

    for (const auto &method : this->outerMethods_) {
        method->Obfuscate();
    }

    LOG(INFO, PANDAGUARD) << TAG << "object update for " << this->literalArrayIdx_ << " end";
}

void panda::guard::Object::WriteNameCache(const std::string &filePath)
{
    if (!this->obfuscated_ || !this->contentNeedUpdate_) {
        return;
    }

    if (this->needUpdateName_ && !this->obfName_.empty()) {
        GuardContext::GetInstance()->GetNameCache()->AddObfIdentifierName(filePath, this->GetNameCacheScope(),
                                                                          this->obfName_);
    }

    for (const auto &property : this->properties_) {
        property->WriteNameCache(filePath);
    }

    for (const auto &property : this->outerProperties_) {
        property->WriteNameCache(filePath);
    }
}

void panda::guard::Object::SetContentNeedUpdate(bool toUpdate)
{
    this->contentNeedUpdate_ = toUpdate;
    for (const auto &property : this->properties_) {
        property->SetContentNeedUpdate(toUpdate);
    }
    for (const auto &method : this->outerMethods_) {
        method->SetContentNeedUpdate(toUpdate);
    }
}

void panda::guard::Object::SetExportAndRefreshNeedUpdate(bool isExport)
{
    for (const auto &property : this->properties_) {
        property->SetExportAndRefreshNeedUpdate(isExport);
    }

    for (const auto &property : this->outerProperties_) {
        property->SetExportAndRefreshNeedUpdate(isExport);
    }

    for (const auto &method : this->outerMethods_) {
        method->SetExportAndRefreshNeedUpdate(isExport);
    }

    Entity::SetExportAndRefreshNeedUpdate(isExport);
}

void panda::guard::Object::SetExportName(const std::string &exportName)
{
    this->name_ = exportName;
    this->obfName_ = exportName;
    this->SetNameCacheScope(exportName);
}
