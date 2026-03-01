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

#ifndef VERIFIER_VERIFIER_H
#define VERIFIER_VERIFIER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bytecode_instruction_enum_gen.h"
#include "bytecode_instruction-inl.h"
#include "code_data_accessor-inl.h"
#include "file.h"
#include "file_items.h"
#include "literal_data_accessor.h"
#include "method_data_accessor-inl.h"
#include "utils/bit_utils.h"
#include "utils/utf.h"

namespace panda::verifier {

using Opcode = BytecodeInstruction::Opcode;
using TaggedType = uint64_t;

enum class ActionType {
    CHECKCONSTPOOLCONTENT,
    COLLECTINFOS,
};

struct MethodInfos {
    const BytecodeInstruction &bc_ins_init;
    BytecodeInstruction &bc_ins;
    const BytecodeInstruction &bc_ins_last;
    panda_file::MethodDataAccessor &method_accessor;
    const panda_file::File::EntityId &method_id;
    std::optional<uint64_t> &valid_regs_num;
    const uint8_t *ins_arr;
    uint32_t &ins_slot_num;
    bool &has_slot;
    bool &is_two_slot;
};

class Verifier {
public:
    explicit Verifier(const std::string &filename);
    ~Verifier() = default;

    bool Verify();
    bool CollectIdInfos();
    bool VerifyChecksum();
    bool VerifyConstantPool();
    bool VerifyRegisterIndex();
    bool VerifyConstantPoolIndex();
    bool VerifyConstantPoolContent();

    bool include_literal_array_ids = true;
    std::vector<uint32_t> literal_ids_;
    std::unordered_map<uint32_t, uint32_t> inner_literal_map_;
    std::unordered_map<uint32_t, uint32_t> inner_method_map_;

    static constexpr size_t TAG_BITS_SIZE = 16;
    static constexpr size_t TAG_BITS_SHIFT = BitNumbers<TaggedType>() - TAG_BITS_SIZE;
    static_assert((TAG_BITS_SHIFT + TAG_BITS_SIZE) == sizeof(TaggedType) * CHAR_BIT, "Insufficient bits!");
    static constexpr TaggedType TAG_MASK = ((1ULL << TAG_BITS_SIZE) - 1ULL) << TAG_BITS_SHIFT;
    static constexpr TaggedType TAG_INT = TAG_MASK;
    static constexpr size_t DOUBLE_ENCODE_OFFSET_BIT = 48;
    static constexpr TaggedType DOUBLE_ENCODE_OFFSET = 1ULL << DOUBLE_ENCODE_OFFSET_BIT;

private:
    void GetLiteralIds();
    template <typename T>
    void PushToLiteralIds(T &ids);
    void GetConstantPoolIds();
    bool CollectIdInInstructions(const panda_file::File::EntityId &method_id);
    void CollectModuleLiteralId(const panda_file::File::EntityId &field_id);
    bool CheckConstantPool(const verifier::ActionType type);
    size_t GetVRegCount(const BytecodeInstruction &bc_ins);
    bool CheckConstantPoolActions(const verifier::ActionType type, panda_file::File::EntityId method_id);
    bool VerifyMethodId(const uint32_t &method_id) const;
    bool VerifyLiteralId(const uint32_t &literal_id) const;
    bool VerifyStringId(const uint32_t &string_id) const;
    bool IsRangeInstAndHasInvalidRegIdx(const BytecodeInstruction &bc_ins,
                                        const size_t count, uint64_t valid_regs_num);
    bool IsRegIdxOutOfBounds(uint64_t reg_idx, uint64_t valid_regs_num);
    bool CheckVRegIdx(const BytecodeInstruction &bc_ins, const size_t count, uint64_t valid_regs_num);
    std::optional<int64_t> GetFirstImmFromInstruction(const BytecodeInstruction &bc_ins);
    std::optional<uint64_t> GetSlotNumberFromAnnotation(panda_file::MethodDataAccessor &method_accessor);
    bool VerifyMethodIdInLiteralArray(const uint32_t &id);
    bool VerifyStringIdInLiteralArray(const uint32_t &id);
    bool VerifyLiteralIdInLiteralArray(const uint32_t &id);
    bool IsModuleLiteralId(const panda_file::File::EntityId &id) const;
    bool VerifySingleLiteralArray(const panda_file::File::EntityId &literal_id);
    bool VerifyLiteralArrays();
    bool VerifyJumpInstruction(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                               const BytecodeInstruction &bc_ins_init, const uint8_t *ins_arr,
                               panda_file::File::EntityId code_id);
    bool GetIcSlotFromInstruction(const BytecodeInstruction &bc_ins, uint32_t &first_slot_index, bool &has_slot,
                                  bool &is_two_slot);
    bool VerifySlotNumber(panda_file::MethodDataAccessor &method_accessor, const uint32_t &slot_number,
                          const panda_file::File::EntityId &method_id);
    bool CheckConstantPoolMethodContent(const panda_file::File::EntityId &method_id);
    bool CheckConstantPoolIndex() const;
    std::optional<uint64_t> SafeAdd(uint64_t a, uint64_t b) const;
    bool VerifyCatchBlocks(panda_file::CodeDataAccessor::TryBlock &try_block, const BytecodeInstruction &bc_ins,
                           const BytecodeInstruction &bc_ins_last);
    bool VerifyTryBlocks(panda_file::CodeDataAccessor &code_accessor, const BytecodeInstruction &bc_ins,
                         const BytecodeInstruction &bc_ins_last);
    bool PrecomputeInstructionIndices(const BytecodeInstruction &bc_ins_start, const BytecodeInstruction &bc_ins_last);
    bool IsMethodBytecodeInstruction(const BytecodeInstruction &bc_ins_cur);
    bool VerifyMethodRegisterIndex(panda_file::CodeDataAccessor &code_accessor,
                                   std::optional<uint64_t> &max_reg_idx);
    bool VerifyMethodInstructions(const MethodInfos &infos);

    inline bool IsImpureNaN(double value)
    {
        // Tests if the double value would break tagged double encoding.
        return bit_cast<TaggedType>(value) >= (Verifier::TAG_INT - Verifier::DOUBLE_ENCODE_OFFSET);
    }

    std::unique_ptr<const panda_file::File> file_;
    std::vector<uint32_t> constant_pool_ids_;
    std::vector<uint32_t> all_method_ids_;
    std::unordered_set<uint32_t> ins_method_ids_;
    std::unordered_set<uint32_t> ins_literal_ids_;
    std::unordered_set<uint32_t> ins_string_ids_;
    std::unordered_set<uint32_t> module_literals_;
    static constexpr size_t DEFAULT_ARGUMENT_NUMBER = 3;
    static constexpr uint32_t FILE_CONTENT_OFFSET = 12U;
    static constexpr size_t FIRST_INDEX = 0;
    static constexpr size_t SECOND_INDEX = 1;
    static constexpr uint64_t  MAX_REGISTER_INDEX = 0xffff;
    std::unordered_map<const uint8_t*, size_t> instruction_index_map_;
};
} // namespace panda::verifier
#endif
