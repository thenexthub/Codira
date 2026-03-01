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

#include <functional>
#include <string>
#include <unordered_map>

#include "libarkbase/utils/utils.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/codegen_native.h"
#include "optimizer/code_generator/method_properties.h"
#include "utils-vixl.h"
#include "aarch64/decoder-aarch64.h"
#include "aarch64/disasm-aarch64.h"
#include "aarch64/operands-aarch64.h"
#include "tests/unit_test.h"
#include <regex>

namespace ark::compiler {

// NOLINTNEXTLINE(misc-unused-using-decls)
using vixl::operator""_h;

class VixlDisasmTest : public GraphTest {
public:
    VixlDisasmTest() : decoder_(GetAllocator()), disasm_(GetAllocator()) {}

    auto &GetDecoder()
    {
        return decoder_;
    }

    auto &GetDisasm()
    {
        return disasm_;
    }

private:
    vixl::aarch64::Decoder decoder_;
    vixl::aarch64::Disassembler disasm_;
};

class CodegenCallerSavedRegistersTest : public VixlDisasmTest {
public:
    void BuildGraphSaveOnlyLiveRegisters(int argsCount);
};

class DecoderVisitor : public vixl::aarch64::DecoderVisitor {
public:
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE(A) \
    virtual void Visit##A([[maybe_unused]] const vixl::aarch64::Instruction *instr) {}
    VISITOR_LIST(DECLARE)
#undef DECLARE

    void Visit(vixl::aarch64::Metadata *metadata, const vixl::aarch64::Instruction *instr) final
    {
        const auto &form = (*metadata)["form"];
        auto hash = vixl::Hash(form.c_str());
        auto visitorIt {FORM_TO_VISITOR.find(hash)};
        ASSERT(visitorIt != std::end(FORM_TO_VISITOR));

        const auto &visitor {visitorIt->second};
        ASSERT(visitor != nullptr);
        visitor(this, instr);
    }

private:
    using FormToVisitorFnMap = FormToVisitorFnMapT<DecoderVisitor>;

    static const FormToVisitorFnMap FORM_TO_VISITOR;
};

class LoadStoreRegistersCollector : public DecoderVisitor {
public:
    // use the same body for all VisitXXX methods to simplify visitor's implementation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE(A)                                                                                                 \
    void Visit##A(const vixl::aarch64::Instruction *instr) override                                                \
    {                                                                                                              \
        if (std::string(#A) == "LoadStorePairOffset") {                                                            \
            VisitLoadStorePair(instr);                                                                             \
        } else if (std::string(#A) == "LoadStoreUnscaledOffset" || std::string(#A) == "LoadStoreUnsignedOffset" || \
                   std::string(#A) == "LoadStoreRegisterOffset") {                                                 \
            VisitLoadStore(instr);                                                                                 \
        }                                                                                                          \
    }

    VISITOR_LIST(DECLARE)
#undef DECLARE
    RegMask GetAccessedRegisters()
    {
        return regs_;
    }

    VRegMask GetAccessedVRegisters()
    {
        return vregs_;
    }

private:
    void VisitLoadStorePair(const vixl::aarch64::Instruction *instr)
    {
        if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_x) == vixl::aarch64::LoadStorePairOp::LDP_x ||
            instr->Mask(vixl::aarch64::LoadStorePairOp::STP_x) == vixl::aarch64::LoadStorePairOp::STP_x) {
            regs_.set(instr->GetRt());
            regs_.set(instr->GetRt2());
        } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_d) == vixl::aarch64::LoadStorePairOp::LDP_d ||
                   instr->Mask(vixl::aarch64::LoadStorePairOp::STP_d) == vixl::aarch64::LoadStorePairOp::STP_d) {
            vregs_.set(instr->GetRt());
            vregs_.set(instr->GetRt2());
        }
    }

    void VisitLoadStore(const vixl::aarch64::Instruction *instr)
    {
        if (instr->Mask(vixl::aarch64::LoadStoreOp::LDR_x) == vixl::aarch64::LoadStoreOp::LDR_x ||
            instr->Mask(vixl::aarch64::LoadStoreOp::STR_x) == vixl::aarch64::LoadStoreOp::STR_x) {
            regs_.set(instr->GetRt());
        } else if (instr->Mask(vixl::aarch64::LoadStoreOp::LDR_d) == vixl::aarch64::LoadStoreOp::LDR_d ||
                   instr->Mask(vixl::aarch64::LoadStoreOp::STR_d) == vixl::aarch64::LoadStoreOp::STR_d) {
            vregs_.set(instr->GetRt());
        }
    }

private:
    RegMask regs_;
    VRegMask vregs_;
};

class LoadStoreInstCollector : public DecoderVisitor {
public:
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE(A)                                                  \
    void Visit##A(const vixl::aarch64::Instruction *instr) override \
    {                                                               \
        if (std::string(#A) == "LoadStorePairOffset") {             \
            VisitLoadStorePair(instr);                              \
        }                                                           \
    }

    VISITOR_LIST(DECLARE)
#undef DECLARE
    auto GetLdpX()
    {
        return ldp_;
    }

    auto GetStpX()
    {
        return stp_;
    }

    auto GetLdpD()
    {
        return ldpV_;
    }

    auto GetStpD()
    {
        return stpV_;
    }

    RegMask GetAccessedPairRegisters()
    {
        return regs_;
    }

    VRegMask GetAccessedPairVRegisters()
    {
        return vregs_;
    }

    void VisitLoadStorePair(const vixl::aarch64::Instruction *instr)
    {
        if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_x) == vixl::aarch64::LoadStorePairOp::LDP_x) {
            ldp_++;
            regs_.set(instr->GetRt());
            regs_.set(instr->GetRt2());
        } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::STP_x) == vixl::aarch64::LoadStorePairOp::STP_x) {
            stp_++;
            regs_.set(instr->GetRt());
            regs_.set(instr->GetRt2());
        } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::LDP_d) == vixl::aarch64::LoadStorePairOp::LDP_d) {
            ldpV_++;
            vregs_.set(instr->GetRt());
            vregs_.set(instr->GetRt2());
        } else if (instr->Mask(vixl::aarch64::LoadStorePairOp::STP_d) == vixl::aarch64::LoadStorePairOp::STP_d) {
            stpV_++;
            vregs_.set(instr->GetRt());
            vregs_.set(instr->GetRt2());
        }
    }

private:
    size_t ldp_ {0};
    size_t stp_ {0};
    size_t ldpV_ {0};
    size_t stpV_ {0};
    RegMask regs_;
    VRegMask vregs_;
};

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays)

void CodegenCallerSavedRegistersTest::BuildGraphSaveOnlyLiveRegisters(int argsCount)
{
    GRAPH(GetGraph())
    {
        for (int i = 0; i < argsCount; i++) {
            PARAMETER(i, i).u64();
        }
        for (int i = 0; i < argsCount; i++) {
            PARAMETER(i + argsCount, i + argsCount).f64();
        }

        BASIC_BLOCK(2U, -1)
        {
            INST(16U, Opcode::Add).u64().Inputs(0, 1U);
            INST(17U, Opcode::Add).u64().Inputs(16U, 2U);
            INST(18U, Opcode::Add).u64().Inputs(17U, 3U);
            INST(19U, Opcode::Add).u64().Inputs(18U, 4U);
            INST(20U, Opcode::Add).u64().Inputs(19U, 5U);
            INST(21U, Opcode::Add).u64().Inputs(20U, 6U);
            INST(22U, Opcode::Add).u64().Inputs(21U, 7U);
            INST(23U, Opcode::Add).f64().Inputs(8U, 9U);
            INST(24U, Opcode::Add).f64().Inputs(23U, 10U);
            INST(25U, Opcode::Add).f64().Inputs(24U, 11U);
            INST(26U, Opcode::Add).f64().Inputs(25U, 12U);
            INST(27U, Opcode::Add).f64().Inputs(26U, 13U);
            INST(28U, Opcode::Add).f64().Inputs(27U, 14U);
            INST(29U, Opcode::Add).f64().Inputs(28U, 15U);
            INST(30U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(29U);
            INST(31U, Opcode::Add).u64().Inputs(30U, 22U);

            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(32U, Opcode::NewArray).ref().TypeId(8U).Inputs(44U, 31U);
            INST(33U, Opcode::Return).ref().Inputs(32U);
        }
    }
}

TEST_F(CodegenCallerSavedRegistersTest, SaveOnlyLiveRegisters)
{
    g_options.SetCompilerSaveOnlyLiveRegisters(true);
    constexpr auto ARGS_COUNT = 8;
    BuildGraphSaveOnlyLiveRegisters(ARGS_COUNT);

    SetNumArgs(ARGS_COUNT * 2U);
    SetNumVirtRegs(0);
    GraphChecker(GetGraph()).Check();
    RegAlloc(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Codegen>());

    auto codeEntry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetCode().Data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    auto &decoder {GetDecoder()};
    LoadStoreRegistersCollector visitor;
    vixl::aarch64::Decoder::ScopedAddVisitors sv(decoder, {&visitor});
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto instr = codeEntry; instr < codeExit; instr += vixl::aarch64::kInstructionSize) {
        decoder.Decode(instr);
    }
    // not using reg lists from vixl::aarch64 to check only r0-r7
    constexpr auto CALLER_REGS = RegMask((1 << ARGS_COUNT) - 1);  // NOLINT(hicpp-signed-bitwise)
    EXPECT_EQ(visitor.GetAccessedRegisters() & CALLER_REGS, RegMask {0});
    EXPECT_TRUE((visitor.GetAccessedVRegisters() & CALLER_REGS).none());
}

class CodegenSpillFillCoalescingTest : public VixlDisasmTest {
public:
    CodegenSpillFillCoalescingTest()
    {
        g_options.SetCompilerSpillFillPair(true);
        g_options.SetCompilerVerifyRegalloc(false);
    }

    void FillSpillFillInst(unsigned int alignmentOffset)
    {
        auto sfInst = INS(0).CastToSpillFill();
        sfInst->AddSpill(0U, 0U + alignmentOffset, DataType::Type::INT64);
        sfInst->AddSpill(1U, 1U + alignmentOffset, DataType::Type::INT64);
        sfInst->AddSpill(0U, 2U + alignmentOffset, DataType::Type::FLOAT64);
        sfInst->AddSpill(1U, 3U + alignmentOffset, DataType::Type::FLOAT64);
        sfInst->AddFill(4U + alignmentOffset, 3U, DataType::Type::INT64);
        sfInst->AddFill(5U + alignmentOffset, 2U, DataType::Type::INT64);
        sfInst->AddFill(6U + alignmentOffset, 3U, DataType::Type::FLOAT64);
        sfInst->AddFill(7U + alignmentOffset, 2U, DataType::Type::FLOAT64);
    }

    void CheckSpillFillCoalescingForEvenRegsNumber(bool aligned)
    {
        GRAPH(GetGraph())
        {
            BASIC_BLOCK(2U, -1)
            {
                INST(0, Opcode::SpillFill);
                INST(1, Opcode::ReturnVoid);
            }
        }

        unsigned int alignmentOffset = aligned ? 1 : 0;
        FillSpillFillInst(alignmentOffset);

        SetNumArgs(0);
        SetNumVirtRegs(0);
        GraphChecker(GetGraph()).Check();
        GetGraph()->SetStackSlotsCount(8U + alignmentOffset);
        RegAlloc(GetGraph());
        ASSERT_TRUE(GetGraph()->RunPass<Codegen>());

        auto codeEntry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + GetGraph()->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        LoadStoreInstCollector visitor;
        auto &decoder {GetDecoder()};
        vixl::aarch64::Decoder::ScopedAddVisitors sv(decoder, {&visitor});
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto instr = codeEntry; instr < codeExit; instr += vixl::aarch64::kInstructionSize) {
            decoder.Decode(instr);
        }
        EXPECT_EQ(visitor.GetStpX(), 1 /* 1 use pre increment and not counted */);
        EXPECT_EQ(visitor.GetLdpX(), 1 /* 1 use post increment and not counted */);
        EXPECT_EQ(visitor.GetLdpD(), 1);
        EXPECT_EQ(visitor.GetStpD(), 1);

        constexpr auto TEST_REGS = RegMask(0xF);
        EXPECT_EQ(visitor.GetAccessedPairRegisters() & TEST_REGS, RegMask {0xF});
        EXPECT_EQ(visitor.GetAccessedPairVRegisters() & TEST_REGS, RegMask {0xF});
    }

    void FormSpillFillInst(SpillFillInst *sfInst, unsigned int alignmentOffset)
    {
        sfInst->AddSpill(0U, 0U + alignmentOffset, DataType::Type::INT64);
        sfInst->AddSpill(1U, 1U + alignmentOffset, DataType::Type::INT64);
        sfInst->AddSpill(2U, 2U + alignmentOffset, DataType::Type::INT64);
        sfInst->AddSpill(0U, 3U + alignmentOffset, DataType::Type::FLOAT64);
        sfInst->AddSpill(1U, 4U + alignmentOffset, DataType::Type::FLOAT64);
        sfInst->AddSpill(2U, 5U + alignmentOffset, DataType::Type::FLOAT64);
        sfInst->AddFill(6U + alignmentOffset, 3U, DataType::Type::INT64);
        sfInst->AddFill(7U + alignmentOffset, 4U, DataType::Type::INT64);
        sfInst->AddFill(8U + alignmentOffset, 5U, DataType::Type::INT64);
        sfInst->AddFill(9U + alignmentOffset, 3U, DataType::Type::FLOAT64);
        sfInst->AddFill(10U + alignmentOffset, 4U, DataType::Type::FLOAT64);
        sfInst->AddFill(11U + alignmentOffset, 5U, DataType::Type::FLOAT64);
    }

    void CheckSpillFillCoalescingForOddRegsNumber(bool aligned)
    {
        GRAPH(GetGraph())
        {
            BASIC_BLOCK(2U, -1)
            {
                INST(0, Opcode::SpillFill);
                INST(1, Opcode::ReturnVoid);
            }
        }

        int alignmentOffset = aligned ? 1 : 0;

        auto sfInst = INS(0).CastToSpillFill();
        FormSpillFillInst(sfInst, alignmentOffset);

        SetNumArgs(0);
        SetNumVirtRegs(0);
        GraphChecker(GetGraph()).Check();
        GetGraph()->SetStackSlotsCount(12U + alignmentOffset);
        RegAlloc(GetGraph());
        ASSERT_TRUE(GetGraph()->RunPass<Codegen>());

        auto codeEntry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + GetGraph()->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        auto &decoder {GetDecoder()};
        LoadStoreInstCollector visitor;
        vixl::aarch64::Decoder::ScopedAddVisitors sv(decoder, {&visitor});
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto instr = codeEntry; instr < codeExit; instr += vixl::aarch64::kInstructionSize) {
            decoder.Decode(instr);
        }
        EXPECT_EQ(visitor.GetStpX(), 1 /* 1 use pre increment and not counted */);
        EXPECT_EQ(visitor.GetLdpX(), 1 /* 1 use post increment and not counted */);
        EXPECT_EQ(visitor.GetLdpD(), 1);
        EXPECT_EQ(visitor.GetStpD(), 1);

        constexpr auto TEST_REGS = RegMask(0x3F);
        if (aligned) {
            EXPECT_EQ(visitor.GetAccessedPairRegisters() & TEST_REGS, RegMask {0b11011});
            EXPECT_EQ(visitor.GetAccessedPairVRegisters() & TEST_REGS, RegMask {0b110110});
        } else {
            EXPECT_EQ(visitor.GetAccessedPairRegisters() & TEST_REGS, RegMask {0b110110});
            EXPECT_EQ(visitor.GetAccessedPairVRegisters() & TEST_REGS, RegMask {0b11011});
        }
    }
};

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToUnalignedNeighborSlotsEvenRegsNumber)
{
    CheckSpillFillCoalescingForEvenRegsNumber(false);
}

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToAlignedNeighborSlotsEvenRegsNumber)
{
    CheckSpillFillCoalescingForEvenRegsNumber(true);
}

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToUnalignedNeighborSlotsOddRegsNumber)
{
    CheckSpillFillCoalescingForOddRegsNumber(false);
}

TEST_F(CodegenSpillFillCoalescingTest, CoalesceAccessToAlignedNeighborSlotsOddRegsNumber)
{
    CheckSpillFillCoalescingForOddRegsNumber(true);
}

// clang-format off
class CodegenLeafPrologueTest : public VixlDisasmTest {
public:
    CodegenLeafPrologueTest()
    {
        g_options.SetCompilerVerifyRegalloc(false);
#ifndef NDEBUG
        graph_->SetLowLevelInstructionsEnabled();
#endif
    }

    std::vector<std::string> CheckLeafPrologueExpectedAsm()
    {
        return {
#ifdef PANDA_COMPILER_DEBUG_INFO
            "stp x29, x30, [sp, #-16]!",  // prolog save FP and LR
            "mov x29, sp",                // prolog set FP
            "stur x19, [sp, #-72]",   // prolog save callee-saved
#else
            "stur x19, [sp, #-88]",   // prolog save callee-saved
#endif
            "ldr x19, [x1]",
            "add x19, x19, #0x1 // (1)",
            "str x19, [x1]",
#ifdef PANDA_COMPILER_DEBUG_INFO
            "ldur x19, [sp, #-72]",   // epilog restore callee-saved
            "ldp x29, x30, [sp], #16",    // epilog restore FP and LR
#else
            "ldur x19, [sp, #-88]",   // epilog restore callee-saved
#endif
            "ret"};
    }

    void CheckLeafPrologue()
    {
        // RedundantOps::inc()
        RuntimeInterface::FieldPtr i = reinterpret_cast<void *>(0xDEADBEEF);  // NOLINT(modernize-use-auto)
        GRAPH(GetGraph())
        {
            PARAMETER(0, 0).ref();
            BASIC_BLOCK(2U, 3_I)
            {
                INST(1U, Opcode::LoadObject).s64().Inputs(0).TypeId(208U).ObjField(i);
                INST(2U, Opcode::AddI).s64().Inputs(1).Imm(1);
                INST(3U, Opcode::StoreObject).s64().Inputs(0, 2_I).TypeId(208U).ObjField(i);
            }
            BASIC_BLOCK(3U, -1)
            {
                INST(4U, Opcode::ReturnVoid);
            }
        }
        SetNumArgs(1);

        auto expectedAsm = CheckLeafPrologueExpectedAsm();

        GraphChecker(GetGraph()).Check();
        GetGraph()->RunPass<RegAllocLinearScan>();
        bool setupFrame = GetGraph()->GetMethodProperties().GetRequireFrameSetup();
        ASSERT_TRUE(setupFrame ? GetGraph()->RunPass<Codegen>() : GetGraph()->RunPass<CodegenNative>());
        ASSERT_TRUE(GetGraph()->GetCode().Size() == expectedAsm.size() * vixl::aarch64::kInstructionSize);
        auto codeEntry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + GetGraph()->GetCode().Size();
        size_t codeItems = (codeExit - codeEntry) / vixl::aarch64::kInstructionSize;
        ASSERT_TRUE(codeItems == expectedAsm.size());

        auto& disasm {GetDisasm()};
        auto& decoder {GetDecoder()};
        vixl::aarch64::Decoder::ScopedAddVisitors sv(decoder, {&disasm});
        for (size_t item = 0; item < codeItems; ++item) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            decoder.Decode(codeEntry + item * vixl::aarch64::kInstructionSize);
            EXPECT_EQ(expectedAsm.at(item), disasm.GetOutput());
        }
    }

    std::vector<std::string> CheckLeafWithParamsOnStackPrologueExpectedAsm()
    {
        // In this case two parameters are passed on stack,
        // thus to address them SP needs to be adjusted in prolog/epilog.
        return {
#ifdef PANDA_COMPILER_DEBUG_INFO
            "stp x29, x30, [sp, #-16]!",    // prolog save FP and LR
            "mov x29, sp",                  // prolog set FP
            "stur x19, [sp, #-104]",    // prolog callee-saved
            "stp x20, x21, [sp, #-96]",     // prolog callee-saved
            "stp x22, x23, [sp, #-80]",     // prolog callee-saved
            "sub sp, sp, #0x230 // (560)",  // prolog adjust SP
#else
            "stur x19, [sp, #-120]",    // prolog callee-saved
            "stp x20, x21, [sp, #-112]",    // prolog callee-saved
            "stp x22, x23, [sp, #-96]",     // prolog callee-saved
            "sub sp, sp, #0x240 // (576)",  // prolog adjust SP
#endif
            "add",                          // "add x19, x2, x3"
            "add",                          // "add x21, x4, x5"
            "add",                          // "add x22, x6, x7"
            "add x16, sp, #0x240 // (576)", // load params from stack
            "ldp x22, x23, [x16]",          // load params from stack
            "add",                          // "add x23, x23, x24"
            "add",                          // "add x19, x19, x21"
            "add",                          // "add x21, x22, x23"
            "add",                          // "add x19, x19, x21"
            "str x19, [x1]",
            "ldr x19, [sp, #456]",     // restore callee-saved
            "ldp x20, x21, [sp, #464]",     // restore callee-saved
            "ldp x22, x23, [sp, #480]",     // restore callee-saved
#ifdef PANDA_COMPILER_DEBUG_INFO
            "add sp, sp, #0x230 // (560)",  // epilog adjust SP
            "ldp x29, x30, [sp], #16",      // epilog restore FP and LR
#else
            "add sp, sp, #0x240 // (576)",  // epilog adjust SP
#endif
            "ret"};
    }

    void CheckLeafWithParamsOnStackPrologueBuildGraph()
    {
        RuntimeInterface::FieldPtr i = reinterpret_cast<void *>(0xDEADBEEF);  // NOLINT(modernize-use-auto)
        GRAPH(GetGraph())
        {
            PARAMETER(0U, 0_I).ref();
            PARAMETER(1U, 1_I).s64();
            PARAMETER(2U, 2_I).s64();
            PARAMETER(3U, 3_I).s64();
            PARAMETER(4U, 4_I).s64();
            PARAMETER(5U, 5_I).s64();
            PARAMETER(6U, 6_I).s64();
            PARAMETER(7U, 7_I).s64();
            PARAMETER(8U, 8_I).s64();

            BASIC_BLOCK(2U, -1)
            {
                INST(10U, Opcode::Add).s64().Inputs(1U, 2_I);
                INST(11U, Opcode::Add).s64().Inputs(3U, 4_I);
                INST(12U, Opcode::Add).s64().Inputs(5U, 6_I);
                INST(13U, Opcode::Add).s64().Inputs(7U, 8_I);
                INST(14U, Opcode::Add).s64().Inputs(10U, 11_I);
                INST(15U, Opcode::Add).s64().Inputs(12U, 13_I);
                INST(16U, Opcode::Add).s64().Inputs(14U, 15_I);
                INST(17U, Opcode::StoreObject).s64().Inputs(0, 16_I).TypeId(301U).ObjField(i);
                INST(18U, Opcode::ReturnVoid);
            }
        }
    }

    void CheckLeafWithParamsOnStackPrologue()
    {
        // RedundantOps::sum()
        CheckLeafWithParamsOnStackPrologueBuildGraph();
        SetNumArgs(9U);

        auto expectedAsm = CheckLeafWithParamsOnStackPrologueExpectedAsm();

        std::regex addRegex("^add[[:blank:]]+x[0-9]+,[[:blank:]]+x[0-9]+,[[:blank:]]+x[0-9]+",
                             std::regex::egrep | std::regex::icase);

        GraphChecker(GetGraph()).Check();
        GetGraph()->RunPass<RegAllocLinearScan>();
        bool setupFrame = GetGraph()->GetMethodProperties().GetRequireFrameSetup();
        ASSERT_TRUE(setupFrame ? GetGraph()->RunPass<Codegen>() : GetGraph()->RunPass<CodegenNative>());
        ASSERT_TRUE(GetGraph()->GetCode().Size() == expectedAsm.size() * vixl::aarch64::kInstructionSize);
        auto codeEntry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + GetGraph()->GetCode().Size();
        size_t codeItems = (codeExit - codeEntry) / vixl::aarch64::kInstructionSize;
        ASSERT_TRUE(codeItems == expectedAsm.size());

        auto& decoder {GetDecoder()};
        auto& disasm {GetDisasm()};
        vixl::aarch64::Decoder::ScopedAddVisitors sv(decoder, {&disasm});
        for (size_t item = 0; item < codeItems; ++item) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            decoder.Decode(codeEntry + item * vixl::aarch64::kInstructionSize);
            // replace 'add rx, ry, rz' with 'add' to make comparison independent of regalloc
            std::string s = std::regex_replace(disasm.GetOutput(), addRegex, "add");
            EXPECT_EQ(expectedAsm.at(item), s);
        }
    }
};
// clang-format on

TEST_F(CodegenLeafPrologueTest, LeafPrologueGeneration)
{
    CheckLeafPrologue();
}

TEST_F(CodegenLeafPrologueTest, LeafWithParamsOnStackPrologueGeneration)
{
    CheckLeafWithParamsOnStackPrologue();
}

class CodegenTest : public VixlDisasmTest {
public:
    template <typename T, size_t LEN>
    void AssertCode(const T (&expectedCode)[LEN])
    {
        auto codeEntry = reinterpret_cast<vixl::aarch64::Instruction *>(GetGraph()->GetCode().Data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto codeExit = codeEntry + GetGraph()->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        auto &decoder {GetDecoder()};
        auto &disasm {GetDisasm()};
        vixl::aarch64::Decoder::ScopedAddVisitors sv(decoder, {&disasm});

        size_t index = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto instr = codeEntry; instr < codeExit; instr += vixl::aarch64::kInstructionSize) {
            decoder.Decode(instr);
            auto output = disasm.GetOutput();
            if (index == 0) {
                if (std::strncmp(output, expectedCode[index], std::strlen(expectedCode[index])) == 0) {
                    index++;
                }
                continue;
            }
            if (index >= LEN) {
                break;
            }
            ASSERT_TRUE(std::strncmp(output, expectedCode[index], std::strlen(expectedCode[index])) == 0);
            index++;
        }
        ASSERT_EQ(index, LEN);
    }
};

TEST_F(CodegenTest, CallVirtual)
{
    auto graph = GetGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        BASIC_BLOCK(2U, -1)
        {
            INST(2U, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3U, Opcode::CallVirtual).v0id().InputsAutoType(0_I, 1_I, 2_I);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }
    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());
    // exclude offset from verification to avoid test modifications
    const char *expectedCode[] = {(OBJECT_POINTER_SIZE == sizeof(uint32_t)) ? "ldr w0, [x1]" : "ldr x0, [x1]",
                                  "ldr x0, [x0, #", "ldr x30, [x0, #",
                                  "blr x30"};  // CallVirtual is encoded without tmp reg
    AssertCode(expectedCode);
}

TEST_F(CodegenTest, EncodeMemCopy)
{
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i32().DstReg(0U);
        BASIC_BLOCK(2U, -1)
        {
            INST(2U, Opcode::SpillFill);
            INST(3U, Opcode::Return).i32().Inputs(0).DstReg(0U);
        }
    }
    auto spillFill = INS(2U).CastToSpillFill();
    // Add moves chain: R0 -> S0 -> S1 -> R0 [u32]
    spillFill->AddSpillFill(Location::MakeRegister(0), Location::MakeStackSlot(0), DataType::INT32);
    spillFill->AddSpillFill(Location::MakeStackSlot(0), Location::MakeStackSlot(1), DataType::INT32);
    spillFill->AddSpillFill(Location::MakeStackSlot(1), Location::MakeRegister(0), DataType::INT32);

    graph->SetStackSlotsCount(2U);
#ifndef NDEBUG
    graph->SetRegAllocApplied();
#endif
    EXPECT_TRUE(graph->RunPass<Codegen>());

    // Check that stack slots are 64-bit wide
    const char *expectedCode[] = {"str x0, [sp, #16]", "ldr x16, [sp, #16]", "str x16, [sp, #8]", "ldr w0, [sp, #8]"};
    AssertCode(expectedCode);
}

// MAdd a, b, c <=> c + a * b
TEST_F(CodegenTest, EncodeMAddWithZeroRegA)
{
    // a = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1, 0).i64();
        PARAMETER(2U, 1).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MAdd).i64().Inputs(0U, 1_I, 2_I);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mov x0, x2"};
    AssertCode(expectedCode);
}

TEST_F(CodegenTest, EncodeMAddWithZeroRegB)
{
    // b = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1U, 0).i64();
        PARAMETER(2U, 1).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MAdd).i64().Inputs(1, 0, 2_I);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mov x0, x2"};
    AssertCode(expectedCode);
}

TEST_F(CodegenTest, EncodeMAddWithZeroRegC)
{
    // c = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1U, 0).i64();
        PARAMETER(2U, 1).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MAdd).i64().Inputs(1, 2_I, 0);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mul x0, x1, x2"};
    AssertCode(expectedCode);
}

// MSub a, b, c <=> c - a * b
TEST_F(CodegenTest, EncodeMSubWithZeroRegA)
{
    // a = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1U, 0).i64();
        PARAMETER(2U, 1).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MSub).i64().Inputs(0, 1, 2_I);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mov x0, x2"};
    AssertCode(expectedCode);
}

TEST_F(CodegenTest, EncodeMSubWithZeroRegB)
{
    // b = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1U, 0).i64();
        PARAMETER(2U, 1).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MSub).i64().Inputs(1, 0, 2_I);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mov x0, x2"};
    AssertCode(expectedCode);
}

TEST_F(CodegenTest, EncodeMSubWithZeroRegC)
{
    // c = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1, 0).i64();
        PARAMETER(2U, 1).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MSub).i64().Inputs(1, 2_I, 0);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mneg x0, x1, x2"};
    AssertCode(expectedCode);
}

// MNeg a, b <=> -(a * b)
TEST_F(CodegenTest, EncodeMNegWithZeroRegA)
{
    // a = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1, 0).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MNeg).i64().Inputs(0, 1);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mov x0, #0"};
    AssertCode(expectedCode);
}

TEST_F(CodegenTest, EncodeMNegWithZeroRegB)
{
    // b = 0
    auto graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).i64();
        PARAMETER(1, 0).i64();

        BASIC_BLOCK(2U, -1)
        {
            INST(3U, Opcode::MNeg).i64().Inputs(1, 0);
            INST(4U, Opcode::Return).i64().Inputs(3U);
        }
    }

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());

    const char *expectedCode[] = {"mov x0, #0"};
    AssertCode(expectedCode);
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const DecoderVisitor::FormToVisitorFnMap DecoderVisitor::FORM_TO_VISITOR = {
    DEFAULT_FORM_TO_VISITOR_MAP(DecoderVisitor)};

// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays)

}  // namespace ark::compiler
