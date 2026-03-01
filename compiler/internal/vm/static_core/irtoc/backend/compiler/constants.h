/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_IRTOC_COMPILER_CONSTANTS_H
#define PANDA_IRTOC_COMPILER_CONSTANTS_H

#include "libarkbase/utils/cframe_layout.h"

namespace ark::compiler {
/**
 *  fix number of spill slots such that RETURN handlers are able to generate epilogue correspondent to
 *  InterpreterEntry prologue independently from number of spill slots in handler itself.
 *  Should be synced with spills_count_max in interpreter.irt
 */
constexpr size_t IRTOC_NUM_SPILL_SLOTS = 32;

constexpr auto IRTOC_CFRAME_LAYOUT_X86_64 = CFrameLayout {Arch::X86_64, IRTOC_NUM_SPILL_SLOTS};
constexpr auto IRTOC_CFRAME_LAYOUT_AARCH64 = CFrameLayout {Arch::AARCH64, IRTOC_NUM_SPILL_SLOTS};

constexpr size_t GetIrtocCFrameSizeBytes(Arch arch)
{
    constexpr auto BYTES = CFrameLayout::OffsetUnit::BYTES;
    switch (arch) {
        case Arch::X86_64:
            return IRTOC_CFRAME_LAYOUT_X86_64.GetFrameSize<BYTES>();
        case Arch::AARCH64:
            return IRTOC_CFRAME_LAYOUT_AARCH64.GetFrameSize<BYTES>();
        default:
            UNREACHABLE();
    }
}

constexpr size_t GetIrtocCFrameSizeBytesBelowFP(Arch arch)
{
    constexpr auto BYTES = CFrameLayout::OffsetUnit::BYTES;
    size_t pointerSize = PointerSize(arch);
    switch (arch) {
        case Arch::X86_64:
            return IRTOC_CFRAME_LAYOUT_X86_64.GetFrameSize<BYTES>() - 2 * pointerSize;  // 2 : Two slots for FP & LR
        case Arch::AARCH64:
            return IRTOC_CFRAME_LAYOUT_AARCH64.GetFrameSize<BYTES>() - 2 * pointerSize;  // 2 : Two slots for FP & LR
        default:
            UNREACHABLE();
    }
}
}  // namespace ark::compiler

#endif  // PANDA_IRTOC_COMPILER_CONSTANTS_H
