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

#include "bytecode_analysis_results.h"

namespace panda::bytecodeopt {

std::mutex BytecodeAnalysisResults::mutex_;
BytecodeAnalysisResultMap BytecodeAnalysisResults::analysis_results_;
BytecodeMapsMap BytecodeAnalysisResults::bytecode_maps_;

void BytecodeAnalysisResult::SetModuleConstantAnalysisResult(const ModuleConstantAnalysisResult &result)
{
    for (auto &iter : result) {
        uint32_t slot = iter.first;
        if (slot < local_export_slot_external_names_.size()) {
            const auto &external_names = local_export_slot_external_names_[slot];
            for (const auto &external_name_iter : external_names) {
                constant_local_export_values_.emplace(external_name_iter, *iter.second);
            }
        }
    }
}

void BytecodeAnalysisResult::SetConstantLocalExportSlots(const std::unordered_set<uint32_t> &slots)
{
    constant_local_export_slots_ = slots;
}

void BytecodeAnalysisResult::SetLocalExportInfo(uint32_t slot, const std::string &external_name)
{
    if (slot == local_export_slot_external_names_.size()) {
        auto &vec = local_export_slot_external_names_.emplace_back();
        vec.push_back(external_name);
    } else {
        ASSERT(slot < local_export_slot_external_names_.size());
        local_export_slot_external_names_[slot].push_back(external_name);
    }
}

void BytecodeAnalysisResult::SetRegularImportInfo(uint32_t slot,
                                                  const std::string &import_name,
                                                  const std::string &source_record)
{
    [[maybe_unused]] auto iter = regular_import_slot_infos_.find(slot);
    ASSERT(iter == regular_import_slot_infos_.end());
    RegularImportInfo info {import_name, source_record};
    regular_import_slot_infos_.emplace(slot, info);
}

void BytecodeAnalysisResult::SetNamespaceImportInfo(uint32_t slot, const std::string &source_record)
{
    namespace_import_slot_source_record_names_.emplace(slot, source_record);
}

bool BytecodeAnalysisResult::GetLocalExportInfo(uint32_t slot, uint32_t name_idx, std::string &external_name) const
{
    if (slot >= local_export_slot_external_names_.size()) {
        return false;
    }
    const auto &names = local_export_slot_external_names_[slot];
    if (name_idx > names.size()) {
        return false;
    }
    external_name = names[name_idx];
    return true;
}

bool BytecodeAnalysisResult::GetRegularImportInfo(uint32_t slot,
                                                  std::string &import_name,
                                                  std::string &source_record) const
{
    auto iter = regular_import_slot_infos_.find(slot);
    if (iter == regular_import_slot_infos_.end()) {
        return false;
    }
    import_name = iter->second.import_name;
    source_record = iter->second.source_record_name;
    return true;
}

bool BytecodeAnalysisResult::GetNamespaceImportInfo(uint32_t slot, std::string &source_record) const
{
    auto iter = namespace_import_slot_source_record_names_.find(slot);
    if (iter == namespace_import_slot_source_record_names_.end()) {
        return false;
    }
    source_record = iter->second;
    return true;
}

bool BytecodeAnalysisResult::GetExportedConstantValue(const std::string &name, ConstantValue &value) const
{
    auto iter = constant_local_export_values_.find(name);
    if (iter != constant_local_export_values_.end()) {
        value = iter->second;
        return true;
    }
    return false;
}

void BytecodeAnalysisResult::Dump(std::ostream &os)
{
    os << "------------------------------------" << std::endl;
    os << "Constant local export values: " << std::endl;
    for (auto iter : constant_local_export_values_) {
        os << iter.first << ": " << iter.second.ToString() << std::endl;
    }
    os << "Constant local export slots: " << std::endl;
    for (auto iter : constant_local_export_slots_) {
        os << iter << ", ";
    }
    os << std::endl;
    os << "Local export slot external names: " << std::endl;
    for (size_t i = 0; i < local_export_slot_external_names_.size(); i++) {
        os << i << ": ";
        for (auto iter : local_export_slot_external_names_[i]) {
            os << iter << ", ";
        }
        os << std::endl;
    }
    os << "Regular import slot infos: " << std::endl;
    for (auto iter : regular_import_slot_infos_) {
        os << iter.first << ": import_name: " << iter.second.import_name
           << ", source_record_name: " << iter.second.source_record_name << std::endl;
    }
    os << "Namespace import slot source record names: " << std::endl;
    for (auto iter : namespace_import_slot_source_record_names_) {
        os << iter.first << ": " << iter.second << std::endl;
    }
    os << "------------------------------------" << std::endl;
}

pandasm::AsmEmitter::PandaFileToPandaAsmMaps &BytecodeAnalysisResults::GetOrCreateBytecodeMaps(
    const std::string &filename, bool &exists)
{
    return GetOrCreateElementInMap<pandasm::AsmEmitter::PandaFileToPandaAsmMaps>(bytecode_maps_, filename, exists);
}

void BytecodeAnalysisResults::DeleteBytecodeMaps(const std::string &filename)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bytecode_maps_.erase(filename);
}

BytecodeAnalysisResult &BytecodeAnalysisResults::GetOrCreateBytecodeAnalysisResult(const std::string &recordname,
                                                                                   bool &exists)
{
    return GetOrCreateElementInMap<BytecodeAnalysisResult>(analysis_results_, recordname, exists);
}

bool BytecodeAnalysisResults::GetLocalExportConstForRecord(const std::string &recordname,
                                                           uint32_t local_export_slot,
                                                           ConstantValue &value)
{
    constexpr uint32_t DEFAULT_EXTERNAL_NAME_IDX = 0;
    auto iter = analysis_results_.find(recordname);
    if (iter == analysis_results_.end()) {
        return false;
    }
    auto &analysis_resut = iter->second;
    std::string external_name;
    if (!analysis_resut->GetLocalExportInfo(local_export_slot, DEFAULT_EXTERNAL_NAME_IDX, external_name)) {
        return false;
    }
    return analysis_resut->GetExportedConstantValue(external_name, value);
}

bool BytecodeAnalysisResults::GetRegularImportConstForRecord(const std::string &recordname,
                                                             uint32_t regular_import_slot,
                                                             ConstantValue &value)
{
    auto iter = analysis_results_.find(recordname);
    if (iter == analysis_results_.end()) {
        return false;
    }
    std::string import_name;
    std::string source_record;
    if (!iter->second->GetRegularImportInfo(regular_import_slot, import_name, source_record)) {
        return false;
    }

    iter = analysis_results_.find(source_record);
    if (iter == analysis_results_.end()) {
        return false;
    }
    return iter->second->GetExportedConstantValue(import_name, value);
}

bool BytecodeAnalysisResults::GetModuleNamespaceConstForRecord(const std::string &recordname,
                                                               uint32_t module_namespace_slot,
                                                               const std::string &property_name,
                                                               ConstantValue &value)
{
    auto iter = analysis_results_.find(recordname);
    if (iter == analysis_results_.end()) {
        return false;
    }
    std::string source_record;
    if (!iter->second->GetNamespaceImportInfo(module_namespace_slot, source_record)) {
        return false;
    }
    iter = analysis_results_.find(source_record);
    if (iter == analysis_results_.end()) {
        return false;
    }
    return iter->second->GetExportedConstantValue(property_name, value);
}

void BytecodeAnalysisResults::Clear()
{
    std::unique_lock<std::mutex> lock(mutex_);
    bytecode_maps_.clear();
    analysis_results_.clear();
}

}  // namespace panda::bytecodeopt