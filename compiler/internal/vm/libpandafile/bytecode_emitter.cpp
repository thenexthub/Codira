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

#include "bytecode_emitter.h"
#include <bytecode_instruction-inl.h>

namespace panda {

using Opcode = BytecodeInstruction::Opcode;
using Format = BytecodeInstruction::Format;
using BitImmSize = BytecodeEmitter::BitImmSize;

static inline constexpr BitImmSize GetBitLengthUnsigned(uint32_t val)
{
    constexpr size_t BIT_4 = 4;
    constexpr size_t BIT_8 = 8;

    auto bitlen = MinimumBitsToStore(val);
    if (bitlen <= BIT_4) {
        return BitImmSize::BITSIZE_4;
    }
    if (bitlen <= BIT_8) {
        return BitImmSize::BITSIZE_8;
    }
    return BitImmSize::BITSIZE_16;
}

static inline constexpr BitImmSize GetBitLengthSigned(int32_t val)
{
    constexpr int32_t INT4T_MIN = -8;
    constexpr int32_t INT4T_MAX = 7;
    constexpr int32_t INT8T_MIN = std::numeric_limits<int8_t>::min();
    constexpr int32_t INT8T_MAX = std::numeric_limits<int8_t>::max();
    constexpr int32_t INT16T_MIN = std::numeric_limits<int16_t>::min();
    constexpr int32_t INT16T_MAX = std::numeric_limits<int16_t>::max();
    if (INT4T_MIN <= val && val <= INT4T_MAX) {
        return BitImmSize::BITSIZE_4;
    }
    if (INT8T_MIN <= val && val <= INT8T_MAX) {
        return BitImmSize::BITSIZE_8;
    }
    if (INT16T_MIN <= val && val <= INT16T_MAX) {
        return BitImmSize::BITSIZE_16;
    }
    return BitImmSize::BITSIZE_32;
}

static inline void EmitImpl([[maybe_unused]] Span<uint8_t> buf, [[maybe_unused]] Span<const uint8_t> offsets) {}

template <typename Type, typename... Types>
static void EmitImpl(Span<uint8_t> buf, Span<const uint8_t> offsets, Type arg, Types... args)
{
    static constexpr uint8_t BYTEMASK = 0xFF;
    static constexpr uint8_t BITMASK_4 = 0xF;
    static constexpr size_t BIT_4 = 4;
    static constexpr size_t BIT_8 = 8;
    static constexpr size_t BIT_16 = 16;
    static constexpr size_t BIT_32 = 32;
    static constexpr size_t BIT_64 = 64;

    uint8_t offset = offsets[0];
    size_t bitlen = offsets[1] - offsets[0];
    size_t byte_offset = offset / BIT_8;
    size_t bit_offset = offset % BIT_8;
    switch (bitlen) {
        case BIT_4: {
            auto val = static_cast<uint8_t>(arg);
            buf[byte_offset] |= static_cast<uint8_t>(static_cast<uint8_t>(val & BITMASK_4) << bit_offset);
            break;
        }
        case BIT_8: {
            auto val = static_cast<uint8_t>(arg);
            buf[byte_offset] = val;
            break;
        }
        case BIT_16: {
            auto val = static_cast<uint16_t>(arg);
            buf[byte_offset] = val & BYTEMASK;
            buf[byte_offset + 1] = val >> BIT_8;
            break;
        }
        case BIT_32: {
            auto val = static_cast<uint32_t>(arg);
            for (size_t i = 0; i < sizeof(uint32_t); i++) {
                buf[byte_offset + i] = (val >> (i * BIT_8)) & BYTEMASK;
            }
            break;
        }
        case BIT_64: {
            auto val = static_cast<uint64_t>(arg);
            for (size_t i = 0; i < sizeof(uint64_t); i++) {
                buf[byte_offset + i] = (val >> (i * BIT_8)) & BYTEMASK;
            }
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
    EmitImpl(buf, offsets.SubSpan(1), args...);
}

#ifndef WITH_MOCK
template <Format format, typename It, typename... Types>
static size_t Emit(It out, Types... args);

void BytecodeEmitter::Bind(const Label &label)
{
    *label.pc_ = pc_;
    targets_.insert(label);
}

BytecodeEmitter::ErrorCode BytecodeEmitter::Build(std::vector<uint8_t> *output)
{
    ErrorCode res = CheckLabels();
    if (res != ErrorCode::SUCCESS) {
        return res;
    }
    res = ReserveSpaceForOffsets();
    if (res != ErrorCode::SUCCESS) {
        return res;
    }
    res = UpdateBranches();
    if (res != ErrorCode::SUCCESS) {
        return res;
    }
    *output = bytecode_;
    return ErrorCode::SUCCESS;
}

/*
 * NB! All conditional jumps with displacements not fitting into imm16
 * are transformed into two instructions:
 * jcc far   # cc is any condiitonal code
 *      =>
 * jCC next  # CC is inverted cc
 * jmp far
 * next:     # This label is inserted just after previous instruction.
 */
BytecodeEmitter::ErrorCode BytecodeEmitter::ReserveSpaceForOffsets()
{
    uint32_t bias = 0;
    std::map<uint32_t, Label> new_branches;
    auto it = branches_.begin();
    while (it != branches_.end()) {
        uint32_t insn_pc = it->first + bias;
        auto label = it->second;

        BytecodeInstruction insn(&bytecode_[insn_pc]);
        auto opcode = insn.GetOpcode();
        const auto ENCODED_IMM_SIZE = GetBitImmSizeByOpcode(opcode);
        const auto REAL_IMM_SIZE = GetBitLengthSigned(EstimateMaxDistance(insn_pc, label.GetPc(), bias));

        auto new_target = insn_pc;
        size_t extra_bytes = 0;

        if (REAL_IMM_SIZE > ENCODED_IMM_SIZE) {
            auto res = DoReserveSpaceForOffset(insn, insn_pc, REAL_IMM_SIZE, &extra_bytes, &new_target);
            if (res != ErrorCode::SUCCESS) {
                return res;
            }
        }

        new_branches.insert(std::make_pair(new_target, label));
        if (extra_bytes > 0) {
            bias += extra_bytes;
            UpdateLabelTargets(insn_pc, extra_bytes);
        }
        it = branches_.erase(it);
    }
    branches_ = std::move(new_branches);
    return ErrorCode::SUCCESS;
}

BytecodeEmitter::ErrorCode BytecodeEmitter::DoReserveSpaceForOffset(const BytecodeInstruction &insn, uint32_t insn_pc,
                                                                    BitImmSize expected_imm_size,
                                                                    size_t *extra_bytes_ptr, uint32_t *target_ptr)
{
    auto opcode = insn.GetOpcode();
    const auto INSN_SIZE = GetSizeByOpcode(opcode);

    auto upd_op = GetSuitableJump(opcode, expected_imm_size);
    if (upd_op != Opcode::LAST) {
        *extra_bytes_ptr = GetSizeByOpcode(upd_op) - INSN_SIZE;
        bytecode_.insert(bytecode_.begin() + insn_pc + INSN_SIZE, *extra_bytes_ptr, 0);
    } else {
        *extra_bytes_ptr = GetSizeByOpcode(Opcode::JMP_IMM32);
        bytecode_.insert(bytecode_.begin() + insn_pc + INSN_SIZE, *extra_bytes_ptr, 0);

        upd_op = RevertConditionCode(opcode);
        if (upd_op == Opcode::LAST) {
            UNREACHABLE();  // no revcc and no far opcode
            return ErrorCode::INTERNAL_ERROR;
        }
        UpdateBranchOffs(&bytecode_[insn_pc], INSN_SIZE + GetSizeByOpcode(Opcode::JMP_IMM32));
        *target_ptr = insn_pc + INSN_SIZE;
        Emit<Format::IMM32>(bytecode_.begin() + *target_ptr, Opcode::JMP_IMM32, 0);
    }
    if (BytecodeInstruction(reinterpret_cast<uint8_t *>(&upd_op)).IsPrefixed()) {
        Emit<BytecodeInstruction::Format::PREF_NONE>(bytecode_.begin() + insn_pc, upd_op);
    } else {
        Emit<BytecodeInstruction::Format::NONE>(bytecode_.begin() + insn_pc, upd_op);
    }
    return ErrorCode::SUCCESS;
}

BytecodeEmitter::ErrorCode BytecodeEmitter::UpdateBranches()
{
    for (std::pair<const uint32_t, Label> &branch : branches_) {
        uint32_t insn_pc = branch.first;
        Label label = branch.second;
        auto offset = static_cast<int32_t>(label.GetPc()) - static_cast<int32_t>(insn_pc);
        UpdateBranchOffs(&bytecode_[insn_pc], offset);
    }
    return ErrorCode::SUCCESS;
}

void BytecodeEmitter::UpdateLabelTargets(uint32_t pc, size_t bias)
{
    pc_list_.push_front(pc);
    Label fake(pc_list_.begin());
    std::list<Label> updated_labels;
    auto it = targets_.upper_bound(fake);
    while (it != targets_.end()) {
        Label label = *it;
        it = targets_.erase(it);
        *label.pc_ += bias;
        updated_labels.push_back(label);
    }
    targets_.insert(updated_labels.begin(), updated_labels.end());
    pc_list_.pop_front();
}

int32_t BytecodeEmitter::EstimateMaxDistance(uint32_t insn_pc, uint32_t target_pc, uint32_t bias) const
{
    int32_t distance = 0;
    uint32_t end_pc = 0;
    std::map<uint32_t, Label>::const_iterator it;
    if (static_cast<int64_t>(target_pc) < static_cast<int64_t>(insn_pc) + INT32_MIN ||
        static_cast<int64_t>(target_pc) > static_cast<int64_t>(insn_pc) + INT32_MAX) {
        LOG(ERROR, ASSEMBLER) << "Failed to emit jmp/jcc with displacement not fitting into imm32";
    }

    if (target_pc > insn_pc) {
        it = branches_.lower_bound(insn_pc - bias);
        distance = static_cast<int32_t>(target_pc - insn_pc);
        end_pc = target_pc - bias;
    } else if (target_pc < insn_pc) {
        it = branches_.lower_bound(target_pc - bias);
        distance = static_cast<int32_t>(static_cast<int64_t>(target_pc) - static_cast<int64_t>(insn_pc));
        end_pc = insn_pc - bias;
    } else {
        // Do we support branch to itself?
        return 0;
    }

    while (it != branches_.end() && it->first < end_pc) {
        auto insn = BytecodeInstruction(&bytecode_[it->first + bias]);
        auto longest = GetSizeByOpcode(GetLongestJump(insn.GetOpcode()));
        distance += static_cast<int32_t>(longest - insn.GetSize());
        ++it;
    }
    return distance;
}

BytecodeEmitter::ErrorCode BytecodeEmitter::CheckLabels()
{
    for (const std::pair<const uint32_t, Label> &branch : branches_) {
        const Label &label = branch.second;
        if (targets_.find(label) == targets_.end()) {
            return ErrorCode::UNBOUND_LABELS;
        }
    }
    return ErrorCode::SUCCESS;
}

#include <bytecode_emitter_gen.h>
#endif  // WITH_MOCK

}  // namespace panda
