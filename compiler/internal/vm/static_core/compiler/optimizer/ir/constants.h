/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_IR_CONSTANTS_H
#define COMPILER_OPTIMIZER_IR_CONSTANTS_H

#include <cstdint>
#include <limits>

#include "compiler_options.h"

namespace ark::compiler {
constexpr int BITS_PER_BYTE = 8;
constexpr int BITS_PER_INSTPTR = sizeof(intptr_t) * BITS_PER_BYTE;

using PcType = uint32_t;
using LinearNumber = uint32_t;

// Update this when it will be strictly necessary to assign more than 255 registers in bytecode optimizer.
using Register = uint16_t;
using StackSlot = uint16_t;
using ImmTableSlot = uint16_t;

constexpr uint32_t MAX_NUM_STACK_SLOTS = std::numeric_limits<uint8_t>::max();
constexpr uint32_t MAX_NUM_IMM_SLOTS = std::numeric_limits<uint8_t>::max();

constexpr uint32_t INVALID_PC = std::numeric_limits<PcType>::max();
constexpr uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();
constexpr uint32_t INVALID_VN = std::numeric_limits<uint32_t>::max();
constexpr LinearNumber INVALID_LINEAR_NUM = std::numeric_limits<LinearNumber>::max();
constexpr Register INVALID_REG = std::numeric_limits<uint8_t>::max();
constexpr StackSlot INVALID_STACK_SLOT = std::numeric_limits<uint8_t>::max();
constexpr ImmTableSlot INVALID_IMM_TABLE_SLOT = std::numeric_limits<uint8_t>::max();
constexpr std::uint32_t INVALID_COLUMN_NUM = std::numeric_limits<std::uint32_t>::max();

constexpr Register VIRTUAL_FRAME_SIZE = INVALID_REG - 1U;

using LifeNumber = uint32_t;
constexpr auto INVALID_LIFE_NUMBER = std::numeric_limits<LifeNumber>::max();
constexpr auto LIFE_NUMBER_GAP = 2U;

enum ShiftType : uint8_t { LSL, LSR, ASR, ROR, INVALID_SHIFT };

enum ShiftOpcode { NEG_SR, ADD_SR, SUB_SR, AND_SR, OR_SR, XOR_SR, AND_NOT_SR, OR_NOT_SR, XOR_NOT_SR, INVALID_SR };

constexpr uint32_t MAX_SCALE = 3;

constexpr int MAX_SUCCS_NUM = 2;

#ifdef ENABLE_LIBABCKIT
constexpr uint32_t MAX_VALUE = ((1U << (2 * BITS_PER_BYTE - 4))) * 2 - 1U;
constexpr uint32_t MAX_NUM_STACK_SLOTS_LARGE = MAX_VALUE;
constexpr uint32_t MAX_NUM_IMM_SLOTS_LARGE = MAX_VALUE;
constexpr Register INVALID_REG_LARGE = MAX_VALUE;
constexpr StackSlot INVALID_STACK_SLOT_LARGE = MAX_VALUE;
constexpr ImmTableSlot INVALID_IMM_TABLE_SLOT_LARGE = MAX_VALUE;
constexpr Register VIRTUAL_FRAME_SIZE_LARGE = INVALID_REG_LARGE - 1U;

inline bool IsFrameSizeLarge()
{
    return UNLIKELY(g_options.GetCompilerFrameSize() == "large");
}

inline uint16_t GetFrameSize()
{
    return IsFrameSizeLarge() ? VIRTUAL_FRAME_SIZE_LARGE : VIRTUAL_FRAME_SIZE;
}

inline Register GetInvalidReg()
{
    return IsFrameSizeLarge() ? INVALID_REG_LARGE : INVALID_REG;
}

inline StackSlot GetInvalidStackSlot()
{
    return IsFrameSizeLarge() ? INVALID_STACK_SLOT_LARGE : INVALID_STACK_SLOT;
}

inline ImmTableSlot GetInvalidImmTableSlot()
{
    return IsFrameSizeLarge() ? INVALID_IMM_TABLE_SLOT_LARGE : INVALID_IMM_TABLE_SLOT;
}

inline uint16_t GetMaxNumStackSlots()
{
    return IsFrameSizeLarge() ? MAX_NUM_STACK_SLOTS_LARGE : MAX_NUM_STACK_SLOTS;
}

inline uint16_t GetMaxNumImmSlots()
{
    return IsFrameSizeLarge() ? MAX_NUM_IMM_SLOTS_LARGE : MAX_NUM_IMM_SLOTS;
}

#else
constexpr Register VIRTUAL_FRAME_SIZE_LARGE = VIRTUAL_FRAME_SIZE;
constexpr uint32_t MAX_NUM_STACK_SLOTS_LARGE = MAX_NUM_STACK_SLOTS;
inline bool IsFrameSizeLarge()
{
    ASSERT(g_options.GetCompilerFrameSize() == "default");
    return false;
}

inline uint16_t GetFrameSize()
{
    return VIRTUAL_FRAME_SIZE;
}

inline Register GetInvalidReg()
{
    return INVALID_REG;
}

inline StackSlot GetInvalidStackSlot()
{
    return INVALID_STACK_SLOT;
}

inline ImmTableSlot GetInvalidImmTableSlot()
{
    return INVALID_IMM_TABLE_SLOT;
}

inline uint16_t GetMaxNumStackSlots()
{
    return MAX_NUM_STACK_SLOTS;
}

inline uint16_t GetMaxNumImmSlots()
{
    return MAX_NUM_IMM_SLOTS;
}
#endif

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_CONSTANTS_H
