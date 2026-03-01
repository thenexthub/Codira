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

#include "module_record.h"

#include "configs/guard_context.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view TAG = "[Module_Record]";
constexpr std::string_view MODULE_RECORD_IDX = "moduleRecordIdx";
constexpr std::string_view SCOPE_DELIMITER = "#";
constexpr std::string_view PATH_DELIMITER = "/";

uint16_t GetMethodAffiliateValueByOffset(const std::vector<panda::pandasm::LiteralArray::Literal> &literals,
                                         uint32_t &offset)
{
    PANDA_GUARD_ASSERT_PRINT(offset >= literals.size(), TAG, panda::guard::ErrorCode::GENERIC_ERROR, "offset overflow");
    PANDA_GUARD_ASSERT_PRINT(literals[offset].tag_ != panda::panda_file::LiteralTag::METHODAFFILIATE, TAG,
                             panda::guard::ErrorCode::GENERIC_ERROR, "not method affiliate value");
    return std::get<uint16_t>(literals[offset++].value_);
}

uint32_t GetIntegerValueByOffset(const std::vector<panda::pandasm::LiteralArray::Literal> &literals, uint32_t &offset)
{
    PANDA_GUARD_ASSERT_PRINT(offset >= literals.size(), TAG, panda::guard::ErrorCode::GENERIC_ERROR, "offset overflow");
    PANDA_GUARD_ASSERT_PRINT(!literals[offset].IsIntegerValue(), TAG, panda::guard::ErrorCode::GENERIC_ERROR,
                             "not integer value");
    return std::get<uint32_t>(literals[offset++].value_);
}

std::string GetStringValueByOffset(const std::vector<panda::pandasm::LiteralArray::Literal> &literals, uint32_t &offset)
{
    PANDA_GUARD_ASSERT_PRINT(offset >= literals.size(), TAG, panda::guard::ErrorCode::GENERIC_ERROR, "offset overflow");
    PANDA_GUARD_ASSERT_PRINT(!literals[offset].IsStringValue(), TAG, panda::guard::ErrorCode::GENERIC_ERROR,
                             "not string value");
    return std::get<std::string>(literals[offset++].value_);
}
}  // namespace

void panda::guard::FilePathItem::Update()
{
    this->refFilePath_.Update();
    if (this->refFilePath_.filePath_ == this->refFilePath_.obfFilePath_) {
        return;
    }

    auto &literalArrayTable = program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);
    it->second.literals_[this->refFilePath_.filePathIndex_].value_ = this->refFilePath_.obfFilePath_;
}

void panda::guard::FilePathItem::ExtractNames(std::set<std::string> &strings) const
{
    auto parts = StringUtil::Split(this->refFilePath_.GetRawPath(), PATH_DELIMITER.data());
    for (const auto &part : parts) {
        strings.emplace(part);
    }
}

void panda::guard::FilePathItem::RefreshNeedUpdate()
{
    this->needUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsFileNameObfEnabled() &&
        this->refFilePath_.pathType_ != FilePathType::EXTERNAL_DEPENDENCE &&
        !GuardContext::GetInstance()->GetGuardOptions()->IsInRecordNameWhiteList(this->refFilePath_.rawName_);
    if (!this->needUpdate_) {
        auto parts = StringUtil::Split(this->refFilePath_.GetRawPath(), PATH_DELIMITER.data());
        for (const auto &part : parts) {
            GuardContext::GetInstance()->GetNameMapping()->AddFileNameMapping(part);
        }
    }
}

void panda::guard::FilePathItem::Build()
{
    this->refFilePath_.Init();
}

void panda::guard::RegularImportItem::Update()
{
    auto &literalArrayTable = this->program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);

    this->obfLocalName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->localName_);
    it->second.literals_[this->localNameIndex_].value_ = this->obfLocalName_;

    this->obfImportName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->importName_);
    it->second.literals_[this->importNameIndex_].value_ = this->obfImportName_;
}

void panda::guard::RegularImportItem::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->localName_);
    strings.emplace(this->importName_);
}

void panda::guard::RegularImportItem::RefreshNeedUpdate()
{
    this->needUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsExportObfEnabled() && !remoteFile_;
    if (!this->needUpdate_) {
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->localName_);
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->importName_);
    }
}

void panda::guard::RegularImportItem::WriteFileCache(const std::string &filePath)
{
    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    if (this->localName_ != this->obfLocalName_) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->localName_, this->obfLocalName_);
    }
    if ((this->importName_ != this->localName_) && (this->importName_ != this->obfImportName_)) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->importName_, this->obfImportName_);
    }
}

void panda::guard::RegularImportItem::WritePropertyCache()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->IsPropertyObfEnabled() && !options->IsExportObfEnabled()) {
        return;
    }

    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    nameCache->AddObfPropertyName(this->localName_, this->obfLocalName_);
    nameCache->AddObfPropertyName(this->importName_, this->obfImportName_);
}

void panda::guard::NameSpaceImportItem::Update()
{
    auto &literalArrayTable = this->program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);

    this->obfLocalName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->localName_);
    it->second.literals_[this->localNameIndex_].value_ = this->obfLocalName_;
}

void panda::guard::NameSpaceImportItem::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->localName_);
}

void panda::guard::NameSpaceImportItem::RefreshNeedUpdate()
{
    this->needUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsToplevelObfEnabled() && !remoteFile_;
    if (!this->needUpdate_) {
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->localName_);
    }
}

void panda::guard::NameSpaceImportItem::WriteFileCache(const std::string &filePath)
{
    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    if (this->localName_ != this->obfLocalName_) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->localName_, this->obfLocalName_);
    }
}

void panda::guard::NameSpaceImportItem::WritePropertyCache()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->IsPropertyObfEnabled() && !options->IsExportObfEnabled()) {
        return;
    }
    GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(this->localName_, this->obfLocalName_);
}

void panda::guard::LocalExportItem::Update()
{
    auto &literalArrayTable = this->program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);

    if (GuardContext::GetInstance()->GetGuardOptions()->IsExportObfEnabled()) {
        this->obfLocalName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->localName_);
        it->second.literals_[this->localNameIndex_].value_ = this->obfLocalName_;

        this->obfExportName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->exportName_);
        it->second.literals_[this->exportNameIndex_].value_ = this->obfExportName_;
        return;
    }

    if (this->localName_ != this->exportName_) {
        this->obfLocalName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->localName_);
        it->second.literals_[this->localNameIndex_].value_ = this->obfLocalName_;
    }
}

void panda::guard::LocalExportItem::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->localName_);
    strings.emplace(this->exportName_);
}

void panda::guard::LocalExportItem::RefreshNeedUpdate()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->IsToplevelObfEnabled()) {
        this->needUpdate_ = false;
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->localName_);
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->exportName_);
        return;
    }
    if (options->IsExportObfEnabled()) {
        this->needUpdate_ = true;
        return;
    }
    if (this->localName_ != this->exportName_) {
        this->needUpdate_ = true;
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->exportName_);
        return;
    }
    this->needUpdate_ = false;
    GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->localName_);
    GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->exportName_);
}

void panda::guard::LocalExportItem::WriteFileCache(const std::string &filePath)
{
    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    if (this->localName_ != this->obfLocalName_) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->localName_, this->obfLocalName_);
    }
    if ((this->exportName_ != this->localName_) && (this->exportName_ != this->obfExportName_)) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->exportName_, this->obfExportName_);
    }
}

void panda::guard::LocalExportItem::WritePropertyCache()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->IsPropertyObfEnabled() && !options->IsExportObfEnabled()) {
        return;
    }

    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    nameCache->AddObfPropertyName(this->localName_, this->obfLocalName_);
    nameCache->AddObfPropertyName(this->exportName_, this->obfExportName_);
}

void panda::guard::IndirectExportItem::Update()
{
    auto &literalArrayTable = this->program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);

    this->obfImportName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->importName_);
    it->second.literals_[this->importNameIndex_].value_ = this->obfImportName_;

    this->obfExportName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->exportName_);
    it->second.literals_[this->exportNameIndex_].value_ = this->obfExportName_;
}

void panda::guard::IndirectExportItem::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->importName_);
    strings.emplace(this->exportName_);
}

void panda::guard::IndirectExportItem::RefreshNeedUpdate()
{
    this->needUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsExportObfEnabled() && !remoteFile_;
    if (!this->needUpdate_) {
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->importName_);
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->exportName_);
    }
}

void panda::guard::IndirectExportItem::WriteFileCache(const std::string &filePath)
{
    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    if (this->importName_ != this->obfImportName_) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->importName_, this->obfImportName_);
    }
    if ((this->exportName_ != this->importName_) && (this->exportName_ != this->obfExportName_)) {
        nameCache->AddObfIdentifierName(filePath, SCOPE_DELIMITER.data() + this->exportName_, this->obfExportName_);
    }
}

void panda::guard::IndirectExportItem::WritePropertyCache()
{
    const auto &options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->IsPropertyObfEnabled() && !options->IsExportObfEnabled()) {
        return;
    }

    auto nameCache = GuardContext::GetInstance()->GetNameCache();
    nameCache->AddObfPropertyName(this->importName_, this->obfImportName_);
    nameCache->AddObfPropertyName(this->exportName_, this->obfExportName_);
}

void panda::guard::ModuleRecord::Build()
{
    auto recordItem = program_->prog_->record_table.find(this->name_);
    PANDA_GUARD_ASSERT_PRINT(recordItem == program_->prog_->record_table.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "name:" << this->name_ << "not find in record_table");

    for (const auto &field : recordItem->second.field_list) {
        if (field.name == MODULE_RECORD_IDX) {
            const auto &value = field.metadata->GetValue();
            this->literalArrayIdx_ = value->GetValue<std::string>();
            break;
        }
    }

    if (this->literalArrayIdx_.empty()) {
        // commonJs module don't have MODULE_RECORD_IDX
        LOG(INFO, PANDAGUARD) << TAG << "no MODULE_RECORD_IDX in:" << this->name_;
        return;
    }

    auto &literalArray = program_->prog_->literalarray_table.at(literalArrayIdx_);
    CreateModuleVar(literalArray);
}

void panda::guard::ModuleRecord::ExtractNames(std::set<std::string> &strings) const
{
    for (const auto &item : this->filePathList_) {
        item.ExtractNames(strings);
    }

    for (const auto &item : this->regularImportList_) {
        item.ExtractNames(strings);
    }

    for (const auto &item : this->nameSpaceImportList_) {
        item.ExtractNames(strings);
    }

    for (const auto &item : this->localExportList_) {
        item.ExtractNames(strings);
    }

    for (const auto &item : this->indirectExportList_) {
        item.ExtractNames(strings);
    }
}

bool panda::guard::ModuleRecord::IsExportVar(const std::string &var)
{
    bool exportVar = std::any_of(localExportList_.begin(), localExportList_.end(), [&](const LocalExportItem &item) {
        return (item.localName_ == var) || (item.exportName_ == var);
    });
    if (exportVar) {
        return true;
    }
    return std::any_of(indirectExportList_.begin(), indirectExportList_.end(), [&](const IndirectExportItem &item) {
        return (item.importName_ == var) || (item.exportName_ == var);
    });
}

void panda::guard::ModuleRecord::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "update for " << this->literalArrayIdx_ << " start";

    for (auto &item : this->regularImportList_) {
        item.Obfuscate();
    }

    for (auto &item : this->nameSpaceImportList_) {
        item.Obfuscate();
    }

    for (auto &item : this->localExportList_) {
        item.Obfuscate();
    }

    for (auto &item : this->indirectExportList_) {
        item.Obfuscate();
    }
    LOG(INFO, PANDAGUARD) << TAG << "update for " << this->literalArrayIdx_ << " end";
}

void panda::guard::ModuleRecord::WriteNameCache(const std::string &filePath)
{
    if (!this->obfuscated_) {
        return;
    }

    for (auto &item : this->filePathList_) {
        item.WriteNameCache(filePath);
    }

    for (auto &item : this->regularImportList_) {
        item.WriteNameCache(filePath);
    }

    for (auto &item : this->nameSpaceImportList_) {
        item.WriteNameCache(filePath);
    }

    for (auto &item : this->localExportList_) {
        item.WriteNameCache(filePath);
    }

    for (auto &item : this->indirectExportList_) {
        item.WriteNameCache(filePath);
    }
}

void panda::guard::ModuleRecord::CreateModuleVar(const pandasm::LiteralArray &literalArray)
{
    uint32_t offset = 0;
    CreateFilePathList(literalArray.literals_, offset);
    CreateRegularImportList(literalArray.literals_, offset);
    CreateNameSpaceImportList(literalArray.literals_, offset);
    CreateLocalExportList(literalArray.literals_, offset);
    CreateIndirectExportList(literalArray.literals_, offset);

    Print();
}

void panda::guard::ModuleRecord::CreateFilePathList(const std::vector<pandasm::LiteralArray::Literal> &literals,
                                                    uint32_t &offset)
{
    uint32_t num = GetIntegerValueByOffset(literals, offset);
    for (uint32_t idx = 0; idx < num; idx++) {
        FilePathItem item(this->program_, this->literalArrayIdx_);
        item.refFilePath_.filePathIndex_ = offset;
        item.refFilePath_.SetFilePath(GetStringValueByOffset(literals, offset));
        item.Create();
        this->filePathList_.emplace_back(item);
    }
}

void panda::guard::ModuleRecord::CreateRegularImportList(const std::vector<pandasm::LiteralArray::Literal> &literals,
                                                         uint32_t &offset)
{
    uint32_t num = GetIntegerValueByOffset(literals, offset);
    for (uint32_t idx = 0; idx < num; idx++) {
        RegularImportItem item(this->program_, this->literalArrayIdx_);
        item.localNameIndex_ = offset;
        item.localName_ = GetStringValueByOffset(literals, offset);
        item.obfLocalName_ = item.localName_;
        item.importNameIndex_ = offset;
        item.importName_ = GetStringValueByOffset(literals, offset);
        item.obfImportName_ = item.importName_;
        uint16_t filePathItemIndex = GetMethodAffiliateValueByOffset(literals, offset);
        PANDA_GUARD_ASSERT_PRINT(filePathItemIndex >= this->filePathList_.size(), TAG, ErrorCode::GENERIC_ERROR,
                                 "filePathItem index overflow");
        item.remoteFile_ = this->filePathList_[filePathItemIndex].refFilePath_.isRemoteFile_;
        this->regularImportList_.emplace_back(item);
    }
}

void panda::guard::ModuleRecord::CreateNameSpaceImportList(const std::vector<pandasm::LiteralArray::Literal> &literals,
                                                           uint32_t &offset)
{
    uint32_t num = GetIntegerValueByOffset(literals, offset);
    for (uint32_t idx = 0; idx < num; idx++) {
        NameSpaceImportItem item(this->program_, this->literalArrayIdx_);
        item.localNameIndex_ = offset;
        item.localName_ = GetStringValueByOffset(literals, offset);
        item.obfLocalName_ = item.localName_;
        uint16_t filePathItemIndex = GetMethodAffiliateValueByOffset(literals, offset);
        PANDA_GUARD_ASSERT_PRINT(filePathItemIndex >= this->filePathList_.size(), TAG, ErrorCode::GENERIC_ERROR,
                                 "filePathItem index overflow");
        item.remoteFile_ = this->filePathList_[filePathItemIndex].refFilePath_.isRemoteFile_;
        this->nameSpaceImportList_.emplace_back(item);
    }
}

void panda::guard::ModuleRecord::CreateLocalExportList(const std::vector<pandasm::LiteralArray::Literal> &literals,
                                                       uint32_t &offset)
{
    uint32_t num = GetIntegerValueByOffset(literals, offset);
    for (uint32_t idx = 0; idx < num; idx++) {
        LocalExportItem item(this->program_, this->literalArrayIdx_);
        item.localNameIndex_ = offset;
        item.localName_ = GetStringValueByOffset(literals, offset);
        item.obfLocalName_ = item.localName_;
        item.exportNameIndex_ = offset;
        item.exportName_ = GetStringValueByOffset(literals, offset);
        item.obfExportName_ = item.exportName_;
        this->localExportList_.emplace_back(item);
    }
}

void panda::guard::ModuleRecord::CreateIndirectExportList(const std::vector<pandasm::LiteralArray::Literal> &literals,
                                                          uint32_t &offset)
{
    uint32_t num = GetIntegerValueByOffset(literals, offset);
    for (uint32_t idx = 0; idx < num; idx++) {
        IndirectExportItem item(this->program_, this->literalArrayIdx_);
        item.exportNameIndex_ = offset;
        item.exportName_ = GetStringValueByOffset(literals, offset);
        item.obfExportName_ = item.exportName_;
        item.importNameIndex_ = offset;
        item.importName_ = GetStringValueByOffset(literals, offset);
        item.obfImportName_ = item.importName_;
        uint16_t filePathItemIndex = GetMethodAffiliateValueByOffset(literals, offset);
        PANDA_GUARD_ASSERT_PRINT(filePathItemIndex >= this->filePathList_.size(), TAG, ErrorCode::GENERIC_ERROR,
                                 "filePathItem index overflow");
        item.remoteFile_ = this->filePathList_[filePathItemIndex].refFilePath_.isRemoteFile_;
        this->indirectExportList_.emplace_back(item);
    }
}

void panda::guard::ModuleRecord::Print()
{
    LOG(INFO, PANDAGUARD) << TAG << "literalArrayIdx_:" << this->literalArrayIdx_;

    for (const auto &item : this->filePathList_) {
        LOG(INFO, PANDAGUARD) << TAG << "filePathList file:" << item.refFilePath_.filePath_;
    }

    for (const auto &item : this->regularImportList_) {
        LOG(INFO, PANDAGUARD) << TAG << "regularImport local name:" << item.localName_;
        LOG(INFO, PANDAGUARD) << TAG << "regularImport import name:" << item.importName_;
    }

    for (const auto &item : this->nameSpaceImportList_) {
        LOG(INFO, PANDAGUARD) << TAG << "NameSpaceImport local name:" << item.localName_;
    }

    for (const auto &item : this->localExportList_) {
        LOG(INFO, PANDAGUARD) << TAG << "localExport local name:" << item.localName_;
        LOG(INFO, PANDAGUARD) << TAG << "localExport export name:" << item.exportName_;
    }

    for (const auto &item : this->indirectExportList_) {
        LOG(INFO, PANDAGUARD) << TAG << "indirectExport local name:" << item.importName_;
        LOG(INFO, PANDAGUARD) << TAG << "indirectExport export name:" << item.exportName_;
    }
}

void panda::guard::ModuleRecord::RefreshNeedUpdate()
{
    for (auto &item : this->regularImportList_) {
        item.RefreshNeedUpdate();
    }

    for (auto &item : this->nameSpaceImportList_) {
        item.RefreshNeedUpdate();
    }

    for (auto &item : this->localExportList_) {
        item.RefreshNeedUpdate();
    }

    for (auto &item : this->indirectExportList_) {
        item.RefreshNeedUpdate();
    }
}

std::string panda::guard::ModuleRecord::GetLocalExportName(uint32_t index)
{
    PANDA_GUARD_ASSERT_PRINT(index >= this->localExportList_.size(), TAG, ErrorCode::GENERIC_ERROR, "index is invalid");
    return this->localExportList_[index].exportName_;
}

void panda::guard::ModuleRecord::UpdateFileNameReferences()
{
    LOG(INFO, PANDAGUARD) << TAG << "update FileNameReferences for " << this->literalArrayIdx_ << " start";
    for (auto &item : this->filePathList_) {
        item.Obfuscate();
    }
    LOG(INFO, PANDAGUARD) << TAG << "update FileNameReferences for " << this->literalArrayIdx_ << " start";
}

void panda::guard::ModuleRecord::UpdateLiteralArrayIdx(const std::string &literalArrayIdx)
{
    this->literalArrayIdx_ = literalArrayIdx;

    for (auto &item : this->filePathList_) {
        item.literalArrayIdx_ = literalArrayIdx;
    }

    for (auto &item : this->regularImportList_) {
        item.literalArrayIdx_ = literalArrayIdx;
    }

    for (auto &item : this->nameSpaceImportList_) {
        item.literalArrayIdx_ = literalArrayIdx;
    }

    for (auto &item : this->localExportList_) {
        item.literalArrayIdx_ = literalArrayIdx;
    }

    for (auto &item : this->indirectExportList_) {
        item.literalArrayIdx_ = literalArrayIdx;
    }
}
