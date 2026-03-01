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

#include "frame_builder.h"
#include "llvm_ark_interface.h"

#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/cframe_layout.h"
#include "libarkbase/utils/arch.h"
#include "compiler/optimizer/code_generator/target_info.h"

#include <llvm/CodeGen/GlobalISel/MachineIRBuilder.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Target/TargetMachine.h>

struct InlineAsmBuilder {
    InlineAsmBuilder(llvm::MachineBasicBlock *block, llvm::MachineBasicBlock::iterator iterator)
        : mblock_ {block}, insertionPoint_ {iterator}
    {
    }
    ~InlineAsmBuilder() = default;

    InlineAsmBuilder(const InlineAsmBuilder &) = delete;
    void operator=(const InlineAsmBuilder &) = delete;
    InlineAsmBuilder(InlineAsmBuilder &&) = delete;
    InlineAsmBuilder &operator=(InlineAsmBuilder &&) = delete;

    void CreateInlineAsm(std::string_view inlineAsm, llvm::ArrayRef<ssize_t> imms = {})
    {
        ASSERT(mblock_ != nullptr);

        auto mfunc = mblock_->getParent();
        auto &targetInstructionInfo = *mfunc->getSubtarget().getInstrInfo();
        auto &asmInstructionDescriptor = targetInstructionInfo.get(llvm::TargetOpcode::INLINEASM);
        auto mib = llvm::BuildMI(*mblock_, insertionPoint_, llvm::DebugLoc(), asmInstructionDescriptor);
        mib.addExternalSymbol(inlineAsm.data());
        mib.addImm(static_cast<unsigned>(llvm::InlineAsm::Extra_HasSideEffects));

        for (auto value : imms) {
            mib.addImm(llvm::InlineAsm::getFlagWord(llvm::InlineAsm::Kind_Imm, 1U));
            mib.add(llvm::ArrayRef {llvm::MachineOperand::CreateImm(value)});
        }
    }

private:
    llvm::MachineBasicBlock *mblock_ {nullptr};
    llvm::MachineBasicBlock::iterator insertionPoint_;
};

namespace {

void RemoveInstsIf(llvm::MachineBasicBlock &mblock, const std::function<bool(llvm::MachineInstr &)> &predicate)
{
    std::vector<llvm::MachineInstr *> markedAsRemoved;
    for (auto &inst : mblock) {
        if (predicate(inst)) {
            markedAsRemoved.push_back(&inst);
        }
    }

    for (auto inst : markedAsRemoved) {
        inst->removeFromParent();
    }
}

}  // namespace

// AMD64 Frame builder implementation

bool AMD64FrameBuilder::RemovePrologue(llvm::MachineBasicBlock &mblock)
{
    auto predicate = [](llvm::MachineInstr &inst) -> bool {
        bool isFrameSetup = inst.getFlag(llvm::MachineInstr::FrameSetup);
        return isFrameSetup;
    };
    RemoveInstsIf(mblock, predicate);
    return mblock.getPrevNode() == nullptr;
}

bool AMD64FrameBuilder::RemoveEpilogue(llvm::MachineBasicBlock &mblock)
{
    bool isEpilogue = false;
    auto predicate = [&isEpilogue](llvm::MachineInstr &inst) -> bool {
        bool isFrameDestroy = inst.getFlag(llvm::MachineInstr::FrameDestroy);
        // Sometimes llvm may generate code, when there is no any frame-destroy instructions
        // that contain RET. And we still need to remember such basic blocks.
        isEpilogue |= inst.isReturn();
        return isFrameDestroy;
    };
    RemoveInstsIf(mblock, predicate);
    return isEpilogue;
}

void AMD64FrameBuilder::InsertPrologue(llvm::MachineBasicBlock &mblock)
{
    auto [xregsMask, vregsMask] = frameInfo_.regMasks;
    if (!frameInfo_.hasCalls && !frameInfo_.usesStack && xregsMask == 0 && vregsMask == 0) {
        // Do not generate any code
        return;
    }

    InlineAsmBuilder builder(&mblock, mblock.begin());

    constexpr ark::CFrameLayout FL(ark::Arch::X86_64, 0);
    constexpr auto SP_ORIGIN = ark::CFrameLayout::OffsetOrigin::SP;
    constexpr auto BYTES_UNITS = ark::CFrameLayout::OffsetUnit::BYTES;

    constexpr ssize_t SLOT_SIZE = ark::PointerSize(ark::Arch::X86_64);
    constexpr ssize_t FRAME_SIZE = FL.GetFrameSize<BYTES_UNITS>();
    constexpr ssize_t METHOD_OFFSET = FL.GetOffset<SP_ORIGIN, BYTES_UNITS>(ark::CFrameLayout::MethodSlot::Start());
    constexpr ssize_t FLAGS_OFFSET = FL.GetOffset<SP_ORIGIN, BYTES_UNITS>(ark::CFrameLayout::FlagsSlot::Start());
    constexpr ssize_t CALLEE_OFFSET = FL.GetOffset<SP_ORIGIN, BYTES_UNITS>(FL.GetCalleeRegsStartSlot());

    auto frameFlags = constantPool_(FrameConstantDescriptor::FRAME_FLAGS);
    auto tlsFrameOffset = constantPool_(FrameConstantDescriptor::TLS_FRAME_OFFSET);

    builder.CreateInlineAsm("push   %rbp");
    builder.CreateInlineAsm("movq   %rsp, %rbp");
    builder.CreateInlineAsm("lea    ${0:c}(%rsp), %rsp", {-(FRAME_SIZE - SLOT_SIZE * 2U)});
    builder.CreateInlineAsm("movq   %rdi,  ${0:c}(%rsp)", {METHOD_OFFSET});
    if (!frameInfo_.usesFloatRegs) {
        frameFlags = 0;
    }
    builder.CreateInlineAsm("movq   $0, ${1:c}(%rsp)", {frameFlags, FLAGS_OFFSET});
    builder.CreateInlineAsm("movb   $$0x1, ${0:c}(%r15)", {tlsFrameOffset});
    builder.CreateInlineAsm("movq   %r15, ${0:c}(%rsp)", {CALLEE_OFFSET - 0 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   %r14, ${0:c}(%rsp)", {CALLEE_OFFSET - 1 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   %r13, ${0:c}(%rsp)", {CALLEE_OFFSET - 2 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   %r12, ${0:c}(%rsp)", {CALLEE_OFFSET - 3 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   %rbx, ${0:c}(%rsp)", {CALLEE_OFFSET - 4 * SLOT_SIZE});
    // Ignore space allocated for rbp, we already save it
    ssize_t spaceForSpills = frameInfo_.stackSize - SLOT_SIZE;
    if (spaceForSpills != 0) {
        builder.CreateInlineAsm("sub  $0, %rsp", {spaceForSpills});
    }
    // StackOverflowCheck
    builder.CreateInlineAsm("test   %rdi, ${0:c}(%rsp)", {frameInfo_.soOffset});
}

void AMD64FrameBuilder::InsertEpilogue(llvm::MachineBasicBlock &mblock)
{
    auto [xregsMask, vregsMask] = frameInfo_.regMasks;
    if (!frameInfo_.hasCalls && !frameInfo_.usesStack && xregsMask == 0 && vregsMask == 0) {
        // Do not generate any code
        return;
    }

    InlineAsmBuilder builder(&mblock, mblock.getFirstTerminator());

    constexpr ark::CFrameLayout FL(ark::Arch::X86_64, 0);

    constexpr auto SP_ORIGIN = ark::CFrameLayout::OffsetOrigin::SP;
    constexpr auto BYTES_UNITS = ark::CFrameLayout::OffsetUnit::BYTES;

    constexpr ssize_t SLOT_SIZE = ark::PointerSize(ark::Arch::X86_64);
    constexpr ssize_t FRAME_SIZE = FL.GetFrameSize<BYTES_UNITS>();
    constexpr ssize_t CALLEE_OFFSET = FL.GetOffset<SP_ORIGIN, BYTES_UNITS>(FL.GetCalleeRegsStartSlot());

    builder.CreateInlineAsm("movq   %rbp, %rsp");
    builder.CreateInlineAsm("lea    ${0:c}(%rsp), %rsp", {-(FRAME_SIZE - SLOT_SIZE * 2U)});
    builder.CreateInlineAsm("movq   ${0:c}(%rsp), %rbx", {CALLEE_OFFSET - 4 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   ${0:c}(%rsp), %r12", {CALLEE_OFFSET - 3 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   ${0:c}(%rsp), %r13", {CALLEE_OFFSET - 2 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   ${0:c}(%rsp), %r14", {CALLEE_OFFSET - 1 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   ${0:c}(%rsp), %r15", {CALLEE_OFFSET - 0 * SLOT_SIZE});
    builder.CreateInlineAsm("movq   %rbp, %rsp");
    builder.CreateInlineAsm("pop  %rbp");
}

// ARM64 Frame builder implementation

bool ARM64FrameBuilder::RemovePrologue(llvm::MachineBasicBlock &mblock)
{
    auto predicate = [](llvm::MachineInstr &inst) -> bool {
        bool isFrameSetup = inst.getFlag(llvm::MachineInstr::FrameSetup);
        return isFrameSetup;
    };
    RemoveInstsIf(mblock, predicate);
    return mblock.getPrevNode() == nullptr;
}

bool ARM64FrameBuilder::RemoveEpilogue(llvm::MachineBasicBlock &mblock)
{
    bool isEpilogue = false;
    auto predicate = [&isEpilogue](llvm::MachineInstr &inst) -> bool {
        bool isFrameDestroy = inst.getFlag(llvm::MachineInstr::FrameDestroy);
        // Sometimes llvm may generate code, when there is no any frame-destroy instructions
        // that contain RET. And we still need to remember such basic blocks.
        isEpilogue |= inst.isReturn();
        return isFrameDestroy;
    };
    RemoveInstsIf(mblock, predicate);
    return isEpilogue;
}

namespace {
namespace arm_frame_helpers {
constexpr ark::CFrameLayout FL(ark::Arch::AARCH64, 0);
constexpr ssize_t SLOT_SIZE = ark::PointerSize(ark::Arch::AARCH64);
constexpr ssize_t DSLOT_SIZE = SLOT_SIZE * 2U;
constexpr auto FP_ORIGIN = ark::CFrameLayout::OffsetOrigin::FP;
constexpr auto BYTES_UNITS = ark::CFrameLayout::OffsetUnit::BYTES;
constexpr ssize_t FLAGS_OFFSET = FL.GetOffset<FP_ORIGIN, BYTES_UNITS>(ark::CFrameLayout::FlagsSlot::Start());
constexpr ssize_t X_CALLEE_OFFSET = FL.GetOffset<FP_ORIGIN, BYTES_UNITS>(FL.GetCalleeRegsStartSlot()) - SLOT_SIZE;
constexpr ssize_t V_CALLEE_OFFSET = FL.GetOffset<FP_ORIGIN, BYTES_UNITS>(FL.GetCalleeFpRegsStartSlot()) - SLOT_SIZE;
constexpr auto INVALID_REGISTER = 255;
constexpr auto MAX_OFFSET = 256;

static_assert(FL.GetOffset<FP_ORIGIN, BYTES_UNITS>(FL.GetCallerRegsStartSlot()) <= MAX_OFFSET);
}  // namespace arm_frame_helpers
}  // namespace

void ARM64FrameBuilder::InsertPrologue(llvm::MachineBasicBlock &mblock)
{
    InlineAsmBuilder builder(&mblock, mblock.begin());

    auto [xregsMask, vregsMask] = frameInfo_.regMasks;

    // Do not generate any code
    if (!frameInfo_.hasCalls && !frameInfo_.usesStack && xregsMask == 0 && vregsMask == 0) {
        return;
    }

    builder.CreateInlineAsm("stp  x29, x30, [sp, #-16]!");
    builder.CreateInlineAsm("mov  x29, sp");

    if (frameInfo_.usesFloatRegs) {
        auto frameFlags = constantPool_(FrameConstantDescriptor::FRAME_FLAGS);
        builder.CreateInlineAsm("mov x30, $0", {frameFlags});
        builder.CreateInlineAsm("stp x30, x0, [fp, ${0:n}]", {arm_frame_helpers::FLAGS_OFFSET});
    } else {
        builder.CreateInlineAsm("stp xzr, x0, [fp, ${0:n}]", {arm_frame_helpers::FLAGS_OFFSET});
    }

    if (frameInfo_.hasCalls) {
        auto tlsFrameOffset = constantPool_(FrameConstantDescriptor::TLS_FRAME_OFFSET);
        builder.CreateInlineAsm("mov x30, #1");
        builder.CreateInlineAsm("str w30, [x28, ${0:c}]", {tlsFrameOffset});
    }

    if (xregsMask != 0) {
        EmitCSRSaveRestoreCode(&builder, xregsMask, "stur x$0, [sp, -$1]", "stp x$0, x$1, [sp, -$2]",
                               arm_frame_helpers::X_CALLEE_OFFSET);
    }

    if (vregsMask != 0) {
        EmitCSRSaveRestoreCode(&builder, vregsMask, "stur d$0, [sp, -$1]", "stp d$0, d$1, [sp, -$2]",
                               arm_frame_helpers::V_CALLEE_OFFSET);
    }

    constexpr uint32_t MAX_IMM_12 = 4096;
    builder.CreateInlineAsm("sub sp, fp, ${0:c}", {frameInfo_.stackSize % MAX_IMM_12});
    ASSERT(frameInfo_.stackSize / MAX_IMM_12 < MAX_IMM_12);
    if (frameInfo_.stackSize / MAX_IMM_12 > 0) {
        builder.CreateInlineAsm("sub sp, sp, ${0:c}", {MAX_IMM_12 * (frameInfo_.stackSize / MAX_IMM_12)});
    }

    // StackOverflow check
    builder.CreateInlineAsm("add x30, sp, ${0:c}", {frameInfo_.soOffset});
    builder.CreateInlineAsm("ldr x30, [x30]");
}

void ARM64FrameBuilder::InsertEpilogue(llvm::MachineBasicBlock &mblock)
{
    InlineAsmBuilder builder(&mblock, mblock.getFirstTerminator());

    auto [xregsMask, vregsMask] = frameInfo_.regMasks;
    if (!frameInfo_.hasCalls && !frameInfo_.usesStack && xregsMask == 0 && vregsMask == 0) {
        // Do not generate any code
        return;
    }

    builder.CreateInlineAsm("mov sp, fp");
    if (xregsMask != 0) {
        EmitCSRSaveRestoreCode(&builder, xregsMask, "ldur x$0, [sp, -$1]", "ldp x$0, x$1, [sp, -$2]",
                               arm_frame_helpers::X_CALLEE_OFFSET);
    }
    if (vregsMask != 0) {
        EmitCSRSaveRestoreCode(&builder, vregsMask, "ldur d$0, [sp, -$1]", "ldp d$0, d$1, [sp, -$2]",
                               arm_frame_helpers::V_CALLEE_OFFSET);
    }

    builder.CreateInlineAsm("ldp x29, x30, [sp], #16");
}

void ARM64FrameBuilder::EmitCSRSaveRestoreCode(InlineAsmBuilder *builder, uint32_t regsMask,
                                               std::string_view asmSingleReg, std::string_view asmPairRegs,
                                               ssize_t calleeOffset)
{
    std::vector<int> regs;
    unsigned i = sizeof(regsMask) * 8U;
    do {
        i--;
        if ((regsMask & (1U << i)) != 0) {
            regs.push_back(i);
        }
    } while (i != 0);

    if (regs.size() % 2U == 1) {
        regs.push_back(arm_frame_helpers::INVALID_REGISTER);
    }

    auto off = calleeOffset;
    for (i = 0; i < regs.size(); i += 2U) {
        auto [reg_a, reg_b] = std::minmax(regs[i + 0U], regs[i + 1U]);
        if (reg_b != arm_frame_helpers::INVALID_REGISTER) {
            off += arm_frame_helpers::DSLOT_SIZE;
            builder->CreateInlineAsm(asmPairRegs, {reg_a, reg_b, off});
        } else {
            off += arm_frame_helpers::SLOT_SIZE;
            builder->CreateInlineAsm(asmSingleReg, {reg_a, off});
        }
    }
}
