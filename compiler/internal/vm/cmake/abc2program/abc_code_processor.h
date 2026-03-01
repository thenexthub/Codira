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

#ifndef ABC2PROGRAM_ABC_CODE_PROCESSOR_H
#define ABC2PROGRAM_ABC_CODE_PROCESSOR_H

#include <unordered_map>
#include <vector>
#include "abc_file_entity_processor.h"
#include "common/abc_code_converter.h"
#include "code_data_accessor-inl.h"


namespace panda::abc2program {

class AbcCodeProcessor : public AbcFileEntityProcessor {
public:
    AbcCodeProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                     panda_file::File::EntityId method_id, pandasm::Function &function);
    void FillProgramData() override;

private:
    void FillFunctionRegsNum();
    void FillIns();
    void FillInsWithoutLabels();
    void AddLabels();
    bool NeedToAddDummyEndIns() const;
    void AddDummyEndIns();
    void AddJumpLabels() const;
    void AddJumpLabel4InsAtIndex(uint32_t inst_idx, pandasm::InsPtr &curr_pa_ins) const;
    void AddLabel4InsAtIndex(uint32_t inst_idx) const;
    void AddLabel4InsAtPc(uint32_t inst_pc) const;
    std::string GetLabelNameAtPc(uint32_t inst_pc) const;
    void FillCatchBlocks();
    void HandleTryBlock(panda_file::CodeDataAccessor::TryBlock &try_block);
    void HandleCatchBlock(panda_file::CodeDataAccessor::CatchBlock &catch_block);
    void FillCatchBlockLabels(pandasm::Function::CatchBlock &pa_catch_block) const;
    void FillExceptionRecord(panda_file::CodeDataAccessor::CatchBlock &catch_block,
                             pandasm::Function::CatchBlock &pa_catch_block) const;
    void FillLocalVariableTable();
    template <typename T>
    void SkipToNextEntryIfNeeded(uint32_t &idx,
                                 uint32_t &offset_start,
                                 uint32_t &offset_end,
                                 uint32_t inst_idx,
                                 const T &table);
    void FillInsDebug();
    uint32_t GetInstIdxByInstPc(uint32_t inst_pc) const;
    uint32_t GetInstPcByInstIdx(uint32_t inst_idx) const;
    panda_file::File::EntityId method_id_;
    pandasm::Function &function_;
    std::unique_ptr<panda_file::CodeDataAccessor> code_data_accessor_;
    std::unique_ptr<AbcCodeConverter> code_converter_;
    std::vector<uint32_t> jump_inst_idx_vec_;
    std::map<uint32_t, uint32_t> inst_pc_idx_map_;
    std::unordered_map<uint32_t, uint32_t> inst_idx_pc_map_;
    mutable std::unordered_map<uint32_t, std::string> inst_idx_label_map_;
    uint32_t ins_size_ = 0;
    uint32_t curr_try_begin_inst_pc_ = 0;
    uint32_t curr_try_end_inst_pc_ = 0;
    uint32_t curr_catch_begin_pc_ = 0;
    uint32_t curr_catch_end_pc_ = 0;
    const panda_file::DebugInfoExtractor &debug_info_extractor_;
};  // AbcCodeProcessor

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC_CODE_PROCESSOR_H
