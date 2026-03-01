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

#ifndef LIBPANDAFILE_MODULE_DATA_ACCESSOR_INL_H
#define LIBPANDAFILE_MODULE_DATA_ACCESSOR_INL_H

#include "module_data_accessor.h"
#include "file_items.h"
#include "helpers.h"

namespace panda::panda_file {
template <class Callback>
inline void ModuleDataAccessor::EnumerateModuleRecord(const Callback &cb)
{
    auto sp = entry_data_sp_;

    auto regular_import_num = panda_file::helpers::Read<panda_file::ID_SIZE>(&sp);
    for (size_t idx = 0; idx < regular_import_num; idx++) {
        auto local_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        auto import_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        auto module_request_idx = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint16_t)>(&sp));
        cb(ModuleTag::REGULAR_IMPORT, 0, module_request_idx, import_name_offset, local_name_offset);
    }

    auto namespace_import_num = panda_file::helpers::Read<panda_file::ID_SIZE>(&sp);
    for (size_t idx = 0; idx < namespace_import_num; idx++) {
        auto local_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        auto module_request_idx = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint16_t)>(&sp));
        cb(ModuleTag::NAMESPACE_IMPORT, 0, module_request_idx, 0, local_name_offset);
    }

    auto local_export_num = panda_file::helpers::Read<panda_file::ID_SIZE>(&sp);
    for (size_t idx = 0; idx < local_export_num; idx++) {
        auto local_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        auto export_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        cb(ModuleTag::LOCAL_EXPORT, export_name_offset, 0, 0, local_name_offset);
    }

    auto indirect_export_num = panda_file::helpers::Read<panda_file::ID_SIZE>(&sp);
    for (size_t idx = 0; idx < indirect_export_num; idx++) {
        auto export_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        auto import_name_offset = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
        auto module_request_idx = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint16_t)>(&sp));
        cb(ModuleTag::INDIRECT_EXPORT, export_name_offset, module_request_idx, import_name_offset, 0);
    }

    auto starExportNum = panda_file::helpers::Read<panda_file::ID_SIZE>(&sp);
    for (size_t idx = 0; idx < starExportNum; idx++) {
        auto module_request_idx = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint16_t)>(&sp));
        cb(ModuleTag::STAR_EXPORT, 0, module_request_idx, 0, 0);
    }
}
}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_MODULE_DATA_ACCESSOR_INL_H