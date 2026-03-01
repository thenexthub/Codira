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

#include <cstdint>
#include <gtest/gtest.h>
#include <type_traits>

#include "disassembler.h"

namespace panda::disasm {

panda_file::File::EntityId method_id {0x00};
panda::disasm::Disassembler disasm {};

TEST(BytecodeInstruction, Signed)
{
    {
        // jeqz 23
        const uint8_t bytecode[] = {0x4f, 0x17};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x4f);
        EXPECT_EQ(std::stoi(ins->GetId(0)), static_cast<int64_t>(0x17));
    }

    {
        // jmp -22
        const uint8_t bytecode[] = {0x4d, 0xea};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x4d);
        EXPECT_EQ(std::stoi(ins->GetId(0)), static_cast<int8_t>(-22));
    }

    {
        // ldai 30
        const uint8_t bytecode[] = {0x62, 0x1e, 0x00, 0x00, 0x00};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x62);
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(0)), static_cast<int32_t>(0x1e));
    }

    {
        // fldai 3.14
        const uint8_t bytecode[] = {0x63, 0x1f, 0x85, 0xeb, 0x51, 0xb8, 0x1e, 0x09, 0x40};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x63);
        EXPECT_EQ(std::get<double>(ins->GetImm(0)), 3.14);
    }
}

TEST(BytecodeInstruction, Unsigned)
{
    {
        // callthis2 142, v0, v2, v3
        const uint8_t bytecode[] = {0x2f, 0x8e, 0x00, 0x02, 0x03};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x2f);
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(0)), static_cast<uint8_t>(0x8e));
    }

    {
        // neg 13
        const uint8_t bytecode[] = {0x1f, 0x0d};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x1f);
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(0)), static_cast<uint8_t>(0x0d));
    }

    {
        // stlexvar 0, 2
        const uint8_t bytecode[] = {0x3d, 0x20};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x3d);
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(0)), 0);
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(1)), 2);
    }

    {
        // newobjrange 17, 1, v11
        const uint8_t bytecode[] = {0x08, 0x11, 0x01, 0x0b};
        BytecodeInstruction inst(bytecode);
        const panda::pandasm::Ins *ins = disasm.BytecodeInstructionToPandasmInstruction(inst, method_id);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x08);
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(0)), static_cast<int8_t>(0x11));
        EXPECT_EQ(std::get<int64_t>(ins->GetImm(1)), static_cast<int8_t>(0x01));
    }
}

}  // namespace panda::disasm

