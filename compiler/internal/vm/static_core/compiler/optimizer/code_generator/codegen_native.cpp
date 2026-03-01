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

#include "codegen_native.h"
#include "optimizer/code_generator/method_properties.h"

namespace ark::compiler {

void CodegenNative::CreateFrameInfo()
{
    auto &fl = GetFrameLayout();
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(true) |
        FrameInfo::CallersRelativeFp::Encode(false) | FrameInfo::CalleesRelativeFp::Encode(true));
    ASSERT(frame != nullptr);
    frame->SetFrameSize(fl.GetFrameSize<CFrameLayout::OffsetUnit::BYTES>());
    frame->SetSpillsCount(fl.GetSpillsCount());

    frame->SetCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    ASSERT(!GetGraph()->GetMethodProperties().GetRequireFrameSetup());
    // we don't need to setup frame in native mode
    frame->SetSetupFrame(false);
    // we don't need to save FP and LR registers only for leaf methods
    frame->SetSaveFrameAndLinkRegs(!GetGraph()->GetMethodProperties().IsLeaf());
    // we may use lr reg as temp only if we saved lr in the prologue
    if (GetTarget().SupportLinkReg()) {
        GetEncoder()->EnableLrAsTempReg(frame->GetSaveFrameAndLinkRegs());
    }
    // we never need to save unused registers in native mode
    frame->SetSaveUnusedCalleeRegs(false);
    // we have to sub/add SP in prologue/epilogue in the following cases:
    // - non-leaf method
    // - leaf method and there are spills or parameters on stack
    frame->SetAdjustSpReg(!GetGraph()->GetMethodProperties().IsLeaf() || GetGraph()->GetStackSlotsCount() != 0 ||
                          GetGraph()->GetMethodProperties().GetHasParamsOnStack());
    SetFrameInfo(frame);
}

void CodegenNative::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Method Prologue");

    GetCallingConvention()->GenerateNativePrologue(*GetFrameInfo());

#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
    MakeTrace();
#endif
}

void CodegenNative::GenerateEpilogue()
{
    ASSERT(GetGraph()->GetMethodProperties().IsLeaf());
    SCOPED_DISASM_STR(this, "Method Epilogue");

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    GetCallingConvention()->GenerateNativeEpilogue(*GetFrameInfo(), MakeTrace);
#else
    GetCallingConvention()->GenerateNativeEpilogue(*GetFrameInfo(), []() {});
#endif
}
}  // namespace ark::compiler
