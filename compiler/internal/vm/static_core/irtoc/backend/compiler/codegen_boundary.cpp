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

#include "codegen_boundary.h"
#include "libarkbase/utils/cframe_layout.h"

namespace ark::compiler {

static void PushStackRegister(Encoder *encoder, Target target, Reg threadReg, size_t tlsFrameOffset)
{
    static constexpr ssize_t FP_OFFSET = 2;
    const auto wordSize = static_cast<ssize_t>(target.WordSize());
    ASSERT(sizeof(FrameBridgeKind) <= static_cast<size_t>(wordSize));
    constexpr ssize_t MAX_ALLOWED = std::numeric_limits<ssize_t>::max() / FP_OFFSET;
    if (wordSize > MAX_ALLOWED) {
        LOG(FATAL, IRTOC) << "WordSize too large for signed offset";
        return;
    }
    encoder->EncodeSti(FrameBridgeKind::COMPILED_CODE_TO_INTERPRETER, wordSize,
                       MemRef(target.GetStackReg(), -wordSize));
    encoder->EncodeStr(target.GetFrameReg(), MemRef(target.GetStackReg(), -FP_OFFSET * wordSize));

    {
        ScopedTmpReg tmp(encoder);
        encoder->EncodeSub(tmp, target.GetStackReg(), Imm(2U * target.WordSize()));
        encoder->EncodeStr(tmp, MemRef(threadReg, tlsFrameOffset));
    }
}

static void PushLinkAndStackRegister(Encoder *encoder, Target target, Reg threadReg, size_t tlsFrameOffset)
{
    ScopedTmpReg tmp(encoder);
    static constexpr ssize_t FP_OFFSET = 3;
    static constexpr ssize_t LR_OFFSET = 2;
    encoder->EncodeMov(tmp, Imm(FrameBridgeKind::COMPILED_CODE_TO_INTERPRETER));
    encoder->EncodeStp(tmp, target.GetLinkReg(), MemRef(target.GetStackReg(), -LR_OFFSET * target.WordSize()));
    encoder->EncodeStr(target.GetFrameReg(), MemRef(target.GetStackReg(), -FP_OFFSET * target.WordSize()));

    encoder->EncodeSub(target.GetLinkReg(), target.GetStackReg(), Imm(FP_OFFSET * target.WordSize()));
    encoder->EncodeStr(target.GetLinkReg(), MemRef(threadReg, tlsFrameOffset));
}

void CodegenBoundary::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Boundary Prologue");
    auto encoder = GetEncoder();
    auto frame = GetFrameInfo();
    auto target = GetTarget();
    auto threadReg = ThreadReg();
    auto tlsFrameOffset = GetRuntime()->GetTlsFrameOffset(GetArch());

    if (target.SupportLinkReg()) {
        PushLinkAndStackRegister(encoder, target, threadReg, tlsFrameOffset);
    } else {
        PushStackRegister(encoder, target, threadReg, tlsFrameOffset);
    }

    encoder->EncodeSub(GetTarget().GetStackReg(), GetTarget().GetStackReg(), Imm(frame->GetFrameSize()));

    auto calleeRegs =
        GetCalleeRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong() & ~ThreadReg().GetId();
    auto calleeVregs = GetCalleeRegsMask(GetArch(), true) & ~GetTarget().GetTempVRegsMask().to_ulong();
    auto callerRegs = GetCallerRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong();
    auto callerVregs = GetCallerRegsMask(GetArch(), true) & ~GetTarget().GetTempVRegsMask().to_ulong();

    Reg base = GetTarget().GetFrameReg();
    auto fl = GetFrameLayout();
    {
        SCOPED_DISASM_STR(this, "Save caller registers");
        ssize_t offset = fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
            CFrameLayout::GetStackStartSlot() + fl.GetCallerLastSlot(false));
        encoder->SaveRegisters(callerRegs, false, -offset, base, GetCallerRegsMask(GetArch(), false));
        offset = fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
            CFrameLayout::GetStackStartSlot() + fl.GetCallerLastSlot(true));
        encoder->SaveRegisters(callerVregs, true, -offset, base, GetCallerRegsMask(GetArch(), true));
    }
    {
        SCOPED_DISASM_STR(this, "Save callee registers");
        base = frame->GetCalleesRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
        encoder->SaveRegisters(calleeRegs, false, frame->GetCalleesOffset(), base, GetCalleeRegsMask(GetArch(), false));
        encoder->SaveRegisters(calleeVregs, true, frame->GetFpCalleesOffset(), base,
                               GetCalleeRegsMask(GetArch(), true));
    }
}

void CodegenBoundary::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "Boundary Epilogue");
    RemoveBoundaryFrame(GetGraph()->GetEndBlock());
    GetEncoder()->EncodeReturn();
}

void CodegenBoundary::CreateFrameInfo()
{
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(false) | FrameInfo::PositionedCallees::Encode(true) |
        FrameInfo::CallersRelativeFp::Encode(false) | FrameInfo::CalleesRelativeFp::Encode(false));
    auto target = Target(GetGraph()->GetArch());
    size_t spillsCount = GetGraph()->GetStackSlotsCount();
    size_t padding = 0;
    size_t frameSize =
        (CFrameLayout::HEADER_SIZE - (target.SupportLinkReg() ? 0 : 1) + GetCalleeRegsCount(target.GetArch(), false) +
         GetCalleeRegsCount(target.GetArch(), true) + GetCallerRegsCount(target.GetArch(), false) +
         GetCallerRegsCount(target.GetArch(), true) + spillsCount) *
        target.WordSize();
    if (target.SupportLinkReg()) {
        padding = RoundUp(frameSize, target.GetSpAlignment()) - frameSize;
    } else {
        if ((frameSize % target.GetSpAlignment()) == 0) {
            padding = target.GetSpAlignment() - target.WordSize();
        }
    }
    CHECK_EQ(padding % target.WordSize(), 0U);
    spillsCount += padding / target.WordSize();
    frameSize += padding;
    if (target.SupportLinkReg()) {
        CHECK_EQ(frameSize % target.GetSpAlignment(), 0U);
    } else {
        CHECK_EQ(frameSize % target.GetSpAlignment(), target.GetSpAlignment() - target.WordSize());
    }

    auto offset = static_cast<ssize_t>(spillsCount);
    frame->SetFpCallersOffset(offset);
    offset += helpers::ToSigned(GetCallerRegsCount(target.GetArch(), true));
    frame->SetCallersOffset(offset);
    offset += helpers::ToSigned(GetCallerRegsCount(target.GetArch(), false));

    frame->SetFpCalleesOffset(offset);
    offset += helpers::ToSigned(GetCalleeRegsCount(target.GetArch(), true));
    frame->SetCalleesOffset(offset);

    frame->SetSpillsCount(spillsCount);
    frame->SetFrameSize(frameSize);

    SetFrameInfo(frame);
}

void CodegenBoundary::EmitTailCallIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS src)
{
    auto location = inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto addr = Reg(location.GetValue(), GetTarget().GetPtrRegType());
    RegMask liveoutMask = GetLiveOut(inst->GetBasicBlock());
    ScopedTmpReg target(GetEncoder());
    if (!liveoutMask.Test(addr.GetId())) {
        ASSERT(target.GetReg().IsValid());
        GetEncoder()->EncodeMov(target, addr);
        addr = target.GetReg();
    }
    ASSERT(addr.IsValid());
    RemoveBoundaryFrame(inst->GetBasicBlock());
    GetEncoder()->EncodeJump(addr);
}

void CodegenBoundary::RemoveBoundaryFrame(const BasicBlock *bb) const
{
    auto encoder = GetEncoder();
    auto frame = GetFrameInfo();

    RegMask liveoutMask = GetLiveOut(bb);

    RegMask calleeRegs = GetCalleeRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong();
    calleeRegs &= ~liveoutMask;
    calleeRegs.reset(ThreadReg().GetId());
    RegMask calleeVregs = GetCalleeRegsMask(GetArch(), true) & ~GetTarget().GetTempVRegsMask().to_ulong();
    RegMask callerRegs = GetCallerRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong();
    callerRegs &= ~liveoutMask;
    RegMask callerVregs = GetCallerRegsMask(GetArch(), true) & ~GetTarget().GetTempVRegsMask().to_ulong();

    Reg base = GetTarget().GetFrameReg();
    auto fl = GetFrameLayout();
    ssize_t offset = fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        CFrameLayout::GetStackStartSlot() + fl.GetCallerLastSlot(false));
    encoder->LoadRegisters(callerRegs, false, -offset, base, GetCallerRegsMask(GetArch(), false));
    offset = fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        CFrameLayout::GetStackStartSlot() + fl.GetCallerLastSlot(true));
    encoder->LoadRegisters(callerVregs, true, -offset, base, GetCallerRegsMask(GetArch(), true));

    base = frame->GetCalleesRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    encoder->LoadRegisters(calleeRegs, false, frame->GetCalleesOffset(), base, GetCalleeRegsMask(GetArch(), false));
    encoder->LoadRegisters(calleeVregs, true, frame->GetFpCalleesOffset(), base, GetCalleeRegsMask(GetArch(), true));

    encoder->EncodeAdd(GetTarget().GetStackReg(), GetTarget().GetStackReg(), Imm(frame->GetFrameSize()));

    if (GetTarget().SupportLinkReg()) {
        static constexpr ssize_t FP_OFFSET = -3;
        const auto wordSize = static_cast<ssize_t>(GetTarget().WordSize());
        constexpr ssize_t MAX_ALLOWED = std::numeric_limits<ssize_t>::max() / -FP_OFFSET;
        if (wordSize > MAX_ALLOWED) {
            LOG(FATAL, IRTOC) << "Frame offset calculation overflow";
            return;
        }
        encoder->EncodeLdr(GetTarget().GetLinkReg(), false, MemRef(GetTarget().GetStackReg(), -wordSize));
        encoder->EncodeLdr(GetTarget().GetFrameReg(), false, MemRef(GetTarget().GetStackReg(), FP_OFFSET * wordSize));
    }
}
}  // namespace ark::compiler
