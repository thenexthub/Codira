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

#ifndef PANDA_FRAME_INFO_H
#define PANDA_FRAME_INFO_H

#include "libarkbase/utils/cframe_layout.h"
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/mem/mem.h"

namespace ark::compiler {

class Encoder;
class Graph;

/// This class describes layout of the frame being compiled.
class FrameInfo {
public:
    explicit FrameInfo(uint32_t fields) : fields_(fields) {}
    ~FrameInfo() = default;
    NO_COPY_SEMANTIC(FrameInfo);
    NO_MOVE_SEMANTIC(FrameInfo);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FRAME_INFO_GET_ATTR(name, var)         \
    auto Get##name() const                     \
    {                                          \
        /* CC-OFFNXT(G.PRE.05) function gen */ \
        return var;                            \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FRAME_INFO_SET_ATTR(name, var)                            \
    void Set##name(ssize_t val)                                   \
    {                                                             \
        ASSERT(val <= std::numeric_limits<decltype(var)>::max()); \
        ASSERT(val >= std::numeric_limits<decltype(var)>::min()); \
        var = val;                                                \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FRAME_INFO_ATTR(name, var) \
    FRAME_INFO_GET_ATTR(name, var) \
    FRAME_INFO_SET_ATTR(name, var)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FRAME_INFO_GET_FIELD(name, type)                                   \
    type Get##name() const                                                 \
    {                                                                      \
        /* CC-OFFNXT(G.PRE.02, G.PRE.05) function gen, namespace member */ \
        return name::Get(fields_);                                         \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FRAME_INFO_SET_FIELD(name, type)           \
    void Set##name(type val)                       \
    {                                              \
        /* CC-OFFNXT(G.PRE.02) namespace member */ \
        name::Set(val, &fields_);                  \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FRAME_INFO_FIELD(name, type) \
    FRAME_INFO_GET_FIELD(name, type) \
    FRAME_INFO_SET_FIELD(name, type)

    FRAME_INFO_ATTR(FrameSize, frameSize_);
    FRAME_INFO_ATTR(SpillsCount, spillsCount_);
    FRAME_INFO_ATTR(CallersOffset, callersOffset_);
    FRAME_INFO_ATTR(CalleesOffset, calleesOffset_);
    FRAME_INFO_ATTR(FpCallersOffset, fpCallersOffset_);
    FRAME_INFO_ATTR(FpCalleesOffset, fpCalleesOffset_);
    FRAME_INFO_FIELD(PositionedCallers, bool);
    FRAME_INFO_FIELD(PositionedCallees, bool);
    FRAME_INFO_FIELD(CallersRelativeFp, bool);
    FRAME_INFO_FIELD(CalleesRelativeFp, bool);
    FRAME_INFO_FIELD(SpillsRelativeFp, bool);
    // SaveFrameAndLinkRegs - save/restore FP and LR registers in prologue/epilogue.
    FRAME_INFO_FIELD(SaveFrameAndLinkRegs, bool);
    // SetupFrame - setup CFrame (aka. 'managed' frame).
    // Namely, set FP reg, method and flags in prologue.
    FRAME_INFO_FIELD(SetupFrame, bool);
    // SaveUnusedCalleeRegs - save/restore used+unused callee-saved registers in prologue/epilogue.
    FRAME_INFO_FIELD(SaveUnusedCalleeRegs, bool);
    // AdjustSpReg - sub SP,#framesize in prologue and add SP,#framesize in epilogue.
    FRAME_INFO_FIELD(AdjustSpReg, bool);
    FRAME_INFO_FIELD(HasFloatRegs, bool);
    FRAME_INFO_FIELD(PushCallers, bool);

    using PositionedCallers = BitField<bool, 0, 1>;
    using PositionedCallees = PositionedCallers::NextFlag;
    using CallersRelativeFp = PositionedCallees::NextFlag;
    using CalleesRelativeFp = CallersRelativeFp::NextFlag;
    using SpillsRelativeFp = CalleesRelativeFp::NextFlag;
    using SaveFrameAndLinkRegs = SpillsRelativeFp::NextFlag;
    using SetupFrame = SaveFrameAndLinkRegs::NextFlag;
    using SaveUnusedCalleeRegs = SetupFrame::NextFlag;
    using AdjustSpReg = SaveUnusedCalleeRegs::NextFlag;
    using HasFloatRegs = AdjustSpReg::NextFlag;
    // Whether we need to push callers below stack during calls. Actually it is some kind of workaround, and we need
    // to rework it and make better solution, e.g allocate size for callers in a frame and save them there.
    using PushCallers = HasFloatRegs::NextFlag;

    // The following static 'constructors' are for situations
    // when we have to generate prologue/epilogue but there is
    // no codegen at hand (some tests etc.)
    // 'Leaf' means a prologue for a function which does not call
    // any other functions (library, runtime etc.)
    static FrameInfo LeafPrologue()
    {
        return FrameInfo(AdjustSpReg::Encode(true));
    }

    // 'Native' means just a regular prologue, that is used for native functions.
    // 'Native' is also used for Irtoc.
    static FrameInfo NativePrologue()
    {
        return FrameInfo(AdjustSpReg::Encode(true) | SaveFrameAndLinkRegs::Encode(true) |
                         SaveUnusedCalleeRegs::Encode(true));
    }

    // 'Full' means NativePrologue + setting up frame (set FP, method and flags),
    // i.e. a prologue for managed code.
    static FrameInfo FullPrologue()
    {
        return FrameInfo(AdjustSpReg::Encode(true) | SaveFrameAndLinkRegs::Encode(true) |
                         SaveUnusedCalleeRegs::Encode(true) | SetupFrame::Encode(true));
    }

    CFrameLayout::OffsetOrigin GetSpillsOffsetOrigin() const
    {
        return GetSpillsRelativeFp() ? CFrameLayout::OffsetOrigin::FP : CFrameLayout::OffsetOrigin::SP;
    }

#undef FRAME_INFO_GET_ATTR
#undef FRAME_INFO_SET_ATTR
#undef FRAME_INFO_ATTR
#undef FRAME_INFO_GET_FIELD
#undef FRAME_INFO_SET_FIELD
#undef FRAME_INFO_FIELD

private:
    uint32_t fields_ {0};
    int32_t frameSize_ {0};
    int16_t spillsCount_ {0};
    // Offset to caller registers storage (in words)
    int16_t callersOffset_ {0};
    int16_t calleesOffset_ {0};
    int16_t fpCallersOffset_ {0};
    int16_t fpCalleesOffset_ {0};
};
}  // namespace ark::compiler

#endif  // PANDA_FRAME_INFO_H
