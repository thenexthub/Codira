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

#include "abc_module_array_processor.h"

namespace panda::abc2program {

AbcModuleArrayProcessor::AbcModuleArrayProcessor(panda_file::File::EntityId entity_id,
                                                 Abc2ProgramEntityContainer &entity_container)
    : AbcFileEntityProcessor(entity_id, entity_container)
{
    module_data_accessor_ = std::make_unique<panda_file::ModuleDataAccessor>(*file_, entity_id_);
}

void AbcModuleArrayProcessor::FillProgramData()
{
    const std::vector<uint32_t> &request_modules_offset = module_data_accessor_->getRequestModules();
    std::vector<panda::pandasm::LiteralArray::Literal> literal_vec;
    FillModuleRequests(literal_vec, request_modules_offset);
    uint32_t regular_import_num = 0;
    uint32_t namespace_import_num = 0;
    uint32_t local_export_num = 0;
    uint32_t indirect_export_num = 0;
    uint32_t star_export_num = 0;
    module_data_accessor_->EnumerateModuleRecord([&](panda_file::ModuleTag tag, uint32_t export_name_offset,
        uint32_t request_module_idx, uint32_t import_name_offset, uint32_t local_name_offset) {
        switch (tag) {
            case panda_file::ModuleTag::REGULAR_IMPORT:
                FillRegularImportEntry(literal_vec, request_module_idx, import_name_offset, local_name_offset);
                ++regular_import_num;
                break;
            case panda_file::ModuleTag::NAMESPACE_IMPORT:
                FillNamespaceImportEntry(literal_vec, request_module_idx, local_name_offset);
                ++namespace_import_num;
                break;
            case panda_file::ModuleTag::LOCAL_EXPORT:
                FillLocalExportEntry(literal_vec, export_name_offset, local_name_offset);
                ++local_export_num;
                break;
            case panda_file::ModuleTag::INDIRECT_EXPORT:
                FillIndirectExportEntry(literal_vec, export_name_offset, request_module_idx, import_name_offset);
                ++indirect_export_num;
                break;
            case panda_file::ModuleTag::STAR_EXPORT:
                FillStarExportEntry(literal_vec, request_module_idx);
                ++star_export_num;
                break;
            default: {
                UNREACHABLE();
            }
        }
    });
    std::vector<uint32_t> num_vec = {request_modules_offset.size(), regular_import_num, namespace_import_num,
        local_export_num, indirect_export_num, star_export_num};
    FillEntrySize(literal_vec, num_vec);
    auto module_array = panda::pandasm::LiteralArray(std::move(literal_vec));
    program_->literalarray_table.emplace(entity_container_.GetLiteralArrayIdName(entity_id_.GetOffset()),
                                         std::move(module_array));
}

void AbcModuleArrayProcessor::FillModuleRequests(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    const std::vector<uint32_t> &request_modules_offset)
{
    panda::pandasm::LiteralArray::Literal module_size = {
        .tag_ = panda::panda_file::LiteralTag::INTEGER,
        .value_ = static_cast<uint32_t>(request_modules_offset.size())
    };
    literal_vec.emplace_back(module_size);
    for (auto &request : request_modules_offset) {
        panda::pandasm::LiteralArray::Literal module_request = {
            .tag_ = panda::panda_file::LiteralTag::STRING,
            .value_ = GetStringById(panda_file::File::EntityId{request})
        };
        literal_vec.emplace_back(module_request);
    }
}

void AbcModuleArrayProcessor::FillRegularImportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    uint32_t &request_module_idx, uint32_t &import_name_offset, uint32_t &local_name_offset)
{
    panda::pandasm::LiteralArray::Literal local_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{local_name_offset})
    };
    literal_vec.emplace_back(local_name);
    panda::pandasm::LiteralArray::Literal import_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{import_name_offset})
    };
    literal_vec.emplace_back(import_name);
    panda::pandasm::LiteralArray::Literal module_request = {
        .tag_ = panda::panda_file::LiteralTag::METHODAFFILIATE,
        .value_ = static_cast<uint16_t>(request_module_idx)
    };
    literal_vec.emplace_back(module_request);
}

void AbcModuleArrayProcessor::FillNamespaceImportEntry(
    std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    uint32_t &request_module_idx, uint32_t &local_name_offset)
{
    panda::pandasm::LiteralArray::Literal local_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{local_name_offset})
    };
    literal_vec.emplace_back(local_name);
    panda::pandasm::LiteralArray::Literal module_request = {
        .tag_ = panda::panda_file::LiteralTag::METHODAFFILIATE,
        .value_ = static_cast<uint16_t>(request_module_idx)
    };
    literal_vec.emplace_back(module_request);
}

void AbcModuleArrayProcessor::FillLocalExportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    uint32_t export_name_offset, uint32_t &local_name_offset)
{
    panda::pandasm::LiteralArray::Literal local_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{local_name_offset})
    };
    literal_vec.emplace_back(local_name);
    panda::pandasm::LiteralArray::Literal export_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{export_name_offset})
    };
    literal_vec.emplace_back(export_name);
}

void AbcModuleArrayProcessor::FillIndirectExportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    uint32_t export_name_offset, uint32_t request_module_idx, uint32_t import_name_offset)
{
    panda::pandasm::LiteralArray::Literal export_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{export_name_offset})
    };
    literal_vec.emplace_back(export_name);
    panda::pandasm::LiteralArray::Literal import_name = {
        .tag_ = panda::panda_file::LiteralTag::STRING,
        .value_ = GetStringById(panda_file::File::EntityId{import_name_offset})
    };
    literal_vec.emplace_back(import_name);
    panda::pandasm::LiteralArray::Literal module_request = {
        .tag_ = panda::panda_file::LiteralTag::METHODAFFILIATE,
        .value_ = static_cast<uint16_t>(request_module_idx)
    };
    literal_vec.emplace_back(module_request);
}

void AbcModuleArrayProcessor::FillStarExportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    uint32_t request_module_idx)
{
    panda::pandasm::LiteralArray::Literal module_request = {
        .tag_ = panda::panda_file::LiteralTag::METHODAFFILIATE,
        .value_ = static_cast<uint16_t>(request_module_idx)
    };
    literal_vec.emplace_back(module_request);
}

// Add the literal of the size of various attributes in the module literal array.
void AbcModuleArrayProcessor::FillEntrySize(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
    const std::vector<uint32_t> &num_vec)
{
    // Insert literals of regular_import_num, namespace_import_num, local_export_num,
    // indirect_export_num, star_export_num to literal_vec.
    uint32_t idx_num = num_vec[0] + 1;
    for (size_t index = 1; index < num_vec.size(); ++index) {
        auto num = num_vec[index];
        panda::pandasm::LiteralArray::Literal entry_size = {
            .tag_ = panda::panda_file::LiteralTag::INTEGER,
            .value_ = static_cast<uint32_t>(num)
        };
        literal_vec.insert(literal_vec.begin() + idx_num, entry_size);
        // '1' stands for each entry_size, which has been inserted into literal vec
        idx_num += 1;
        ASSERT(index - 1 < sizeof(LITERAL_NUMS)/sizeof(LITERAL_NUMS[0]));
        // skip total num of module literals for each module kind
        idx_num += LITERAL_NUMS[index-1] * num;
    }
}
}  // namespace panda::abc2program