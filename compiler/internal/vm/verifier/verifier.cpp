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
#include <codecvt>
#include <locale>

#include "verifier.h"
#include "class_data_accessor-inl.h"
#include "libpandafile/util/collect_util.h"
#include "zlib.h"

namespace panda::verifier {

Verifier::Verifier(const std::string &filename)
{
    auto file_to_verify = panda_file::File::Open(filename);
    file_.swap(file_to_verify);
}

bool Verifier::Verify()
{
    if (!VerifyChecksum()) {
        return false;
    }

    if (!CollectIdInfos()) {
        return false;
    }

    if (!VerifyConstantPool()) {
        return false;
    }

    return true;
}

bool Verifier::CollectIdInfos()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }
    GetConstantPoolIds();
    if (include_literal_array_ids) {
        GetLiteralIds();
    }
    return CheckConstantPool(verifier::ActionType::COLLECTINFOS);
}

bool Verifier::VerifyChecksum()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }
    uint32_t file_size = file_->GetHeader()->file_size;
    ASSERT(file_size > FILE_CONTENT_OFFSET);
    uint32_t cal_checksum = adler32(1, file_->GetBase() + FILE_CONTENT_OFFSET, file_size - FILE_CONTENT_OFFSET);
    return file_->GetHeader()->checksum == cal_checksum;
}

bool Verifier::VerifyConstantPool()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    if (!CheckConstantPoolIndex()) {
        return false;
    }

    if (!CheckConstantPool(verifier::ActionType::CHECKCONSTPOOLCONTENT)) {
        return false;
    }

    if (!VerifyLiteralArrays()) {
        return false;
    }

    return true;
}

bool Verifier::VerifyRegisterIndex()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    for (const auto id : all_method_ids_) {
        const panda_file::File::EntityId method_id = panda_file::File::EntityId(id);
        panda_file::MethodDataAccessor method_accessor {*file_, method_id};
        if (!method_accessor.GetCodeId().has_value()) {
            continue;
        }
        panda_file::CodeDataAccessor code_data(*file_, method_accessor.GetCodeId().value());
        const uint64_t reg_nums = code_data.GetNumVregs();
        const uint64_t arg_nums = code_data.GetNumArgs();
        const std::optional<uint64_t> valid_regs_num = SafeAdd(reg_nums, arg_nums);
        if (!valid_regs_num.has_value()) {
            LOG(ERROR, VERIFIER) << "Integer overflow detected during register index calculation!";
            return false;
        }
        if (valid_regs_num.value() > MAX_REGISTER_INDEX + 1) {
            LOG(ERROR, VERIFIER) << "Register index exceeds the maximum allowable value (0xffff)!";
            return false;
        }
        auto bc_ins = BytecodeInstruction(code_data.GetInstructions());
        const auto bc_ins_last = bc_ins.JumpTo(code_data.GetCodeSize());
        ASSERT(arg_nums >= DEFAULT_ARGUMENT_NUMBER);
        while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
            const size_t count = GetVRegCount(bc_ins);
            if (count == 0) { // Skip instructions that do not use registers
                bc_ins = bc_ins.GetNext();
                continue;
            }
            if (!CheckVRegIdx(bc_ins, count, valid_regs_num.value())) {
                return false;
            }
            bc_ins = bc_ins.GetNext();
        }
    }
    return true;
}

bool Verifier::VerifyConstantPoolIndex()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    if (!CheckConstantPoolIndex()) {
        return false;
    }

    return true;
}

bool Verifier::VerifyConstantPoolContent()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    if (!CheckConstantPool(verifier::ActionType::CHECKCONSTPOOLCONTENT)) {
        return false;
    }

    if (!VerifyLiteralArrays()) {
        return false;
    }

    return true;
}

void Verifier::GetConstantPoolIds()
{
    if (constant_pool_ids_.size() != 0) {
        return;
    }
    auto index_headers = file_->GetIndexHeaders();
    for (const auto &index_header : index_headers) {
        auto region_indexs = file_->GetMethodIndex(&index_header);
        for (auto &index : region_indexs) {
            constant_pool_ids_.push_back(index.GetOffset());
        }
    }
}

void Verifier::GetLiteralIds()
{
    if (literal_ids_.size() != 0) {
        return;
    }

    if (panda_file::ContainsLiteralArrayInHeader(file_->GetHeader()->version)) {
        const auto literal_arrays = file_->GetLiteralArrays();
        PushToLiteralIds(literal_arrays);
    } else {
        panda::libpandafile::CollectUtil collect_util;
        std::unordered_set<uint32_t> literal_array_ids;
        collect_util.CollectLiteralArray(*file_, literal_array_ids);
        PushToLiteralIds(literal_array_ids);
    }
}

template <typename T>
void Verifier::PushToLiteralIds(T &ids)
{
    for (const auto id : ids) {
        literal_ids_.push_back(id);
    }
}

bool Verifier::CheckConstantPoolActions(const verifier::ActionType type, panda_file::File::EntityId method_id)
{
    switch (type) {
        case verifier::ActionType::CHECKCONSTPOOLCONTENT: {
            return CheckConstantPoolMethodContent(method_id);
        }
        case verifier::ActionType::COLLECTINFOS: {
            all_method_ids_.push_back(method_id.GetOffset());
            return CollectIdInInstructions(method_id);
        }
        default: {
            return true;
        }
    }
}

bool Verifier::CollectIdInInstructions(const panda_file::File::EntityId &method_id)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    if (!method_accessor.GetCodeId().has_value()) {
        return false;
    }
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_size = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();

    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);

    while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
        if (!bc_ins.IsPrimaryOpcodeValid()) {
            LOG(ERROR, VERIFIER) << "Fail to verify primary opcode!";
            return false;
        }
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
            // the idx of any instruction with a literal id is 0
            // except defineclasswithbuffer/callruntime.definesendableclass
            size_t idx = bc_ins.GetLiteralIndex();
            const auto arg_literal_idx = bc_ins.GetId(idx).AsIndex();
            const auto literal_id = file_->ResolveMethodIndex(method_id, arg_literal_idx);
            ins_literal_ids_.insert(literal_id.GetOffset());
        }
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::METHOD_ID)) {
            const auto arg_method_idx = bc_ins.GetId().AsIndex();
            const auto arg_method_id = file_->ResolveMethodIndex(method_id, arg_method_idx);
            ins_method_ids_.insert(arg_method_id.GetOffset());
        }
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
            const auto arg_string_idx = bc_ins.GetId().AsIndex();
            const auto string_id = file_->ResolveOffsetByIndex(method_id, arg_string_idx);
            ins_string_ids_.insert(string_id.GetOffset());
        }
        bc_ins = bc_ins.GetNext();
    }
    return true;
}

void Verifier::CollectModuleLiteralId(const panda_file::File::EntityId &field_id)
{
    panda_file::FieldDataAccessor field_accessor(*file_, field_id);
    const auto literal_id = field_accessor.GetValue<uint32_t>().value();
    if (std::find(literal_ids_.begin(), literal_ids_.end(), literal_id) != literal_ids_.end()) {
        module_literals_.insert(literal_id);
    }
}

bool Verifier::CheckConstantPool(const verifier::ActionType type)
{
    const auto class_idx = file_->GetClasses();
    for (size_t i = 0; i < class_idx.size(); i++) {
        uint32_t class_id = class_idx[i];
        if (class_id > file_->GetHeader()->file_size) {
            LOG(ERROR, VERIFIER) << "Binary file corrupted. out of bounds (0x" << std::hex
                                 << file_->GetHeader()->file_size;
            return false;
        }
        const panda_file::File::EntityId record_id {class_id};
        if (!file_->IsExternal(record_id)) {
            panda_file::ClassDataAccessor class_accessor {*file_, record_id};
            bool check_res = true;
            class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
                check_res = check_res && CheckConstantPoolActions(type, method_accessor.GetMethodId());
            });
            if (!check_res) {
                return false;
            }
            if (type == verifier::ActionType::COLLECTINFOS) {
                class_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
                    CollectModuleLiteralId(field_accessor.GetFieldId());
                });
            }
        }
    }

    return true;
}

size_t Verifier::GetVRegCount(const BytecodeInstruction &bc_ins)
{
    size_t idx = 0; // Represents the idxTH register index in an instruction
    BytecodeInstruction::Format format = bc_ins.GetFormat();
    while (bc_ins.HasVReg(format, idx)) {
        idx++;
    }
    return idx;
}

bool Verifier::IsRangeInstAndHasInvalidRegIdx(const BytecodeInstruction &bc_ins,
                                              const size_t count, uint64_t valid_regs_num)
{
    ASSERT(bc_ins.IsRangeInstruction());

    uint64_t reg_idx = bc_ins.GetVReg(FIRST_INDEX);
    if (IsRegIdxOutOfBounds(reg_idx, valid_regs_num)) { // for [format: +AA/+AAAA vBB vCC], vBB can be verified here
        return true;
    }

    std::optional<uint64_t> max_ins_reg_idx_opt = bc_ins.GetRangeInsLastRegIdx();
    if (!max_ins_reg_idx_opt.has_value()) {
        LOG(ERROR, VERIFIER) << "Integer overflow detected during register index calculation!";
        return true;
    }

    reg_idx = max_ins_reg_idx_opt.value();
    if (IsRegIdxOutOfBounds(reg_idx, valid_regs_num)) {
        return true;
    }

    return false;
}

bool Verifier::IsRegIdxOutOfBounds(uint64_t reg_idx, uint64_t valid_regs_num)
{
    if (reg_idx >= valid_regs_num) {
        LOG(ERROR, VERIFIER) << "Register index out of bounds: 0x" << std::hex
                             << reg_idx << ", Max allowed: 0x" << std::hex << valid_regs_num;
        return true;
    }
    return false;
}

bool Verifier::CheckVRegIdx(const BytecodeInstruction &bc_ins, const size_t count, uint64_t valid_regs_num)
{
    if (bc_ins.IsRangeInstruction() &&
        IsRangeInstAndHasInvalidRegIdx(bc_ins, count, valid_regs_num)) {
        return false;
    }
    for (size_t idx = 0; idx < count; idx++) { // Represents the idxTH register index in an instruction
        uint16_t reg_idx = bc_ins.GetVReg(idx);
        if (reg_idx >= valid_regs_num) {
            LOG(ERROR, VERIFIER) << "Register index out of bounds: 0x" << std::hex
                                 << reg_idx << ", Max allowed: 0x" << std::hex << valid_regs_num;
            return false;
        }
    }
    return true;
}

bool Verifier::VerifyMethodId(const uint32_t &method_id) const
{
    auto iter = std::find(constant_pool_ids_.begin(), constant_pool_ids_.end(), method_id);
    if (iter == constant_pool_ids_.end() ||
        (std::find(literal_ids_.begin(), literal_ids_.end(), method_id) != literal_ids_.end()) ||
        ins_string_ids_.count(method_id)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method id. method_id(0x" << std::hex << method_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::VerifyLiteralId(const uint32_t &literal_id) const
{
    auto iter = std::find(constant_pool_ids_.begin(), constant_pool_ids_.end(), literal_id);
    if (iter == constant_pool_ids_.end() ||
        (std::find(all_method_ids_.begin(), all_method_ids_.end(), literal_id) != all_method_ids_.end()) ||
        ins_string_ids_.count(literal_id)) {
        LOG(ERROR, VERIFIER) << "Fail to verify literal id. literal_id(0x" << std::hex << literal_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::VerifyStringId(const uint32_t &string_id) const
{
    auto iter = std::find(constant_pool_ids_.begin(), constant_pool_ids_.end(), string_id);
    if (iter == constant_pool_ids_.end() ||
        ins_method_ids_.count(string_id) ||
        (std::find(literal_ids_.begin(), literal_ids_.end(), string_id) != literal_ids_.end())) {
        LOG(ERROR, VERIFIER) << "Fail to verify string id. string_id(0x" << std::hex << string_id << ")!";
        return false;
    }
    return true;
}

std::optional<int64_t> Verifier::GetFirstImmFromInstruction(const BytecodeInstruction &bc_ins)
{
    std::optional<int64_t> first_imm = std::optional<int64_t> {};
    size_t index = 0;
    const auto format = bc_ins.GetFormat();
    if (bc_ins.HasImm(format, index)) {
        first_imm = bc_ins.GetImm64(index);
    }

    return first_imm;
}

std::optional<uint64_t> Verifier::GetSlotNumberFromAnnotation(panda_file::MethodDataAccessor &method_accessor)
{
    std::optional<uint64_t> slot_number {};
    method_accessor.EnumerateAnnotations([&](panda_file::File::EntityId annotation_id) {
        panda_file::AnnotationDataAccessor ada(*file_, annotation_id);
        auto *annotation_name = reinterpret_cast<const char *>(file_->GetStringData(ada.GetClassId()).data);
        if (::strcmp("L_ESSlotNumberAnnotation;", annotation_name) == 0) {
            uint32_t elem_count = ada.GetCount();
            for (uint32_t i = 0; i < elem_count; i++) {
                panda_file::AnnotationDataAccessor::Elem adae = ada.GetElement(i);
                auto *elem_name = reinterpret_cast<const char *>(file_->GetStringData(adae.GetNameId()).data);
                if (::strcmp("SlotNumber", elem_name) == 0) {
                    slot_number = adae.GetScalarValue().GetValue();
                }
            }
        }
    });
    return slot_number;
}

bool Verifier::VerifyMethodIdInLiteralArray(const uint32_t &id)
{
    const auto method_id = panda_file::File::EntityId(id).GetOffset();
    auto iter = std::find(all_method_ids_.begin(), all_method_ids_.end(), method_id);
    if (iter == all_method_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Invalid method id(0x" << id << ") in literal array";
        return false;
    }
    return true;
}

bool Verifier::VerifyStringIdInLiteralArray(const uint32_t &id)
{
    auto string_data = file_->GetStringData(panda_file::File::EntityId(id));
    if (string_data.data == nullptr) {
        LOG(ERROR, VERIFIER) << "Invalid string_id. string_id(0x" << std::hex << id << ")!";
        return false;
    }
    auto desc = std::string(utf::Mutf8AsCString(string_data.data));
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring utf16_desc = converter.from_bytes(desc);
    if (string_data.utf16_length != utf16_desc.length()) {
        LOG(ERROR, VERIFIER) << "Invalid string value(0x" << id << ") in literal array";
        return false;
    }
    return true;
}

bool Verifier::VerifyLiteralIdInLiteralArray(const uint32_t &id)
{
    auto iter = std::find(literal_ids_.begin(), literal_ids_.end(), id);
    if (iter == literal_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Invalid literal id(0x" << id << ") in literal array";
        return false;
    }
    return true;
}

bool Verifier::VerifySingleLiteralArray(const panda_file::File::EntityId &literal_id)
{
    auto sp = file_->GetSpanFromId(literal_id);
    const auto literal_vals_num = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    for (size_t i = 0; i < literal_vals_num; i += 2U) { // 2u skip literal item
        const auto tag = static_cast<panda_file::LiteralTag>(panda_file::helpers::Read<panda_file::TAG_SIZE>(&sp));
        switch (tag) {
            case panda_file::LiteralTag::TAGVALUE:
            case panda_file::LiteralTag::BOOL:
            case panda_file::LiteralTag::ACCESSOR:
            case panda_file::LiteralTag::NULLVALUE:
            case panda_file::LiteralTag::BUILTINTYPEINDEX: {
                sp = sp.SubSpan(sizeof(uint8_t)); // run next sp
                break;
            }
            case panda_file::LiteralTag::METHODAFFILIATE: {
                sp = sp.SubSpan(sizeof(uint16_t));
                break;
            }
            case panda_file::LiteralTag::INTEGER:
            case panda_file::LiteralTag::FLOAT:
            case panda_file::LiteralTag::GETTER:
            case panda_file::LiteralTag::SETTER:
            case panda_file::LiteralTag::GENERATORMETHOD:
            case panda_file::LiteralTag::LITERALBUFFERINDEX:
            case panda_file::LiteralTag::ASYNCGENERATORMETHOD: {
                sp = sp.SubSpan(sizeof(uint32_t));
                break;
            }
            case panda_file::LiteralTag::DOUBLE: {
                const auto value = bit_cast<double>(panda_file::helpers::Read<sizeof(uint64_t)>(&sp));
                // true: High 16-bit of double value >= 0xffff
                if (IsImpureNaN(value)) {
                    LOG(ERROR, VERIFIER) << "Fail to verify double value " << value << " in literal array";
                    return false;
                }
                break;
            }
            case panda_file::LiteralTag::ARRAY_U1:
            case panda_file::LiteralTag::ARRAY_U8:
            case panda_file::LiteralTag::ARRAY_I8:
            case panda_file::LiteralTag::ARRAY_U16:
            case panda_file::LiteralTag::ARRAY_I16:
            case panda_file::LiteralTag::ARRAY_U32:
            case panda_file::LiteralTag::ARRAY_I32:
            case panda_file::LiteralTag::ARRAY_U64:
            case panda_file::LiteralTag::ARRAY_I64:
            case panda_file::LiteralTag::ARRAY_F32:
            case panda_file::LiteralTag::ARRAY_F64:
            case panda_file::LiteralTag::ARRAY_STRING: {
                i = literal_vals_num;
                break;
            }
            case panda_file::LiteralTag::STRING: {
                panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
                break;
            }
            case panda_file::LiteralTag::METHOD: {
                const auto value = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
                inner_method_map_.emplace(literal_id.GetOffset(), value);
                if (!VerifyMethodIdInLiteralArray(value)) {
                    return false;
                }
                break;
            }
            case panda_file::LiteralTag::LITERALARRAY: {
                const auto value = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
                inner_literal_map_.emplace(literal_id.GetOffset(), value);
                if (!VerifyLiteralIdInLiteralArray(value)) {
                    return false;
                }
                break;
            }
            default: {
                LOG(ERROR, VERIFIER) << "Invalid literal tag";
                return false;
            }
        }
    }
    return true;
}

bool Verifier::IsModuleLiteralId(const panda_file::File::EntityId &id) const
{
    return module_literals_.find(id.GetOffset()) != module_literals_.end();
}

bool Verifier::VerifyLiteralArrays()
{
    for (const auto &arg_literal_id : literal_ids_) {
        const auto literal_id = panda_file::File::EntityId(arg_literal_id);
        if (!IsModuleLiteralId(literal_id) && !VerifySingleLiteralArray(literal_id)) {
            return false;
        }
    }
    return true;
}

bool Verifier::PrecomputeInstructionIndices(const BytecodeInstruction &bc_ins_start,
                                            const BytecodeInstruction &bc_ins_last)
{
    instruction_index_map_.clear();
    size_t index = 0;
    auto current_ins = bc_ins_start;
    instruction_index_map_[current_ins.GetAddress()] = index;
    
    while (current_ins.GetAddress() < bc_ins_last.GetAddress()) {
        //Must keep IsPrimaryOpcodeValid is the first check item
        if (!current_ins.IsPrimaryOpcodeValid()) {
            LOG(ERROR, VERIFIER) << "Fail to verify primary opcode!";
            return false;
        }
        current_ins = current_ins.GetNext();
        index++;
        instruction_index_map_[current_ins.GetAddress()] = index;
    }
    return true;
}

bool Verifier::IsMethodBytecodeInstruction(const BytecodeInstruction &bc_ins_cur)
{
    if (instruction_index_map_.find(bc_ins_cur.GetAddress()) != instruction_index_map_.end()) {
        return true;
    }
    return false;
}

bool Verifier::VerifyJumpInstruction(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                     const BytecodeInstruction &bc_ins_first, const uint8_t *ins_arr,
                                     panda_file::File::EntityId code_id)
{
    // update maximum forward offset
    const auto bc_ins_forward_size = bc_ins_last.GetAddress() - bc_ins.GetAddress();
    // update maximum backward offset
    const auto bc_ins_backward_size = bc_ins.GetAddress() - bc_ins_first.GetAddress();

    if (bc_ins.IsJumpInstruction()) {
        std::optional<int64_t> immdata = GetFirstImmFromInstruction(bc_ins);
        if (!immdata.has_value()) {
            LOG(ERROR, VERIFIER) << "Fail to get immediate data!";
            return false;
        }
        if ((immdata.value() > 0) && (immdata.value() >= bc_ins_forward_size)) {
            LOG(ERROR, VERIFIER) << "Jump forward out of boundary";
            return false;
        }
        if ((immdata.value() < 0) && (bc_ins_backward_size + immdata.value() < 0)) {
            LOG(ERROR, VERIFIER) << "Jump backward out of boundary";
            return false;
        }

        const auto bc_ins_dest = bc_ins.JumpTo(immdata.value());
        if (!bc_ins_dest.IsPrimaryOpcodeValid()) {
            LOG(ERROR, VERIFIER) << "Fail to verify target jump primary opcode!";
            return false;
        }
        if (!IsMethodBytecodeInstruction(bc_ins_dest)) {
            LOG(ERROR, VERIFIER) << "> error encountered at " << code_id << " (0x" << std::hex << code_id
                                 << "). incorrect instruction at offset: 0x" << (bc_ins.GetAddress() - ins_arr)
                                 << ": invalid jump offset 0x" << immdata.value()
                                 << " - jumping in the middle of another instruction!";
            return false;
        }
    }

    return true;
}

bool Verifier::GetIcSlotFromInstruction(const BytecodeInstruction &bc_ins, uint32_t &first_slot_index,
                                        bool &has_slot, bool &is_two_slot)
{
    std::optional<uint64_t> first_imm = {};
    if (bc_ins.HasFlag(BytecodeInstruction::Flags::ONE_SLOT)) {
        first_imm = GetFirstImmFromInstruction(bc_ins);
        if (!first_imm.has_value()) {
            LOG(ERROR, VERIFIER) << "Fail to get first immediate data!";
            return false;
        }
        first_slot_index = first_imm.value();
        is_two_slot = false;
        has_slot = true;
    } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::TWO_SLOT)) {
        first_imm = GetFirstImmFromInstruction(bc_ins);
        if (!first_imm.has_value()) {
            LOG(ERROR, VERIFIER) << "Fail to get first immediate data!";
            return false;
        }
        first_slot_index = first_imm.value();
        has_slot = true;
        is_two_slot = true;
    }

    return true;
}

bool Verifier::VerifyCatchBlocks(panda_file::CodeDataAccessor::TryBlock &try_block, const BytecodeInstruction &bc_ins,
                                 const BytecodeInstruction &bc_ins_last)
{
    bool result = true;

    try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
        const auto handler_begin_offset = catch_block.GetHandlerPc();
        // GetCodeSize() returns a unsigned long value, which is always >= 0,
        // so handler_end_offset is guaranteed to be >= handler_begin_offset
        const auto handler_end_offset = handler_begin_offset + catch_block.GetCodeSize();

        const auto handler_begin_bc_ins = bc_ins.JumpTo(handler_begin_offset);
        const auto handler_end_bc_ins = bc_ins.JumpTo(handler_end_offset);

        const bool handler_begin_offset_in_range = bc_ins_last.GetAddress() > handler_begin_bc_ins.GetAddress();
        const bool handler_end_offset_in_range = bc_ins_last.GetAddress() >= handler_end_bc_ins.GetAddress();

        if (!handler_begin_offset_in_range) {
            LOG(ERROR, VERIFIER) << "> Invalid catch block begin offset range! address is: 0x" << std::hex
                                 << handler_begin_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!IsMethodBytecodeInstruction(handler_begin_bc_ins)) {
            LOG(ERROR, VERIFIER) << "> Invalid catch block begin offset validity! address is: 0x" << std::hex
                                 << handler_begin_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!handler_end_offset_in_range) {
            LOG(ERROR, VERIFIER) << "> Invalid catch block end offset range! address is: 0x" << std::hex
                                 << handler_end_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!IsMethodBytecodeInstruction(handler_end_bc_ins)) {
            LOG(ERROR, VERIFIER) << "> Invalid catch block end offset validity! address is: 0x" << std::hex
                                 << handler_end_bc_ins.GetAddress();
            result = false;
            return false;
        }

        return true;
    });

    return result;
}

bool Verifier::VerifyTryBlocks(panda_file::CodeDataAccessor &code_accessor, const BytecodeInstruction &bc_ins,
                               const BytecodeInstruction &bc_ins_last)
{
    bool result = true;

    code_accessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        const auto try_begin_bc_ins = bc_ins.JumpTo(try_block.GetStartPc());
        // GetLength() returns a uint32 value, which is always >= 0,
        // so try_end_bc_ins is guaranteed to be >= try_begin_bc_ins
        const auto try_end_bc_ins = bc_ins.JumpTo(try_block.GetStartPc() + try_block.GetLength());

        const bool try_begin_offset_in_range = bc_ins_last.GetAddress() > try_begin_bc_ins.GetAddress();
        const bool try_end_offset_in_range = bc_ins_last.GetAddress() >= try_end_bc_ins.GetAddress();

        if (!try_begin_offset_in_range) {
            LOG(ERROR, VERIFIER) << "> Invalid try block begin offset range! address is: 0x" << std::hex
                                 << try_begin_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!IsMethodBytecodeInstruction(try_begin_bc_ins)) {
            LOG(ERROR, VERIFIER) << "> Invalid try block begin offset validity! address is: 0x" << std::hex
                                 << try_begin_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!try_end_offset_in_range) {
            LOG(ERROR, VERIFIER) << "> Invalid try block end offset range! address is: 0x" << std::hex
                                 << try_end_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!IsMethodBytecodeInstruction(try_end_bc_ins)) {
            LOG(ERROR, VERIFIER) << "> Invalid try block end offset validity! address is: 0x" << std::hex
                                 << try_end_bc_ins.GetAddress();
            result = false;
            return false;
        }
        if (!VerifyCatchBlocks(try_block, bc_ins, bc_ins_last)) {
            LOG(ERROR, VERIFIER) << "Catch block validation failed!";
            result = false;
            return false;
        }

        return true;
    });

    return result;
}


bool Verifier::VerifySlotNumber(panda_file::MethodDataAccessor &method_accessor, const uint32_t &slot_number,
                                const panda_file::File::EntityId &method_id)
{
    const auto ann_slot_number = GetSlotNumberFromAnnotation(method_accessor);
    if (!ann_slot_number.has_value()) {
        LOG(INFO, VERIFIER) << "There is no slot number information in annotaion.";
        // To be compatible with old abc, slot number verification is not continued
        return true;
    }
    if (slot_number == ann_slot_number.value()) {
        return true;
    }

    LOG(ERROR, VERIFIER) << "Slot number has been falsified in method 0x" << method_id;
    return false;
}

bool Verifier::VerifyMethodRegisterIndex(panda_file::CodeDataAccessor &code_accessor,
                                         std::optional<uint64_t> &valid_regs_num)
{
    const uint64_t reg_nums = code_accessor.GetNumVregs();
    const uint64_t arg_nums = code_accessor.GetNumArgs();
    valid_regs_num = SafeAdd(reg_nums, arg_nums);
    if (!valid_regs_num.has_value()) {
        LOG(ERROR, VERIFIER) << "Integer overflow detected during register index calculation!";
        return false;
    }
    if (valid_regs_num.value() > MAX_REGISTER_INDEX + 1) {
        LOG(ERROR, VERIFIER) << "Register index exceeds the maximum allowable value (0xffff)!";
        return false;
    }
    return true;
}

bool Verifier::VerifyMethodInstructions(const MethodInfos &infos)
{
    auto current_ins = infos.bc_ins;
    auto last_ins = infos.bc_ins_last;
    auto code_id = infos.method_accessor.GetCodeId().value();
    auto method_id = infos.method_id;
    auto valid_regs_num = infos.valid_regs_num.value();
    auto ins_slot_num = infos.ins_slot_num;
    auto has_slot = infos.has_slot;
    auto is_two_slot = infos.is_two_slot;
    
    while (current_ins.GetAddress() != last_ins.GetAddress()) {
        if (current_ins.GetAddress() > last_ins.GetAddress()) {
            LOG(ERROR, VERIFIER) << "> error encountered at " << code_id
                                 << " (0x" << std::hex << code_id
                                 << "). bytecode instructions sequence corrupted for method "
                                 << method_id
                                 << "! went out of bounds";
            return false;
        }
        if (!current_ins.IsJumpInstruction() && !current_ins.IsReturnOrThrowInstruction()
            && current_ins.GetNext().GetAddress() == last_ins.GetAddress()) {
            LOG(ERROR, VERIFIER) << "> error encountered at " << code_id
                                 << " (0x" << std::hex << code_id
                                 << "). bytecode instructions sequence corrupted for method "
                                 << method_id
                                 << "! went out of bounds";
            return false;
        }
        const size_t count = GetVRegCount(current_ins);
        if (count != 0 && !CheckVRegIdx(current_ins, count, valid_regs_num)) {
            return false;
        }
        if (!VerifyJumpInstruction(current_ins, last_ins,
                                   infos.bc_ins_init, infos.ins_arr,
                                   code_id)) {
            LOG(ERROR, VERIFIER) << "Invalid target position of jump instruction";
            return false;
        }
        if (!GetIcSlotFromInstruction(current_ins, ins_slot_num,
                                      has_slot, is_two_slot)) {
            LOG(ERROR, VERIFIER) << "Fail to get first slot index!";
            return false;
        }
        current_ins = current_ins.GetNext();
    }
    return true;
}

bool Verifier::CheckConstantPoolMethodContent(const panda_file::File::EntityId &method_id)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    if (!method_accessor.GetCodeId().has_value()) {
        LOG(ERROR, VERIFIER) << "Fail to get code id!";
        return false;
    }
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_size = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);
    const auto bc_ins_init = bc_ins; // initial PC value
    uint32_t ins_slot_num = 0; // For ic slot index verification
    bool has_slot = false;
    bool is_two_slot = false;
    std::optional<uint64_t> valid_regs_num = 0;
    MethodInfos infos = {bc_ins_init, bc_ins, bc_ins_last, method_accessor, method_id,
                         valid_regs_num, ins_arr, ins_slot_num, has_slot, is_two_slot};
    if (ins_size <= 0) {
        LOG(ERROR, VERIFIER) << "Fail to verify code size!";
        return false;
    }
    if (!VerifyMethodRegisterIndex(code_accessor, valid_regs_num)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method register index!";
        return false;
    }
    if (!PrecomputeInstructionIndices(bc_ins, bc_ins_last)) {
        LOG(ERROR, VERIFIER) << "Fail to precompute instruction indices!";
        return false;
    }
    if (!IsMethodBytecodeInstruction(bc_ins)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method first bytecode instruction!";
    }
    if (!VerifyTryBlocks(code_accessor, bc_ins, bc_ins_last)) {
        LOG(ERROR, VERIFIER) << "Fail to verify try blocks or catch blocks!";
        return false;
    }
    if (!VerifyMethodInstructions(infos)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method instructions!";
        return false;
    }
    if (has_slot) {
        if (is_two_slot) {
            ins_slot_num += 1; // when there are two slots for the last instruction, the slot index increases
        }
        ins_slot_num += 1; // slot index starts with zero
    }
    return true;
}

bool Verifier::CheckConstantPoolIndex() const
{
    for (auto &id : ins_method_ids_) {
        if (!VerifyMethodId(id)) {
            return false;
        }
    }

    for (auto &id : ins_literal_ids_) {
        if (!VerifyLiteralId(id)) {
            return false;
        }
    }

    for (auto &id : ins_string_ids_) {
        if (!VerifyStringId(id)) {
            return false;
        }
    }

    return true;
}

std::optional<uint64_t> Verifier::SafeAdd(uint64_t a, uint64_t b) const
{
    if (a > std::numeric_limits<uint64_t>::max() - b) {
        return std::nullopt;
    }
    return a + b;
}
} // namespace panda::verifier
