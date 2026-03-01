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

#ifndef ABC2PROGRAM_ABC_MODULE_ARRAY_PROCESSOR_H
#define ABC2PROGRAM_ABC_MODULE_ARRAY_PROCESSOR_H

#include "abc_file_entity_processor.h"
#include "module_data_accessor-inl.h"

namespace panda::abc2program {

class AbcModuleArrayProcessor : public AbcFileEntityProcessor {
public:
    AbcModuleArrayProcessor(panda_file::File::EntityId entity_id,
                            Abc2ProgramEntityContainer &entity_container);
    void FillProgramData() override;
private:
    void FillModuleRequests(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        const std::vector<uint32_t> &request_modules_offset);
    void FillRegularImportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        uint32_t &request_module_idx, uint32_t &import_name_offset, uint32_t &local_name_offset);
    void FillNamespaceImportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        uint32_t &request_module_idx, uint32_t &local_name_offset);
    void FillLocalExportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        uint32_t export_name_offset, uint32_t &local_name_offset);
    void FillIndirectExportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        uint32_t export_name_offset, uint32_t request_module_idx, uint32_t import_name_offset);
    void FillStarExportEntry(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        uint32_t request_module_idx);
    void FillEntrySize(std::vector<panda::pandasm::LiteralArray::Literal> &literal_vec,
        const std::vector<uint32_t> &num_vec);
    std::unique_ptr<panda_file::ModuleDataAccessor> module_data_accessor_;
};
} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC_MODULE_ARRAY_PROCESSOR_H