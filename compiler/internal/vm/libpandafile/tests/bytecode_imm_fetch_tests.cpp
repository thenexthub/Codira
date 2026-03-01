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

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>

#include "bytecode_instruction-inl.h"

namespace panda::test {

TEST(BytecodeInstruction, Signed)
{
    {
        // jeqz 23
        const uint8_t bytecode[] = {0x4f, 0x17};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x4f);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_V8_V8_V8, 0>()), static_cast<int8_t>(0x17));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_V8_V8_V8, 0, true>()),
                   static_cast<int8_t>(0x17));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0x17));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }

    {
        // jmp -22
        const uint8_t bytecode[] = {0x4d, 0xea};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x4d);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8, 0>()), static_cast<int8_t>(-22));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8, 0, true>()), static_cast<int8_t>(-22));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(-22));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }

    {
        // ldai 30
        const uint8_t bytecode[] = {0x62, 0x1e, 0x00, 0x00, 0x00};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x62);
        EXPECT_EQ(inst.GetFormat(), BytecodeInstruction::Format::IMM32);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM32, 0>()), static_cast<int32_t>(0x1e));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM32, 0, true>()), static_cast<int32_t>(0x1e));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0x1e));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }

    {
        // fldai 3.14
        const uint8_t bytecode[] = {0x63, 0x1f, 0x85, 0xeb, 0x51, 0xb8, 0x1e, 0x09, 0x40};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x63);
        EXPECT_EQ((bit_cast<double>(inst.GetImm<BytecodeInstruction::Format::IMM64, 0, true>())), 3.14);
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0x40091eb851eb851f));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }
}

TEST(BytecodeInstruction, UnsignedOneImm)
{
    {
        // callthis2 142, v0, v2, v3
        const uint8_t bytecode[] = {0x2f, 0x8e, 0x00, 0x02, 0x03};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x2f);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_V8_V8_V8, 0>()), static_cast<int8_t>(0x8e));
        EXPECT_NE((inst.GetImm<BytecodeInstruction::Format::IMM8_V8_V8_V8, 0>()), static_cast<uint8_t>(0x8e));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_V8_V8_V8, 0, false>()),
                   static_cast<uint8_t>(0x8e));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0x8e));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }

    {
        // neg 13
        const uint8_t bytecode[] = {0x1f, 0x0d};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x1f);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8, 0>()), static_cast<int8_t>(0x0d));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8, 0, false>()),
                   static_cast<uint8_t>(0x0d));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0x0d));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }

    {
        // tryldglobalbyname 128, id6
        const uint8_t bytecode[] = {0x8c, 0x80, 0x00, 0x06, 0x00};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x8c);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM16_ID16, 0>()), static_cast<int16_t>(0x80));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM16_ID16, 0, false>()),
                   static_cast<uint16_t>(0x80));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0x80));
        EXPECT_EQ(inst.GetImmCount(), 1);
    }
}

TEST(BytecodeInstruction, UnsignedTwoImm)
{
    {
        // stlexvar 0, 2
        const uint8_t bytecode[] = {0x3d, 0x20};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x3d);
        EXPECT_EQ(inst.GetFormat(), BytecodeInstruction::Format::IMM4_IMM4);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM4_IMM4, 0>()), 0);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM4_IMM4, 1>()), 2);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM4_IMM4, 0, false>()), 0);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM4_IMM4, 1, false>()), 2);
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(0));
        EXPECT_EQ(inst.GetImmData(1), static_cast<int64_t>(2));
        EXPECT_EQ(inst.GetImmCount(), 2);
    }

    {
        // definefunc 2, id8, 1
        const uint8_t bytecode[] = {0x33, 0x02, 0x08, 0x00, 0x01};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x33);
        EXPECT_EQ(inst.GetFormat(), BytecodeInstruction::Format::IMM8_ID16_IMM8);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_IMM8, 0>()), static_cast<int8_t>(2));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_IMM8, 1>()), static_cast<int8_t>(1));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_IMM8, 0, false>()), static_cast<int8_t>(2));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_IMM8, 1, false>()), static_cast<int8_t>(1));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(2));
        EXPECT_EQ(inst.GetImmData(1), static_cast<int64_t>(1));
        EXPECT_EQ(inst.GetImmCount(), 2);
    }

    {
        // defineclasswithbuffer 2, id9, id12, 2, v3
        const uint8_t bytecode[] = {0x35, 0x02, 0x09, 0x00, 0x0c, 0x00, 0x02, 0x00, 0x03};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<uint8_t>(inst.GetOpcode()), 0x35);
        EXPECT_EQ(inst.GetFormat(), BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8);
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8, 0>()), static_cast<int8_t>(2));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8, 1>()), static_cast<int16_t>(2));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8, 0, false>()),
                   static_cast<uint8_t>(2));
        EXPECT_EQ((inst.GetImm<BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8, 1, false>()),
                   static_cast<uint16_t>(2));
        EXPECT_EQ(inst.GetImmData(0), static_cast<int64_t>(2));
        EXPECT_EQ(inst.GetImmData(1), static_cast<int64_t>(2));
        EXPECT_EQ(inst.GetImmCount(), 2);
    }
}

TEST(BytecodeInstruction, OutputOperator)
{
    {
        const uint8_t bytecode[] = {0x4f, 0x17};
        BytecodeInstruction inst(bytecode);
        std::ostringstream oss;
        oss << inst;
        EXPECT_EQ(oss.str(), "jeqz 23");
    }

    {
        const uint8_t bytecode[] = {0x4d, 0xea};
        BytecodeInstruction inst(bytecode);
        std::ostringstream oss;
        oss << inst;
        EXPECT_EQ(oss.str(), "jmp -22");
    }

    {
        const uint8_t bytecode[] = {0x62, 0x1e, 0x00, 0x00, 0x00};
        BytecodeInstruction inst(bytecode);
        std::ostringstream oss;
        oss << inst;
        EXPECT_EQ(oss.str(), "ldai 30");
    }

    {
        const uint8_t bytecode[] = {0x63, 0x1f, 0x85, 0xeb, 0x51, 0xb8, 0x1e, 0x09, 0x40};
        BytecodeInstruction inst(bytecode);
        std::ostringstream oss;
        oss << inst;
        EXPECT_EQ(oss.str(), "fldai 3.14");
    }

    {
        const uint8_t bytecode[] = {0x2d, 0x80, 0x00};
        BytecodeInstruction inst(bytecode);
        std::ostringstream oss;
        oss << inst;
        EXPECT_EQ(oss.str(), "callthis0 128, v0");
    }
}

TEST(BytecodeInstruction, GetLiteralIndex)
{
    {
        // legal ins, and have 0 literal id
        const uint8_t bytecode[] = {0x05};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<int>(inst.GetLiteralIndex()), -1);
    }
    {
        // legal ins, and have 1 literal id in loc 0
        const uint8_t bytecode[] = {0x06, 0x00, 0x00, 0x00};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<int>(inst.GetLiteralIndex()), 0);
    }
    {
        // legal ins, and have 1 literal id in loc 1
        const uint8_t bytecode[] = {0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<int>(inst.GetLiteralIndex()), 1);
    }
    {
        // illegal ins
        const uint8_t bytecode[] = {0xff};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(static_cast<int>(inst.GetLiteralIndex()), -1);
    }
}

TEST(BytecodeInstruction, GetLastVReg)
{
    {
        // newobjrange 0xb, 0x3, v6
        const uint8_t bytecode[] = {0x08, 0x0b, 0x03, 0x06};
        BytecodeInstruction inst(bytecode);
        EXPECT_EQ(inst.GetLastVReg().value(), 6);
    }
}
}  // namespace panda::test
