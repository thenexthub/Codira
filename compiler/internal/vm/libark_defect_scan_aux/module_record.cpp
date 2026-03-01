/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "module_record.h"
#include <algorithm>
#include "libpandabase/macros.h"

namespace panda::defect_scan_aux {
void ModuleRecord::SetModuleFilename(std::string_view module_filename)
{
    module_filename_ = module_filename;
}

void ModuleRecord::SetRequestModules(std::vector<std::string> &request_modules)
{
    request_modules_ = request_modules;
}

void ModuleRecord::AddImportEntry(const ImportEntry &import_entry)
{
    import_entries_.push_back(import_entry);
}

void ModuleRecord::AddExportEntry(const ExportEntry &export_entry)
{
    export_entries_.push_back(export_entry);
}

void ModuleRecord::SetRegularImportNum(size_t regular_import_num)
{
    regular_import_num_ = regular_import_num;
}

void ModuleRecord::SetLocalExportNum(size_t local_export_num)
{
    local_export_num_ = local_export_num;
}

size_t ModuleRecord::GetRegularImportNum() const
{
    return regular_import_num_;
}

size_t ModuleRecord::GetLocalExportNum() const
{
    return local_export_num_;
}

const std::string &ModuleRecord::GetImportLocalNameByIndex(size_t index) const
{
    ASSERT(index < regular_import_num_);
    return import_entries_[index].local_name_;
}

const std::string &ModuleRecord::GetImportNamespaceNameByIndex(size_t index) const
{
    ASSERT(index < request_modules_.size());
    const std::string &module_name = request_modules_[index];
    auto iter = std::find_if(import_entries_.begin() + regular_import_num_, import_entries_.end(),
                             [&](const ImportEntry &entry) { return entry.module_request_ == module_name; });
    ASSERT(iter != import_entries_.end());
    return iter->local_name_;
}

const std::string &ModuleRecord::GetExportNameByIndex(size_t index) const
{
    ASSERT(index < local_export_num_);
    return export_entries_[index].export_name_;
}

std::string ModuleRecord::GetLocalNameByExportName(std::string_view export_name) const
{
    for (auto &export_entry : export_entries_) {
        if (export_entry.export_name_ == export_name) {
            return export_entry.local_name_;
        }
    }
    return "";
}

std::string ModuleRecord::GetImportNameByExportName(std::string_view export_name) const
{
    for (auto &export_entry : export_entries_) {
        if (export_entry.export_name_ == export_name) {
            return export_entry.import_name_;
        }
    }
    return "";
}

std::string ModuleRecord::GetModuleNameByExportName(std::string_view export_name) const
{
    for (auto &export_entry : export_entries_) {
        if (export_entry.export_name_ == export_name) {
            return export_entry.module_request_;
        }
    }
    return "";
}

std::string ModuleRecord::GetModuleNameByLocalName(std::string_view local_name) const
{
    for (auto &import_entry : import_entries_) {
        if (import_entry.local_name_ == local_name) {
            return import_entry.module_request_;
        }
    }
    return "";
}

std::string ModuleRecord::GetImportNameByLocalName(std::string_view local_name) const
{
    for (auto &import_entry : import_entries_) {
        if (import_entry.local_name_ == local_name) {
            return import_entry.import_name_;
        }
    }
    return "";
}
}  // namespace panda::defect_scan_aux