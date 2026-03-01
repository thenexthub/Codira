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

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_MODULE_RECORD_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_MODULE_RECORD_H

#include <string>
#include <vector>

namespace panda::defect_scan_aux {
struct ImportEntry {
    std::string module_request_;
    std::string import_name_;
    std::string local_name_;
};

struct ExportEntry {
    std::string export_name_;
    std::string module_request_;
    std::string import_name_;
    std::string local_name_;
};

class ModuleRecord {
public:
    explicit ModuleRecord(std::string_view module_filename) : module_filename_(module_filename) {}
    ~ModuleRecord() = default;

    void SetModuleFilename(std::string_view module_filename);
    void SetRequestModules(std::vector<std::string> &request_modules);
    void AddImportEntry(const ImportEntry &import_entry);
    void AddExportEntry(const ExportEntry &export_entry);
    void SetRegularImportNum(size_t regular_import_num);
    void SetLocalExportNum(size_t local_export_num);
    size_t GetRegularImportNum() const;
    size_t GetLocalExportNum() const;
    const std::string &GetImportLocalNameByIndex(size_t index) const;
    const std::string &GetImportNamespaceNameByIndex(size_t index) const;
    const std::string &GetExportNameByIndex(size_t index) const;
    std::string GetLocalNameByExportName(std::string_view export_name) const;
    std::string GetImportNameByExportName(std::string_view export_name) const;
    std::string GetModuleNameByExportName(std::string_view export_name) const;
    std::string GetModuleNameByLocalName(std::string_view local_name) const;
    std::string GetImportNameByLocalName(std::string_view local_name) const;

private:
    std::string module_filename_;
    size_t regular_import_num_ {0};
    size_t local_export_num_ {0};
    std::vector<std::string> request_modules_;
    std::vector<ImportEntry> import_entries_;
    std::vector<ExportEntry> export_entries_;
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_MODULE_RECORD_H