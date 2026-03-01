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

#ifndef PANDA_CODEGEN_INTERPRETER_H
#define PANDA_CODEGEN_INTERPRETER_H

#include "compiler/optimizer/code_generator/codegen_native.h"
#include "irtoc/backend/compiler/constants.h"

namespace ark::compiler {

/**
 * Code generation for functions that should be called inside interpreter
 * Prologue and epilogue of such functions are empty, all arguments are placed in registers manually
 */
class CodegenInterpreter : public CodegenNative {
public:
    using CodegenNative::CodegenNative;

    /**
     *  fix number of spill slots such that RETURN handlers are able to generate epilogue correspondent to
     *  InterpreterEntry prologue independently from number of spill slots in handler itself.
     *  Should be synced with spills_count_max in interpreter.irt
     */
    static constexpr size_t SPILL_SLOTS = IRTOC_NUM_SPILL_SLOTS;

    void CreateFrameInfo() override
    {
        auto &fl = GetFrameLayout();
        auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
            FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(true) |
            FrameInfo::CallersRelativeFp::Encode(true) | FrameInfo::CalleesRelativeFp::Encode(true) |
            FrameInfo::SpillsRelativeFp::Encode(true));
        ASSERT(frame != nullptr);
        frame->SetFrameSize(fl.GetFrameSize<CFrameLayout::OffsetUnit::BYTES>());
        frame->SetSpillsCount(fl.GetSpillsCount());

        constexpr auto FP = CFrameLayout::OffsetOrigin::FP;
        constexpr auto SLOTS = CFrameLayout::OffsetUnit::SLOTS;
        frame->SetCallersOffset(-fl.GetOffset<FP, SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
        frame->SetFpCallersOffset(-fl.GetOffset<FP, SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
        frame->SetCalleesOffset(-fl.GetOffset<FP, SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
        frame->SetFpCalleesOffset(-fl.GetOffset<FP, SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

        frame->SetSetupFrame(true);
        frame->SetSaveFrameAndLinkRegs(true);
        frame->SetSaveUnusedCalleeRegs(true);
        frame->SetAdjustSpReg(true);

        SetFrameInfo(frame);
    }

    void GeneratePrologue() override
    {
        GetEncoder()->SetFrameLayout(ark::CFrameLayout(GetGraph()->GetArch(), SPILL_SLOTS));
        if (GetGraph()->GetMode().IsInterpreterEntry()) {
            ScopedDisasmPrinter printer(this, "Entrypoint Prologue");
            GetCallingConvention()->GenerateNativePrologue(*GetFrameInfo());
        }
    }

    void GenerateEpilogue() override {}

    void EmitTailCallIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS src) override
    {
        auto ptr = ConvertRegister(inst->GetSrcReg(0), DataType::POINTER);  // pointer
        GetEncoder()->EncodeJump(ptr);
    }

    void EmitInitInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                SRCREGS src) override
    {
        auto *enc = GetEncoder();
        auto sp = SpReg();
        ssize_t pointerSize = PointerSize(GetArch());
        /*
         * Layout below the last slot (each slot is of pointer size):
         * [-1] Unused
         * [-2] uint32_t counter
         * [-3] void*    bytecode PC of 1st call record
         * [-4] void*    method of 1st call record          <------- New SP
         */
        enc->EncodeStpPreIndex(src[0], src[1], sp, Imm(-4 * pointerSize));  // 4 : See above

        constexpr uint32_t INITIAL_COUNT = 1U;
        enc->EncodeSti(INITIAL_COUNT, sizeof(uint32_t), MemRef(sp, 2 * pointerSize));  // 2 : Offset of counter from SP
    }

    void EmitPushInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                SRCREGS src) override
    {
        auto *enc = GetEncoder();
        ssize_t pointerSize = PointerSize(GetArch());
        /*
         * Layout of each call record (each slot is of pointer size):
         * [-1] void*    bytecode PC of 1st call record
         * [-2] void*    method of 1st call record          <------- New SP
         */
        enc->EncodeStpPreIndex(src[0], src[1], SpReg(), Imm(-2 * pointerSize));  // 2 : Two slots per call record

        // Counter increment should be performed AFTER pushing call record to prevent memory access below SP.
        ssize_t countOffset = GetInterpreterCallRecordCountOffsetFromFP();
        auto temp = ScopedTmpRegU32 {enc};
        enc->EncodeLdr(temp, false, MemRef {FpReg(), countOffset});
        enc->EncodeAdd(temp, temp, Imm(1));
        enc->EncodeStr(temp, MemRef {FpReg(), countOffset});
    }

    void EmitPopInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                               [[maybe_unused]] SRCREGS src) override
    {
        auto *enc = GetEncoder();
        ssize_t pointerSize = PointerSize(GetArch());

        // Counter decrement should be performed BEFORE popping call record to prevent memory access below SP.
        ssize_t countOffset = GetInterpreterCallRecordCountOffsetFromFP();
        auto temp = ScopedTmpRegU32 {enc};
        enc->EncodeLdr(temp, false, MemRef {FpReg(), countOffset});
        enc->EncodeSub(temp, temp, Imm(1));
        enc->EncodeStr(temp, MemRef {FpReg(), countOffset});
        enc->EncodeAdd(SpReg(), SpReg(), Imm(2 * pointerSize));  // 2 : Two slots per call record
    }

    void EmitPopMultipleInterpreterCallRecordsIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                        SRCREGS src) override
    {
        auto *enc = GetEncoder();
        ssize_t pointerSize = PointerSize(GetArch());

        auto temp = ScopedTmpRegU32 {enc};
        auto tempPtr = ConvertRegister(temp.GetReg().GetId(), DataType::POINTER);

        auto popNum = src[0];
        auto popNumAsPtr = ConvertRegister(popNum.GetId(), DataType::POINTER);

        // Counter decrement should be performed BEFORE popping call record to prevent memory access below SP.
        ssize_t countOffset = GetInterpreterCallRecordCountOffsetFromFP();
        enc->EncodeLdr(temp, false, MemRef {FpReg(), countOffset});
        enc->EncodeSub(temp, temp, popNum);
        enc->EncodeStr(temp, MemRef {FpReg(), countOffset});

        auto scale = Ctz(static_cast<size_t>(pointerSize)) + 1U;
        switch (GetArch()) {
            case Arch::AARCH32:
            case Arch::AARCH64:
                enc->EncodeAdd(SpReg(), SpReg(), Shift {popNumAsPtr, scale});
                break;
            case Arch::X86:
            case Arch::X86_64:
                enc->EncodeShl(tempPtr, popNumAsPtr, Imm(scale));
                enc->EncodeAdd(SpReg(), SpReg(), tempPtr);
                break;
            default:
                UNREACHABLE();
        }
    }

    void EmitUpdateInterpreterCallRecordIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                  SRCREGS src) override
    {
        auto *enc = GetEncoder();
        ssize_t pointerSize = PointerSize(GetArch());

        // Updates bytecode PC. Others unchanged.
        enc->EncodeStr(src[0], MemRef {SpReg(), pointerSize});
    }

private:
    ssize_t GetInterpreterCallRecordCountOffsetFromFP()
    {
        ssize_t pointerSize = PointerSize(GetArch());
        ssize_t offset = GetIrtocCFrameSizeBytesBelowFP(GetArch());
        return -offset - 2 * pointerSize;  // 2 : Offset of counter from SP
    }
};

}  // namespace ark::compiler

#endif  // PANDA_CODEGEN_INTERPRETER_H
