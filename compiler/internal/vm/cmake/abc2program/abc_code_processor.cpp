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

#include "abc_code_processor.h"

namespace panda::abc2program {

AbcCodeProcessor::AbcCodeProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                                   panda_file::File::EntityId method_id, pandasm::Function &function)
    : AbcFileEntityProcessor(entity_id, entity_container), method_id_(method_id), function_(function),
      debug_info_extractor_(entity_container.GetDebugInfoExtractor())
{
    code_data_accessor_ = std::make_unique<panda_file::CodeDataAccessor>(*file_, entity_id_);
    code_converter_ = std::make_unique<AbcCodeConverter>(entity_container_);
}

void AbcCodeProcessor::FillProgramData()
{
    FillFunctionRegsNum();
    FillIns();
    FillCatchBlocks();
    AddLabels();
    FillLocalVariableTable();
}

void AbcCodeProcessor::AddLabels()
{
    std::vector<pandasm::InsPtr> new_ins;
    new_ins.reserve(function_.ins.size() + inst_idx_label_map_.size());

    size_t inserted_label_cnt = 0;
    for (size_t i = 0; i <= function_.ins.size(); i++) {
        if (inst_idx_label_map_.find(i) != inst_idx_label_map_.end()) {
            new_ins.emplace_back(new pandasm::LabelIns(inst_idx_label_map_[i]));
            inserted_label_cnt++;
        }
        // Update inst_pc_idx_map_ to fill function's local_variable_debug correctly
        inst_pc_idx_map_[inst_idx_pc_map_[i]] += inserted_label_cnt;
        if (i < function_.ins.size()) {
            new_ins.emplace_back(std::move(function_.ins[i]));
        }
    }

    function_.ins.swap(new_ins);
}

void AbcCodeProcessor::FillFunctionRegsNum()
{
    function_.regs_num = code_data_accessor_->GetNumVregs();
}

void AbcCodeProcessor::FillIns()
{
    FillInsWithoutLabels();
    if (NeedToAddDummyEndIns()) {
        AddDummyEndIns();
    }
    AddJumpLabels();
    FillInsDebug();
}

void AbcCodeProcessor::FillInsWithoutLabels()
{
    ins_size_ = code_data_accessor_->GetCodeSize();
    const uint8_t *ins_arr = code_data_accessor_->GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size_);
    uint32_t inst_pc = 0;
    uint32_t inst_idx = 0;
    uint32_t inst_cnt = 0;
    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        inst_cnt++;
        bc_ins = bc_ins.GetNext();
    }
    function_.ins.reserve(inst_cnt);
    bc_ins = BytecodeInstruction(ins_arr);
    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        pandasm::Ins *pa_ins = code_converter_->BytecodeInstructionToPandasmInstruction(bc_ins, method_id_);
        if (pa_ins == nullptr) {
            LOG(FATAL, ABC2PROGRAM) << "> Failed to convert Bytecode to Pandasm.";
        }
        /*
         * Private field jump_inst_idx_vec_ store all jump inst idx in a pandasm::Function.
         * For example, a pandasm::Function has 100 pandasm::Ins with only 4 jump pandasm::Ins.
         * When we add label for jump target and add jump id for jump inst,
         * we only need to enumerate the 4 jump pandasm::Ins which stored in jump_inst_idx_vec_,
         * while no need to enumerate the whole 100 pandasm::Ins.
         * It will improve our compile performance.
        */
        if (pa_ins->IsJump()) {
            jump_inst_idx_vec_.emplace_back(inst_idx);
        }
        function_.ins.emplace_back(pa_ins);
        inst_pc_idx_map_.emplace(inst_pc, inst_idx);
        inst_idx_pc_map_.emplace(inst_idx, inst_pc);
        inst_idx++;
        inst_pc += bc_ins.GetSize();
        bc_ins = bc_ins.GetNext();
    }
}

bool AbcCodeProcessor::NeedToAddDummyEndIns() const
{
    bool need_to_add_dummy_end_ins = false;
    code_data_accessor_->EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            uint32_t catch_end_pc = catch_block.GetHandlerPc() + catch_block.GetCodeSize();
            if (catch_end_pc == ins_size_) {
                need_to_add_dummy_end_ins = true;
            }
            return true;
        });
        return true;
    });
    return need_to_add_dummy_end_ins;
}

void AbcCodeProcessor::AddDummyEndIns()
{
    uint32_t inst_idx = static_cast<uint32_t>(function_.ins.size());
    inst_pc_idx_map_.emplace(ins_size_, inst_idx);
    inst_idx_pc_map_.emplace(inst_idx, ins_size_);
    AddLabel4InsAtIndex(inst_idx);
}

uint32_t AbcCodeProcessor::GetInstIdxByInstPc(uint32_t inst_pc) const
{
    if (inst_pc_idx_map_.empty()) {
        return 0;
    }
    auto it = inst_pc_idx_map_.find(inst_pc);
    // Locate the end of ins list of current function
    if (it == inst_pc_idx_map_.end()) {
        return (inst_pc_idx_map_.rbegin()->second) + 1;
    }
    return it->second;
}

uint32_t AbcCodeProcessor::GetInstPcByInstIdx(uint32_t inst_idx) const
{
    auto it = inst_idx_pc_map_.find(inst_idx);
    ASSERT(it != inst_idx_pc_map_.end());
    return it->second;
}

void AbcCodeProcessor::AddJumpLabels() const
{
    for (uint32_t jump_inst_idx : jump_inst_idx_vec_) {
        pandasm::InsPtr &jump_pa_ins = function_.ins[jump_inst_idx];
        AddJumpLabel4InsAtIndex(jump_inst_idx, jump_pa_ins);
    }
}

void AbcCodeProcessor::AddJumpLabel4InsAtIndex(uint32_t curr_inst_idx, pandasm::InsPtr &curr_pa_ins) const
{
    uint32_t curr_inst_pc = GetInstPcByInstIdx(curr_inst_idx);
    const int32_t jmp_offset = std::stoi(curr_pa_ins->Ids().at(0));
    uint32_t dst_inst_pc = curr_inst_pc + jmp_offset;
    uint32_t dst_inst_idx = GetInstIdxByInstPc(dst_inst_pc);
    AddLabel4InsAtIndex(dst_inst_idx);
    curr_pa_ins->SetId(0, inst_idx_label_map_.at(dst_inst_idx));
}

void AbcCodeProcessor::FillCatchBlocks()
{
    code_data_accessor_->EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        HandleTryBlock(try_block);
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            HandleCatchBlock(catch_block);
            return true;
        });
        return true;
    });
}

void AbcCodeProcessor::HandleTryBlock(panda_file::CodeDataAccessor::TryBlock &try_block)
{
    curr_try_begin_inst_pc_ = try_block.GetStartPc();
    curr_try_end_inst_pc_ = try_block.GetStartPc() + try_block.GetLength();
    AddLabel4InsAtPc(curr_try_begin_inst_pc_);
    AddLabel4InsAtPc(curr_try_end_inst_pc_);
}

void AbcCodeProcessor::HandleCatchBlock(panda_file::CodeDataAccessor::CatchBlock &catch_block)
{
    curr_catch_begin_pc_ = catch_block.GetHandlerPc();
    curr_catch_end_pc_ = catch_block.GetHandlerPc() + catch_block.GetCodeSize();
    AddLabel4InsAtPc(curr_catch_begin_pc_);
    AddLabel4InsAtPc(curr_catch_end_pc_);
    pandasm::Function::CatchBlock pa_catch_block{};
    FillCatchBlockLabels(pa_catch_block);
    FillExceptionRecord(catch_block, pa_catch_block);
    function_.catch_blocks.emplace_back(pa_catch_block);
}

void AbcCodeProcessor::FillCatchBlockLabels(pandasm::Function::CatchBlock &pa_catch_block) const
{
    pa_catch_block.try_begin_label = GetLabelNameAtPc(curr_try_begin_inst_pc_);
    pa_catch_block.try_end_label = GetLabelNameAtPc(curr_try_end_inst_pc_);
    pa_catch_block.catch_begin_label = GetLabelNameAtPc(curr_catch_begin_pc_);
    pa_catch_block.catch_end_label = GetLabelNameAtPc(curr_catch_end_pc_);
}

void AbcCodeProcessor::FillExceptionRecord(panda_file::CodeDataAccessor::CatchBlock &catch_block,
                                           pandasm::Function::CatchBlock &pa_catch_block) const
{
    uint32_t class_idx = catch_block.GetTypeIdx();
    if (class_idx == panda_file::INVALID_INDEX) {
        pa_catch_block.exception_record = "";
    } else {
        const panda_file::File::EntityId class_id = file_->ResolveClassIndex(method_id_, class_idx);
        pa_catch_block.exception_record = entity_container_.GetFullRecordNameById(class_id);
    }
}

void AbcCodeProcessor::AddLabel4InsAtIndex(uint32_t inst_idx) const
{
    inst_idx_label_map_.emplace(inst_idx, AbcFileUtils::GetLabelNameByInstIdx(inst_idx));
}

void AbcCodeProcessor::AddLabel4InsAtPc(uint32_t inst_pc) const
{
    uint32_t inst_idx = GetInstIdxByInstPc(inst_pc);
    AddLabel4InsAtIndex(inst_idx);
}

std::string AbcCodeProcessor::GetLabelNameAtPc(uint32_t inst_pc) const
{
    uint32_t inst_idx = GetInstIdxByInstPc(inst_pc);
    return AbcFileUtils::GetLabelNameByInstIdx(inst_idx);
}

void AbcCodeProcessor::FillLocalVariableTable()
{
    const std::vector<panda_file::LocalVariableInfo>& variables =
        debug_info_extractor_.GetLocalVariableTable(method_id_);
    
    for (const auto &variable : variables) {
        uint32_t start_idx = GetInstIdxByInstPc(variable.start_offset);
        uint32_t end_idx = GetInstIdxByInstPc(variable.end_offset);
        uint32_t length = end_idx - start_idx;
        panda::pandasm::debuginfo::LocalVariable local_var = {variable.name,
                                                              variable.type,
                                                              variable.type_signature,
                                                              variable.reg_number,
                                                              start_idx,
                                                              length};
        function_.local_variable_debug.push_back(local_var);
    }
}

void AbcCodeProcessor::FillInsDebug()
{
    constexpr size_t DEFAULT_LINE = -1;
    constexpr uint32_t DEFAULT_COLUMN = -1;
    uint32_t line_idx = 0;
    uint32_t column_idx = 0;
    uint32_t offset_start = UINT_MAX;
    uint32_t offset_end = UINT_MAX;
    size_t line = DEFAULT_LINE;
    uint32_t column = DEFAULT_COLUMN;
    const std::vector<panda::panda_file::LineTableEntry>& line_table =
        debug_info_extractor_.GetLineNumberTable(method_id_);
    const std::vector<panda::panda_file::ColumnTableEntry>& column_table =
        debug_info_extractor_.GetColumnNumberTable(method_id_);

    for (uint32_t inst_idx = 0; inst_idx < function_.ins.size(); inst_idx++) {
        SkipToNextEntryIfNeeded(line_idx, offset_start, offset_end, inst_idx, line_table);
        line = line_idx > 0 ? line_table[line_idx - 1].line : line;
        function_.ins[inst_idx]->ins_debug.line_number = line;
    }

    // Column table may be empty if all ins of current function has default column number
    // The first entry of column table is the offset of the first ins for which column number differs from the default
    offset_start = 0;
    offset_end = column_table.size() > 0 ? column_table[0].offset : UINT_MAX;
    for (uint32_t inst_idx = 0; inst_idx < function_.ins.size(); inst_idx++) {
        SkipToNextEntryIfNeeded(column_idx, offset_start, offset_end, inst_idx, column_table);
        column = column_idx > 0 ? column_table[column_idx - 1].column : column;
        function_.ins[inst_idx]->ins_debug.column_number = column;
    }
}

template <typename T>
void AbcCodeProcessor::SkipToNextEntryIfNeeded(uint32_t &idx,
                                               uint32_t &offset_start,
                                               uint32_t &offset_end,
                                               uint32_t inst_idx,
                                               const T &table)
{
    uint32_t ins_offset = GetInstPcByInstIdx(inst_idx);
    ASSERT(ins_offset < UINT_MAX);
    while (idx < table.size() && (ins_offset < offset_start || ins_offset >= offset_end)) {
        offset_start = table[idx].offset;
        ++idx;
        offset_end = idx < table.size() ? table[idx].offset : UINT_MAX;
    }
}

}  // namespace panda::abc2program
