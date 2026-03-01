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

#include "abc2program_entity_container.h"
#include "abc_file_utils.h"
#include "file-inl.h"
#include "method_data_accessor-inl.h"

namespace panda::abc2program {

static constexpr char NORMALIZED_OHMURL_DELIM = '&';
static constexpr size_t OHMURL_DELIM_OFFSET = 1U;

const panda_file::File &Abc2ProgramEntityContainer::GetAbcFile() const
{
    return file_;
}

pandasm::Program &Abc2ProgramEntityContainer::GetProgram() const
{
    return program_;
}

const panda_file::DebugInfoExtractor &Abc2ProgramEntityContainer::GetDebugInfoExtractor() const
{
    return debug_info_extractor_;
}

std::string Abc2ProgramEntityContainer::GetStringById(const panda_file::File::EntityId &entity_id) const
{
    panda_file::File::StringData sd = file_.GetStringData(entity_id);
    return (reinterpret_cast<const char *>(sd.data));
}

std::string Abc2ProgramEntityContainer::GetFullRecordNameById(const panda_file::File::EntityId &class_id)
{
    uint32_t class_id_offset = class_id.GetOffset();
    auto it = record_full_name_map_.find(class_id_offset);
    if (it != record_full_name_map_.end()) {
        return it->second;
    }
    std::string name = GetStringById(class_id);
    pandasm::Type type = pandasm::Type::FromDescriptor(name);
    std::string record_full_name = type.GetName();
    ModifyRecordName(record_full_name);
    record_full_name_map_.emplace(class_id_offset, record_full_name);
    return record_full_name;
}

/**
 * Support the inter-app hsp dependents bytecode har.
 * The record name format: <bundleName>&<normalizedImportPath>&<version>
 * The ohmurl specs that must have bundleName in inter-app package. The records need compile into the inter-app
 * hsp package. So the recordNames need to add bundleName in front when the abc-file as input for the
 * inter-app hsp package.
 */
void Abc2ProgramEntityContainer::ModifyRecordName(std::string &record_name)
{
    if (!modify_pkg_name_.empty()) {
        ModifyPkgNameForRecordName(record_name);
    }
    if (!origin_version_.empty() && !target_version_.empty() && IsSourceFileRecord(record_name) &&
        GetPkgNameFromRecordName(record_name) == package_name_) {
        ModifyVersionForRecordName(record_name);
        return;
    }

    if (bundle_name_.empty()) {
        return;
    }
    if (IsSourceFileRecord(record_name)) {
        record_name = bundle_name_ + record_name;
    }
}

/**
 * Modify the version segment embedded in a record name's ohmurl.
 *
 * This function updates the version information embedded in the record name string
 * when it matches a given origin version. The record name format typically looks like:
 *   - ".record &library/static-import-test&1.0.0"
 *   - ".record &library/annotation-test&1.0.0.Anno"
 *
 * The string uses '&' as delimiters:
 *   - The first '&' separates ".record" and the package path.
 *   - The second '&' separates the package path and the version information.
 *
 * The function locates the version segment after the second '&', checks if it starts
 * with the expected origin version (`origin_version_`), and if so, replaces it
 * with the target version (`target_version_`), preserving any suffix after the version.
 */
void Abc2ProgramEntityContainer::ModifyVersionForRecordName(std::string &record_name)
{
    // Version part appears after the second '&', e.g.:
    // ".record &library/annotation-test&1.0.0.Anno"
    // So we must start parsing from the second '&' to locate the version segment.
    size_t firstPos = record_name.find(NORMALIZED_OHMURL_DELIM);
    if (firstPos == std::string::npos) {
        return;
    }

    size_t secondPos = record_name.find(NORMALIZED_OHMURL_DELIM, firstPos + OHMURL_DELIM_OFFSET);
    if (secondPos == std::string::npos) {
        return;
    }

    // Replace version prefix only when it matches origin_version_,
    // e.g. "1.0.0.Anno" → "2.0.0.Anno", keeping the suffix intact.
    std::string currentVersionWithSuffix = record_name.substr(secondPos + OHMURL_DELIM_OFFSET);
    if (currentVersionWithSuffix.rfind(origin_version_, 0) == 0) {
        std::string suffix = currentVersionWithSuffix.substr(origin_version_.size());
        record_name.replace(secondPos + OHMURL_DELIM_OFFSET, std::string::npos, target_version_ + suffix);
    }
}

void Abc2ProgramEntityContainer::ModifyPkgNameForRecordName(std::string &record_name)
{
    std::vector<std::string> pkg_names = Split(modify_pkg_name_, COLON_SEPARATOR);
    std::string orginal = NORMALIZED_OHMURL_SEPARATOR + pkg_names[ORIGINAL_PKG_NAME_POS];
    std::string target = NORMALIZED_OHMURL_SEPARATOR + pkg_names[TARGET_PKG_NAME_POS];
    std::string pkg_name = GetPkgNameFromRecordName(record_name);
    if (pkg_name == pkg_names[ORIGINAL_PKG_NAME_POS]) {
        record_name.replace(0, orginal.length(), target);
    }
}

void Abc2ProgramEntityContainer::ModifyPkgNameForFieldName(std::string &field_name)
{
    if (modify_pkg_name_.empty()) {
        return;
    }
    std::vector<std::string> pkg_names = Split(modify_pkg_name_, COLON_SEPARATOR);
    std::string orginal = FIELD_NAME_PREFIX + pkg_names[ORIGINAL_PKG_NAME_POS];
    if (field_name == orginal) {
        field_name = FIELD_NAME_PREFIX + pkg_names[TARGET_PKG_NAME_POS];
    }
}

bool Abc2ProgramEntityContainer::IsSourceFileRecord(const std::string& record_name)
{
    return record_name.find(NORMALIZED_OHMURL_SEPARATOR) == 0;
}

std::string Abc2ProgramEntityContainer::GetFullMethodNameById(const panda_file::File::EntityId &method_id)
{
    auto method_id_offset = method_id.GetOffset();
    auto it = method_full_name_map_.find(method_id_offset);
    if (it != method_full_name_map_.end()) {
        return it->second;
    }
    std::string full_method_name = ConcatFullMethodNameById(method_id);
    method_full_name_map_.emplace(method_id_offset, full_method_name);
    return full_method_name;
}

std::string Abc2ProgramEntityContainer::ConcatFullMethodNameById(const panda_file::File::EntityId &method_id)
{
    panda::panda_file::MethodDataAccessor method_data_accessor(file_, method_id);
    std::string method_name_raw = GetStringById(method_data_accessor.GetNameId());
    std::string record_name = GetFullRecordNameById(method_data_accessor.GetClassId());
    std::stringstream ss;
    if (AbcFileUtils::IsSystemTypeName(record_name)) {
        ss << DOT;
    } else {
        ss << record_name << DOT;
    }
    ss << method_name_raw;
    return ss.str();
}

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetMouleLiteralArrayIdSet() const
{
    return module_literal_array_id_set_;
}

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetModuleRequestPhaseIdSet() const
{
    return module_request_phase_id_set_;
}

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetUnnestedLiteralArrayIdSet() const
{
    return unnested_literal_array_id_set_;
}

std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetUnprocessedNestedLiteralArrayIdSet()
{
    return unprocessed_nested_literal_array_id_set_;
}

void Abc2ProgramEntityContainer::AddModuleLiteralArrayId(uint32_t module_literal_array_id)
{
    module_literal_array_id_set_.insert(module_literal_array_id);
}


void Abc2ProgramEntityContainer::AddModuleRequestPhaseId(uint32_t module_request_phase_id)
{
    module_request_phase_id_set_.insert(module_request_phase_id);
}

void Abc2ProgramEntityContainer::AddUnnestedLiteralArrayId(uint32_t literal_array_id)
{
    unnested_literal_array_id_set_.insert(literal_array_id);
}

void Abc2ProgramEntityContainer::AddProcessedNestedLiteralArrayId(uint32_t nested_literal_array_id)
{
    processed_nested_literal_array_id_set_.insert(nested_literal_array_id);
}

void Abc2ProgramEntityContainer::TryAddUnprocessedNestedLiteralArrayId(uint32_t nested_literal_array_id)
{
    if (unnested_literal_array_id_set_.count(nested_literal_array_id)) {
        return;
    }
    if (processed_nested_literal_array_id_set_.count(nested_literal_array_id)) {
        return;
    }
    unprocessed_nested_literal_array_id_set_.insert(nested_literal_array_id);
}

std::string Abc2ProgramEntityContainer::GetLiteralArrayIdName(uint32_t literal_array_id)
{
    std::stringstream name;
    auto cur_record_name = GetFullRecordNameById(panda_file::File::EntityId(current_class_id_));
    name << cur_record_name << UNDERLINE << literal_array_id;
    return name.str();
}

void Abc2ProgramEntityContainer::AddProgramString(const std::string &str) const
{
    program_.strings.insert(str);
}

}  // namespace panda::abc2program