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
#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <limits>
#include <tuple>
#include <vector>

namespace ark::panda_file::test {

using Opcode = BytecodeInstruction::Opcode;

using Tuple16 = std::tuple<uint8_t, uint8_t>;
using Tuple32 = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;
using Tuple64 = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>;

// NOLINTBEGIN(readability-magic-numbers,hicpp-signed-bitwise)

namespace globals {
constexpr static const int64_t IMM_2 = 2;
constexpr static const int64_t IMM_3 = 3;
constexpr static const int64_t IMM_4 = 4;
constexpr static const int64_t IMM_5 = 5;
constexpr static const int64_t IMM_6 = 6;
constexpr static const int64_t IMM_7 = 7;
constexpr static const int64_t IMM_8 = 8;
constexpr static const int64_t IMM_9 = 9;
constexpr static const int64_t IMM_11 = 11;
constexpr static const int64_t IMM_12 = 12;
constexpr static const int64_t IMM_15 = 15;
constexpr static const int64_t IMM_16 = 16;
constexpr static const int64_t IMM_24 = 24;
constexpr static const int64_t IMM_32 = 32;
constexpr static const int64_t IMM_40 = 40;
constexpr static const int64_t IMM_48 = 48;
constexpr static const int64_t IMM_56 = 56;
}  // namespace globals

static std::vector<uint8_t> &operator<<(std::vector<uint8_t> &out, uint8_t val)
{
    out.push_back(val);
    return out;
}

static std::vector<uint8_t> &operator<<(std::vector<uint8_t> &out, Opcode op);

static std::vector<uint8_t> &operator<<(std::vector<uint8_t> &out, Tuple16 val)
{
    return out << std::get<0>(val) << std::get<1>(val);
}

static std::vector<uint8_t> &operator<<(std::vector<uint8_t> &out, Tuple32 val)
{
    return out << std::get<0>(val) << std::get<1>(val) << std::get<globals::IMM_2>(val)
               << std::get<globals::IMM_3>(val);
}

static std::vector<uint8_t> &operator<<(std::vector<uint8_t> &out, Tuple64 val)
{
    return out << std::get<0>(val) << std::get<1>(val) << std::get<globals::IMM_2>(val) << std::get<globals::IMM_3>(val)
               << std::get<globals::IMM_4>(val) << std::get<globals::IMM_5>(val) << std::get<globals::IMM_6>(val)
               << std::get<globals::IMM_7>(val);
}

static Tuple16 Split16(uint16_t val)
{
    return Tuple16 {val & 0xFF, val >> 8};
}

static Tuple32 Split32(uint32_t val)
{
    return Tuple32 {val & 0xFF, (val >> globals::IMM_8) & 0xFF, (val >> globals::IMM_16) & 0xFF,
                    (val >> globals::IMM_24) & 0xFF};
}

static Tuple64 Split64(uint64_t val)
{
    return Tuple64 {val & 0xFF,
                    (val >> globals::IMM_8) & 0xFF,
                    (val >> globals::IMM_16) & 0xFF,
                    (val >> globals::IMM_24) & 0xFF,
                    (val >> globals::IMM_32) & 0xFF,
                    (val >> globals::IMM_40) & 0xFF,
                    (val >> globals::IMM_48) & 0xFF,
                    (val >> globals::IMM_56) & 0xFF};
}

TEST(BytecodeEmitter, JmpBwd_IMM8)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Bind(label);
    int numRet = -std::numeric_limits<int8_t>::min();
    for (int i = 0; i < numRet; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Jmp(label);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    for (int i = 0; i < numRet; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    expected << Opcode::JMP_IMM8 << -numRet;
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, JmpFwd_IMM8)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Jmp(label);
    // -5 because 2 bytes takes jmp itself and
    // emitter estimate length of jmp by 3 greater the it is actually
    int numRet = std::numeric_limits<int8_t>::max() - globals::IMM_5;
    for (int i = 0; i < numRet; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << Opcode::JMP_IMM8 << numRet + globals::IMM_2;
    for (int i = 0; i < numRet + 1; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, JmpBwd_IMM16)
{
    {
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitter.Bind(label);
        int numRet = -std::numeric_limits<int8_t>::min() + 1;
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitter.Jmp(label);
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        for (int i = 0; i < numRet; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        expected << Opcode::JMP_IMM16 << Split16(-numRet);
        ASSERT_EQ(expected, out);
    }
    {
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitter.Bind(label);
        int numRet = -std::numeric_limits<int16_t>::min();
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitter.Jmp(label);
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        for (int i = 0; i < numRet; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        expected << Opcode::JMP_IMM16 << Split16(-numRet);
        ASSERT_EQ(expected, out);
    }
}

TEST(BytecodeEmitter, JmpFwd_IMM16)
{
    {
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitter.Jmp(label);
        // -4 because 2 bytes takes jmp itself and
        // emitter estimate length of jmp by 3 greater the it is actually
        // and plus one byte to make 8bit overflow
        int numRet = std::numeric_limits<int8_t>::max() - globals::IMM_4;
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitter.Bind(label);
        emitter.ReturnVoid();
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        expected << Opcode::JMP_IMM16 << Split16(numRet + globals::IMM_3);
        for (int i = 0; i < numRet + 1; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        ASSERT_EQ(expected, out);
    }
    {
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitter.Jmp(label);
        // -5 because 2 bytes takes jmp itself and
        // emitter estimate length of jmp by 3 greater the it is actually
        int numRet = std::numeric_limits<int16_t>::max() - globals::IMM_5;
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitter.Bind(label);
        emitter.ReturnVoid();
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        expected << Opcode::JMP_IMM16 << Split16(numRet + globals::IMM_3);
        for (int i = 0; i < numRet + 1; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        ASSERT_EQ(expected, out);
    }
}

static void EmitJmp(Opcode op, int32_t imm, std::vector<uint8_t> *out)
{
    *out << op;
    switch (op) {
        case Opcode::JMP_IMM8:
            *out << static_cast<int8_t>(imm);
            break;
        case Opcode::JMP_IMM16:
            *out << Split16(imm);
            break;
        default:
            *out << Split32(imm);
            break;
    }
}

static Opcode GetOpcode(size_t instSize)
{
    switch (instSize) {
        case globals::IMM_2:
            return Opcode::JMP_IMM8;
        case globals::IMM_3:
            return Opcode::JMP_IMM16;
        default:
            return Opcode::JMP_IMM32;
    }
}

/*
 * Emit bytecode for the following program:
 *
 * label1:
 * jmp label2
 * ...          <- n1 return.void instructions
 * jmp label1
 * ...          <- n2 return.void instructions
 * label2:
 * return.void
 */
// CC-OFFNXT(huge_method[C++], huge_depth[C++]) solid logic
static std::vector<uint8_t> EmitJmpFwdBwd(size_t n1, size_t n2)
{
    std::array<std::tuple<size_t, int32_t, int32_t>, 3U> jmps {
        std::tuple {globals::IMM_2, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max()},
        std::tuple {globals::IMM_3, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()},
        std::tuple {globals::IMM_5, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()}};

    std::vector<uint8_t> out;

    size_t jmpSize1;
    size_t jmpSize2;
    int32_t immMax1;
    int32_t immMin2;

    for (const auto &t1 : jmps) {
        std::tie(jmpSize1, std::ignore, immMax1) = t1;

        for (const auto &t2 : jmps) {
            std::tie(jmpSize2, immMin2, std::ignore) = t2;

            int32_t imm1 = jmpSize1 + n1 + jmpSize2 + n2;
            int32_t imm2 = jmpSize1 + n1;

            if (imm1 <= immMax1 && -imm2 >= immMin2) {
                EmitJmp(GetOpcode(jmpSize1), imm1, &out);

                for (size_t i = 0; i < n1; i++) {
                    out << Opcode::RETURN_VOID;
                }

                EmitJmp(GetOpcode(jmpSize2), -imm2, &out);

                for (size_t i = 0; i < n2; i++) {
                    out << Opcode::RETURN_VOID;
                }

                out << Opcode::RETURN_VOID;

                return out;
            }
        }
    }

    return out;
}

/*
// CC-OFFNXT(G.CMT.04-CPP) false positive
 * Test following control flow:
 *
 * label1:
 * jmp label2
 * ...          <- n1 return.void instructions
 * jmp label1
 * ...          <- n2 return.void instructions
 * label2:
 * return.void
 */
void TestJmpFwdBwd(size_t n1, size_t n2)
{
    BytecodeEmitter emitter;
    Label label1 = emitter.CreateLabel();
    Label label2 = emitter.CreateLabel();

    emitter.Bind(label1);
    emitter.Jmp(label2);
    for (size_t i = 0; i < n1; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Jmp(label1);
    for (size_t i = 0; i < n2; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Bind(label2);
    emitter.ReturnVoid();

    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out)) << "n1 = " << n1 << " n2 = " << n2;

    ASSERT_EQ(EmitJmpFwdBwd(n1, n2), out) << "n1 = " << n1 << " n2 = " << n2;
}

TEST(BytecodeEmitter, JmpFwdBwd)
{
    // fwd jmp imm16
    // bwd jmp imm8
    TestJmpFwdBwd(0, std::numeric_limits<int8_t>::max());

    // fwd jmp imm16
    // bwd jmp imm16
    TestJmpFwdBwd(std::numeric_limits<int8_t>::max(), 0);

    // fwd jmp imm32
    // bwd jmp imm8
    TestJmpFwdBwd(0, std::numeric_limits<int16_t>::max());

    // fwd jmp imm32
    // bwd jmp imm16
    TestJmpFwdBwd(std::numeric_limits<int8_t>::max(), std::numeric_limits<int16_t>::max());

    // fwd jmp imm32
    // bwd jmp imm32
    TestJmpFwdBwd(std::numeric_limits<int16_t>::max(), 0);
}

TEST(BytecodeEmitter, JmpBwd_IMM32)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Bind(label);
    int numRet = -std::numeric_limits<int16_t>::min() + 1;
    for (int i = 0; i < numRet; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Jmp(label);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    for (int i = 0; i < numRet; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    expected << Opcode::JMP_IMM32 << Split32(-numRet);
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, JmpFwd_IMM32)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Jmp(label);
    // -4 because 2 bytes takes jmp itself and
    // emitter estimate length of jmp by 3 greater the it is actually
    // and plus one byte to make 16bit overflow
    int numRet = std::numeric_limits<int16_t>::max() - globals::IMM_4;
    for (int i = 0; i < numRet; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << Opcode::JMP_IMM32 << Split32(numRet + globals::IMM_5);
    for (int i = 0; i < numRet + 1; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    ASSERT_EQ(expected, out);
}

// NOLINTNEXTLINE(readability-identifier-naming)
static void JcmpBwd_V8_IMM8(Opcode opcode,
                            const std::function<void(BytecodeEmitter *, uint8_t, const Label &label)> &emitJcmp)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Bind(label);
    int numRet = globals::IMM_15;
    for (int i = 0; i < numRet; ++i) {
        emitter.ReturnVoid();
    }
    emitJcmp(&emitter, globals::IMM_15, label);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    for (int i = 0; i < numRet; ++i) {
        expected << Opcode::RETURN_VOID;
    }

    expected << opcode << 15U << static_cast<uint8_t>(-numRet);
    ASSERT_EQ(expected, out);
}

// NOLINTNEXTLINE(readability-identifier-naming)
static void JcmpFwd_V8_IMM8(Opcode opcode,
                            const std::function<void(BytecodeEmitter *, uint8_t, const Label &label)> &emitJcmp)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitJcmp(&emitter, globals::IMM_15, label);
    int numRet = globals::IMM_12;
    for (int i = 0; i < numRet; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    // 2 bytes takes jmp itself and plus one byte to make.
    expected << opcode << 15U << static_cast<uint8_t>(numRet + globals::IMM_3);

    for (int i = 0; i < numRet + 1; ++i) {
        expected << Opcode::RETURN_VOID;
    }

    ASSERT_EQ(expected, out);
}

// NOLINTNEXTLINE(readability-identifier-naming)
static void JcmpBwd_V8_IMM16(Opcode opcode,
                             const std::function<void(BytecodeEmitter *, uint8_t, const Label &label)> &emitJcmp)
{
    {
        // Test min imm value
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitter.Bind(label);
        int numRet = -std::numeric_limits<int8_t>::min();
        ++numRet;
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitJcmp(&emitter, 0, label);
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        for (int i = 0; i < numRet; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        expected << opcode << 0U << Split16(-numRet);
        ASSERT_EQ(expected, out);
    }
    {
        // Test max imm value
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitter.Bind(label);
        int numRet = -std::numeric_limits<int16_t>::min();
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitJcmp(&emitter, 0, label);
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        for (int i = 0; i < numRet; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        expected << opcode << 0U << Split16(-numRet);
        ASSERT_EQ(expected, out);
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
static void JcmpFwd_V8_IMM16(Opcode opcode,
                             const std::function<void(BytecodeEmitter *, uint8_t, const Label &label)> &emitJcmp)
{
    {
        // Test min imm
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitJcmp(&emitter, 0, label);
        // -3 because 4 bytes takes jmp itself
        // plus one to make 8bit overflow
        int numRet = std::numeric_limits<int8_t>::max() - globals::IMM_3;
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitter.Bind(label);
        emitter.ReturnVoid();
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        expected << opcode << 0U << Split16(numRet + globals::IMM_4);
        for (int i = 0; i < numRet + 1; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        ASSERT_EQ(expected, out);
    }
    {
        // Test max imm
        BytecodeEmitter emitter;
        Label label = emitter.CreateLabel();
        emitJcmp(&emitter, 0, label);
        // -4 because 4 bytes takes jmp itself
        int numRet = std::numeric_limits<int16_t>::max() - globals::IMM_4;
        for (int i = 0; i < numRet; ++i) {
            emitter.ReturnVoid();
        }
        emitter.Bind(label);
        emitter.ReturnVoid();
        std::vector<uint8_t> out;
        ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
        std::vector<uint8_t> expected;
        expected << opcode << 0U << Split16(numRet + globals::IMM_4);
        for (int i = 0; i < numRet + 1; ++i) {
            expected << Opcode::RETURN_VOID;
        }
        ASSERT_EQ(expected, out);
    }
}

TEST(BytecodeEmitter, Jne_V8_IMM8)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Jne(0, label);
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << Opcode::JNE_V8_IMM8 << 0 << globals::IMM_3 << Opcode::RETURN_VOID;
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, Jne_V8_IMM16)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    static constexpr uint8_t JNE_REG = 16;
    static constexpr size_t N_NOPS = 256;
    static constexpr size_t INSN_SZ = 4;
    emitter.Jne(JNE_REG, label);
    for (size_t i = 0; i < N_NOPS; ++i) {
        emitter.Nop();
    }
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << Opcode::JNE_V8_IMM16 << JNE_REG << Split16(INSN_SZ + N_NOPS);
    for (size_t i = 0; i < N_NOPS; ++i) {
        expected << Opcode::NOP;
    }
    expected << Opcode::RETURN_VOID;
    ASSERT_EQ(expected, out);
}

// NOLINTNEXTLINE(readability-identifier-naming)
static void Jcmpz_IMM8(Opcode opcode, const std::function<void(BytecodeEmitter *, const Label &label)> &emit_jcmp)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emit_jcmp(&emitter, label);
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << opcode << globals::IMM_2 << Opcode::RETURN_VOID;
    ASSERT_EQ(expected, out);
}

// NOLINTNEXTLINE(readability-identifier-naming)
static void Jcmpz_IMM16(Opcode opcode, const std::function<void(BytecodeEmitter *, const Label &label)> &emit_jcmp)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emit_jcmp(&emitter, label);
    for (int i = 0; i < std::numeric_limits<uint8_t>::max() - globals::IMM_2; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Bind(label);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << opcode << Split16(std::numeric_limits<uint8_t>::max() + 1);
    for (int i = 0; i < std::numeric_limits<uint8_t>::max() - 1; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, JmpFwdCrossRef)
{
    // Situation:
    //         +---------+
    //    +----|----+    |
    //    |    |    |    |
    //    |    |    v    v
    // ---*----*----*----*----
    //             lbl1 lbl2
    BytecodeEmitter emitter;
    Label lbl1 = emitter.CreateLabel();
    Label lbl2 = emitter.CreateLabel();
    emitter.Jeq(0, lbl1);
    emitter.Jeq(0, lbl2);
    emitter.ReturnVoid();
    emitter.Bind(lbl1);
    emitter.ReturnVoid();
    for (int i = 0; i < globals::IMM_6; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Bind(lbl2);
    emitter.ReturnVoid();
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));

    std::vector<uint8_t> expected;
    expected << Opcode::JEQ_V8_IMM8 << 0U << globals::IMM_7 << Opcode::JEQ_V8_IMM8 << 0U << globals::IMM_11;
    for (int i = 0; i < globals::IMM_9; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, JmpBwdCrossRef)
{
    // Situation:
    //         +---------+
    //         |         |
    //    +---------+    |
    //    |    |    |    |
    //    v    |    |    v
    // ---*----*----*----*----
    //   lbl1           lbl2
    BytecodeEmitter emitter;
    Label lbl1 = emitter.CreateLabel();
    Label lbl2 = emitter.CreateLabel();
    emitter.Bind(lbl1);
    emitter.ReturnVoid();
    emitter.Jeq(0, lbl2);
    for (int i = 0; i < globals::IMM_5; ++i) {
        emitter.ReturnVoid();
    }
    emitter.Jeq(0, lbl1);
    emitter.Bind(lbl2);
    emitter.ReturnVoid();

    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));

    std::vector<uint8_t> expected;
    expected << Opcode::RETURN_VOID << Opcode::JEQ_V8_IMM8 << 0U << globals::IMM_11;
    for (int i = 0; i < globals::IMM_5; ++i) {
        expected << Opcode::RETURN_VOID;
    }
    expected << Opcode::JEQ_V8_IMM8 << 0U << static_cast<uint8_t>(-globals::IMM_9) << Opcode::RETURN_VOID;
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, Jmp3FwdCrossRefs)
{
    // Situation:
    //     +--------+
    //    +|--------+
    //    ||+-------+---+
    //    |||       v   v
    // ---***-------*---*
    //            lbl1 lbl2
    BytecodeEmitter emitter;
    Label lbl1 = emitter.CreateLabel();
    Label lbl2 = emitter.CreateLabel();

    emitter.Jmp(lbl1);
    emitter.Jmp(lbl1);
    emitter.Jmp(lbl2);

    constexpr int32_t INT8T_MAX = std::numeric_limits<int8_t>::max();

    size_t n = INT8T_MAX - globals::IMM_4;
    for (size_t i = 0; i < n; i++) {
        emitter.ReturnVoid();
    }

    emitter.Bind(lbl1);
    emitter.ReturnVoid();
    emitter.ReturnVoid();
    emitter.Bind(lbl2);

    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));

    std::vector<uint8_t> expected;
    expected << Opcode::JMP_IMM16 << Split16(INT8T_MAX + globals::IMM_5);
    expected << Opcode::JMP_IMM16 << Split16(INT8T_MAX + globals::IMM_2);
    expected << Opcode::JMP_IMM16 << Split16(INT8T_MAX + 1);
    for (size_t i = 0; i < n + globals::IMM_2; i++) {
        expected << Opcode::RETURN_VOID;
    }
    ASSERT_EQ(expected, out);
}

TEST(BytecodeEmitter, UnboundLabel)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Bind(label);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
}

TEST(BytecodeEmitter, JumpToUnboundLabel)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Jmp(label);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::UNBOUND_LABELS, emitter.Build(&out));
}

TEST(BytecodeEmitter, JumpToUnboundLabel2)
{
    BytecodeEmitter emitter;
    Label label1 = emitter.CreateLabel();
    Label label2 = emitter.CreateLabel();
    emitter.Jmp(label1);
    emitter.Bind(label2);
    emitter.Mov(0, 1);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::UNBOUND_LABELS, emitter.Build(&out));
}

TEST(BytecodeEmitter, TwoJumpsToOneLabel)
{
    BytecodeEmitter emitter;
    Label label = emitter.CreateLabel();
    emitter.Bind(label);
    emitter.Mov(0, 1);
    emitter.Jmp(label);
    emitter.Jmp(label);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
}

static void TestNoneFormat(Opcode opcode, const std::function<void(BytecodeEmitter *)> &emit)
{
    BytecodeEmitter emitter;
    emit(&emitter);
    std::vector<uint8_t> out;
    ASSERT_EQ(BytecodeEmitter::ErrorCode::SUCCESS, emitter.Build(&out));
    std::vector<uint8_t> expected;
    expected << opcode;
    ASSERT_EQ(expected, out);
}

// NOLINTEND(readability-magic-numbers,hicpp-signed-bitwise)

#include <bytecode_emitter_tests_gen.h>

}  // namespace ark::panda_file::test
