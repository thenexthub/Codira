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

#ifndef COMPILER_OPTIMIZER_IR_CONSTANTS_H
#define COMPILER_OPTIMIZER_IR_CONSTANTS_H

#include <cstdint>
#include <limits>
#include <variant>

namespace panda::compiler {
constexpr int BITS_PER_BYTE = 8;
constexpr int BITS_PER_INSTPTR = sizeof(intptr_t) * BITS_PER_BYTE;

using PcType = uint32_t;
using LinearNumber = uint32_t;

// Update this when it will be strictly necessary to assign more than 255 registers in bytecode optimizer.
#ifdef IR_FOR_LIBARK_DEFECT_SCAN_AUX
using Register = uint16_t;
using StackSlot = uint16_t;
using ImmTableSlot = uint16_t;
#else
using Register = uint8_t;
using StackSlot = uint8_t;
using ImmTableSlot = uint8_t;
#endif
constexpr uint32_t MAX_NUM_STACK_SLOTS = std::numeric_limits<StackSlot>::max();
constexpr uint32_t MAX_NUM_IMM_SLOTS = std::numeric_limits<ImmTableSlot>::max();

constexpr uint32_t INVALID_PC = std::numeric_limits<PcType>::max();
constexpr uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();
constexpr uint32_t INVALID_VN = std::numeric_limits<uint32_t>::max();
constexpr LinearNumber INVALID_LINEAR_NUM = std::numeric_limits<LinearNumber>::max();
constexpr Register INVALID_REG = std::numeric_limits<Register>::max();
constexpr StackSlot INVALID_STACK_SLOT = std::numeric_limits<StackSlot>::max();
constexpr ImmTableSlot INVALID_IMM_TABLE_SLOT = std::numeric_limits<ImmTableSlot>::max();
constexpr std::uint32_t INVALID_COLUMN_NUM = std::numeric_limits<std::uint32_t>::max();
constexpr std::size_t CALLEE_THRESHOLD = 2;

constexpr Register VIRTUAL_FRAME_SIZE = INVALID_REG - 1U;

constexpr Register INVALID_REG_ID = std::numeric_limits<Register>::max();
constexpr Register ACC_REG_ID = INVALID_REG_ID - 1U;
constexpr uint8_t MAX_NUM_REGS = 32;
constexpr uint8_t MAX_NUM_VREGS = 32;

using LifeNumber = uint32_t;
constexpr auto INVALID_LIFE_NUMBER = std::numeric_limits<LifeNumber>::max();
constexpr auto LIFE_NUMBER_GAP = 2U;

enum ShiftType : uint8_t { LSL, LSR, ASR, ROR, INVALID_SHIFT };

enum ShiftOpcode { NEG_SR, ADD_SR, SUB_SR, AND_SR, OR_SR, XOR_SR, AND_NOT_SR, OR_NOT_SR, XOR_NOT_SR, INVALID_SR };

constexpr uint32_t MAX_SCALE = 3;

constexpr int MAX_SUCCS_NUM = 2;
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_CONSTANTS_H
