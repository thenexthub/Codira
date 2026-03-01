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

#ifndef LIBPANDAFILE_BYTECODE_INSTRUCTION_INL_H_
#define LIBPANDAFILE_BYTECODE_INSTRUCTION_INL_H_

#include "libarkfile/bytecode_instruction.h"
#include "libarkbase/macros.h"

namespace ark {

template <const BytecodeInstMode MODE>
template <class R, class S>
inline auto BytecodeInst<MODE>::ReadHelper(size_t byteoffset, size_t bytecount, size_t offset, size_t width) const
{
    constexpr size_t BYTE_WIDTH = 8;

    S v = 0;
    for (size_t i = 0; i < bytecount; i++) {
        S mask = static_cast<S>(ReadByte(byteoffset + i)) << (i * BYTE_WIDTH);
        v |= mask;
    }

    v >>= offset % BYTE_WIDTH;
    size_t leftShift = sizeof(R) * BYTE_WIDTH - width;

    // Do sign extension using arithmetic shift. It's implementation defined
    // so we check such behavior using static assert
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    static_assert((-1 >> 1) == -1);

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return static_cast<R>(v << leftShift) >> leftShift;
}

template <const BytecodeInstMode MODE>
template <size_t OFFSET, size_t WIDTH, bool IS_SIGNED>
inline auto BytecodeInst<MODE>::Read() const
{
    constexpr size_t BYTE_WIDTH = 8;
    constexpr size_t BYTE_OFFSET = OFFSET / BYTE_WIDTH;
    constexpr size_t BYTE_OFFSET_END = (OFFSET + WIDTH + BYTE_WIDTH - 1) / BYTE_WIDTH;
    constexpr size_t BYTE_COUNT = BYTE_OFFSET_END - BYTE_OFFSET;

    using StorageType = helpers::TypeHelperT<BYTE_COUNT * BYTE_WIDTH, false>;
    using ReturnType = helpers::TypeHelperT<WIDTH, IS_SIGNED>;

    return ReadHelper<ReturnType, StorageType>(BYTE_OFFSET, BYTE_COUNT, OFFSET, WIDTH);
}

template <const BytecodeInstMode MODE>
template <bool IS_SIGNED>
inline auto BytecodeInst<MODE>::Read64(size_t offset, size_t width) const
{
    constexpr size_t BIT64 = 64;
    constexpr size_t BYTE_WIDTH = 8;

    ASSERT((offset % BYTE_WIDTH) + width <= BIT64);

    size_t byteoffset = offset / BYTE_WIDTH;
    size_t byteoffsetEnd = (offset + width + BYTE_WIDTH - 1) / BYTE_WIDTH;
    size_t bytecount = byteoffsetEnd - byteoffset;

    using StorageType = helpers::TypeHelperT<BIT64, false>;
    using ReturnType = helpers::TypeHelperT<BIT64, IS_SIGNED>;

    return ReadHelper<ReturnType, StorageType>(byteoffset, bytecount, offset, width);
}

template <const BytecodeInstMode MODE>
template <typename EnumT>
inline size_t BytecodeInst<MODE>::GetSize() const
{
    return Size(GetFormat<EnumT>());
}

#include <libarkfile/include/bytecode_instruction-inl_gen.h>

}  // namespace ark

#endif  // LIBPANDAFILE_BYTECODE_INSTRUCTION_INL_H_
