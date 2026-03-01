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

#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/bytecode_instruction-inl.h"

#include <libarkbase/macros.h>
#include <libarkbase/utils/bit_utils.h>
#include <libarkbase/utils/span.h>

namespace ark {

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
    size_t byteOffset = offset / BIT_8;
    size_t bitOffset = offset % BIT_8;
    switch (bitlen) {
        case BIT_4: {
            auto val = static_cast<uint8_t>(arg);
            buf[byteOffset] |= static_cast<uint8_t>(static_cast<uint8_t>(val & BITMASK_4) << bitOffset);
            break;
        }
        case BIT_8: {
            auto val = static_cast<uint8_t>(arg);
            buf[byteOffset] = val;
            break;
        }
        case BIT_16: {
            auto val = static_cast<uint16_t>(arg);
            buf[byteOffset] = val & BYTEMASK;
            buf[byteOffset + 1] = val >> BIT_8;
            break;
        }
        case BIT_32: {
            auto val = static_cast<uint32_t>(arg);
            for (size_t i = 0; i < sizeof(uint32_t); i++) {
                buf[byteOffset + i] = (val >> (i * BIT_8)) & BYTEMASK;
            }
            break;
        }
        case BIT_64: {
            auto val = static_cast<uint64_t>(arg);
            for (size_t i = 0; i < sizeof(uint64_t); i++) {
                buf[byteOffset + i] = (val >> (i * BIT_8)) & BYTEMASK;
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

template <Format FORMAT, typename It, typename... Types>
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
    res = ReserveSpaceForOffsets(true);
    if (res != ErrorCode::SUCCESS) {
        return res;
    }
    res = ReserveSpaceForOffsets(false);
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
BytecodeEmitter::ErrorCode BytecodeEmitter::ReserveSpaceForOffsets(const bool isFirstCheck)
{
    uint32_t bias = 0;
    std::map<uint32_t, Label> newBranches;
    auto it = branches_.begin();
    while (it != branches_.end()) {
        uint32_t insnPc = it->first + bias;
        auto label = it->second;

        BytecodeInstruction insn(&bytecode_[insnPc]);
        auto opcode = insn.GetOpcode();
        const auto encodedImmSize = GetBitImmSizeByOpcode(opcode);
        BitImmSize realImmSize;
        if (isFirstCheck) {
            realImmSize = GetBitLengthSigned(EstimateMaxDistance(insnPc, label.GetPc(), bias));
        } else {
            realImmSize = GetBitLengthSigned(label.GetPc() - insnPc);
        }
        auto newTarget = insnPc;
        size_t extraBytes = 0;

        if (realImmSize > encodedImmSize) {
            auto res = UpdateSpaceForOffset(insn, insnPc, realImmSize, &extraBytes, &newTarget);
            if (res != ErrorCode::SUCCESS) {
                return res;
            }
        }

        newBranches.insert(std::make_pair(newTarget, label));
        if (extraBytes > 0) {
            bias += extraBytes;
            UpdateLabelTargets(insnPc, extraBytes);
        }
        it = branches_.erase(it);
    }
    branches_ = std::move(newBranches);
    return ErrorCode::SUCCESS;
}

BytecodeEmitter::ErrorCode BytecodeEmitter::UpdateSpaceForOffset(const BytecodeInstruction &insn, uint32_t insnPc,
                                                                 BitImmSize expectedImmSize, size_t *extraBytesPtr,
                                                                 uint32_t *targetPtr)
{
    auto opcode = insn.GetOpcode();
    const auto insnSize = GetSizeByOpcode(opcode);
    auto lowestOp = GetLowestSizeOp(opcode);
    auto sizeDifference = insnSize - GetSizeByOpcode(lowestOp);
    auto updOp = GetSuitableJump(lowestOp, expectedImmSize);
    using DifferenceTypeT = decltype(bytecode_)::iterator::difference_type;
    if (updOp != Opcode::LAST) {
        *extraBytesPtr = GetSizeByOpcode(updOp) - insnSize;
        bytecode_.insert(bytecode_.begin() + static_cast<DifferenceTypeT>(insnPc + insnSize), *extraBytesPtr, 0);
    } else {
        *extraBytesPtr = GetSizeByOpcode(Opcode::JMP_IMM32) - sizeDifference;
        bytecode_.insert(bytecode_.begin() + static_cast<DifferenceTypeT>(insnPc + insnSize), *extraBytesPtr, 0);
        updOp = RevertConditionCode(lowestOp);
        if (updOp == Opcode::LAST) {
            UNREACHABLE();  // no revcc and no far opcode
            return ErrorCode::INTERNAL_ERROR;
        }
        ASSERT(insnPc < bytecode_.size());
        ASSERT(insnPc + insnSize <= bytecode_.size());
        UpdateBranchOffs(&bytecode_[insnPc],
                         static_cast<int32_t>(insnSize + GetSizeByOpcode(Opcode::JMP_IMM32) - sizeDifference));
        *targetPtr = insnPc + insnSize - sizeDifference;
        Emit<Format::IMM32>(bytecode_.begin() + *targetPtr, Opcode::JMP_IMM32, 0);
    }
    if (BytecodeInstruction(reinterpret_cast<uint8_t *>(&updOp)).IsPrefixed()) {
        Emit<BytecodeInstruction::Format::PREF_NONE>(bytecode_.begin() + insnPc, updOp);
    } else {
        Emit<BytecodeInstruction::Format::NONE>(bytecode_.begin() + insnPc, updOp);
    }
    return ErrorCode::SUCCESS;
}

BytecodeEmitter::ErrorCode BytecodeEmitter::UpdateBranches()
{
    for (std::pair<const uint32_t, Label> &branch : branches_) {
        uint32_t insnPc = branch.first;
        Label label = branch.second;
        auto offset = static_cast<int32_t>(label.GetPc()) - static_cast<int32_t>(insnPc);
        UpdateBranchOffs(&bytecode_[insnPc], offset);
    }
    return ErrorCode::SUCCESS;
}

void BytecodeEmitter::UpdateLabelTargets(uint32_t pc, size_t bias)
{
    pcList_.push_front(pc);
    Label fake(pcList_.begin());
    std::list<Label> updatedLabels;
    auto it = targets_.upper_bound(fake);
    while (it != targets_.end()) {
        Label label = *it;
        it = targets_.erase(it);
        *label.pc_ += bias;
        updatedLabels.push_back(label);
    }
    targets_.insert(updatedLabels.begin(), updatedLabels.end());
    pcList_.pop_front();
}

int32_t BytecodeEmitter::EstimateMaxDistance(uint32_t insnPc, uint32_t targetPc, uint32_t bias) const
{
    int32_t distance = 0;
    uint32_t endPc = 0;
    std::map<uint32_t, Label>::const_iterator it;
    if (targetPc > insnPc) {
        it = branches_.lower_bound(insnPc - bias);
        distance = static_cast<int32_t>(targetPc - insnPc);
        endPc = targetPc - bias;
    } else if (targetPc < insnPc) {
        it = branches_.lower_bound(targetPc - bias);
        distance = static_cast<int32_t>(targetPc - insnPc);
        endPc = insnPc - bias;
    } else {
        // Do we support branch to itself?
        return 0;
    }

    while (it != branches_.end() && it->first < endPc) {
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

#include <libarkfile/include/bytecode_emitter_gen.h>

}  // namespace ark
