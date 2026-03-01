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
#ifndef PANDA_RUNTIME_ENTRYPOINTS_STRING_INDEX_OF_H_
#define PANDA_RUNTIME_ENTRYPOINTS_STRING_INDEX_OF_H_

#include <cstdint>
#include <type_traits>
#include "libarkbase/utils/bit_utils.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/string_flatten.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/vm_handle.h"
#include "libarkbase/mem/mem.h"

namespace ark::intrinsics {

/* CC-OFFNXT(G.FUN.01-CPP) false positive */
#if !defined(PANDA_TARGET_ARM64) && !defined(PANDA_TARGET_ARM32) && !defined(PANDA_TARGET_AMD64) && \
    !defined(PANDA_TARGET_X86)
#error \
    "Unknown target architecture. IndexOf implementation assumes that target architecture is little-endian, \
check it and update the file."
#endif

namespace impl {
template <typename CharType, typename LoadType>
struct IndexOfConstants {
};

template <>
struct IndexOfConstants<uint8_t, uint64_t> {
    static constexpr uint64_t XOR_SUB_MASK = 0x0101010101010101ULL;
    static constexpr uint64_t NXOR_OR_MASK = 0x7F7F7F7F7F7F7F7FULL;
    static constexpr size_t VECTOR_SIZE = sizeof(uint64_t) / sizeof(uint8_t);
    static constexpr size_t BITS_PER_CHAR = sizeof(uint8_t) * BITS_PER_BYTE;
};

template <>
struct IndexOfConstants<uint16_t, uint64_t> {
    static constexpr uint64_t XOR_SUB_MASK = 0x0001000100010001ULL;
    static constexpr uint64_t NXOR_OR_MASK = 0x7FFF7FFF7FFF7FFFULL;
    static constexpr size_t VECTOR_SIZE = sizeof(uint64_t) / sizeof(uint16_t);
    static constexpr size_t BITS_PER_CHAR = sizeof(uint16_t) * BITS_PER_BYTE;
};

template <typename CharType, typename LoadType>
LoadType BuildSearchPattern(CharType character) = delete;

template <typename T>
size_t ClzInSwappedBytes(T val) = delete;

template <typename T>
T SwapBytes(T val) = delete;

#if defined(PANDA_TARGET_64) && defined(__SIZEOF_INT128__)
__extension__ using PandaWideUintT = unsigned __int128;
#else
using PandaWideUintT = uint64_t;
#endif

template <typename CharType, typename LoadType>
int32_t StringIndexOfSwar(CharType *data, int32_t offset, int32_t length, CharType character)
{
    static_assert(std::is_same_v<uint8_t, CharType> || std::is_same_v<uint16_t, CharType>,
                  "Only uint8_t and uint16_t CharTypes are supported");
    auto dataTyped = reinterpret_cast<CharType *>(data);
    auto end = dataTyped + length;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    auto ptr = dataTyped + offset;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto prefixEnd = reinterpret_cast<CharType *>(
        std::min((reinterpret_cast<uintptr_t>(ptr) & ~(sizeof(LoadType) - 1)) + sizeof(LoadType),
                 reinterpret_cast<uintptr_t>(end)));
    ASSERT(prefixEnd >= ptr);
    ASSERT(prefixEnd <= end);
    while (ptr < prefixEnd) {
        if (*ptr == character) {
            return ptr - dataTyped;
        }
        ++ptr;
    }
    auto mainEnd = reinterpret_cast<CharType *>(reinterpret_cast<uintptr_t>(end) & ~(sizeof(LoadType) - 1));
    ASSERT(mainEnd <= end);
    LoadType searchPattern = BuildSearchPattern<CharType, LoadType>(character);
    while (ptr < mainEnd) {
        LoadType value = *reinterpret_cast<LoadType *>(ptr);
        LoadType xoredValue = searchPattern ^ value;
        LoadType eq = (xoredValue - IndexOfConstants<CharType, LoadType>::XOR_SUB_MASK) &
                      ~(xoredValue | IndexOfConstants<CharType, LoadType>::NXOR_OR_MASK);
        if (eq != 0) {
            size_t idx = ClzInSwappedBytes<LoadType>(eq) / IndexOfConstants<CharType, LoadType>::BITS_PER_CHAR;
            ASSERT(ptr[idx] == character);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return ptr + idx - dataTyped;   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ptr += IndexOfConstants<CharType, LoadType>::VECTOR_SIZE;
    }
    while (ptr < end) {
        if (*ptr == character) {
            return ptr - dataTyped;
        }
        ++ptr;
    }
    return -1;
}

template <typename CharType, typename LoadType>
int32_t StringLastIndexOfSwar(CharType *data, int32_t offset, [[maybe_unused]] int32_t length, CharType character)
{
    static_assert(std::is_same_v<uint8_t, CharType> || std::is_same_v<uint16_t, CharType>,
                  "Only uint8_t and uint16_t CharTypes are supported");
    auto ptr = data + offset + 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto alignedDown = ark::AlignDown(reinterpret_cast<uintptr_t>(ptr), sizeof(LoadType));
    auto stop = reinterpret_cast<CharType *>(std::max(alignedDown, reinterpret_cast<uintptr_t>(data)));
    while (ptr > stop) {
        if (*(--ptr) == character) {
            return ptr - data;
        }
    }
    auto alignedUp = reinterpret_cast<CharType *>(ark::AlignUp(reinterpret_cast<uintptr_t>(data), sizeof(LoadType)));
    ASSERT(alignedUp >= data);
    LoadType searchPattern = BuildSearchPattern<CharType, LoadType>(character);
    while (ptr - IndexOfConstants<CharType, LoadType>::VECTOR_SIZE >= alignedUp) {
        ptr -= IndexOfConstants<CharType, LoadType>::VECTOR_SIZE;
        LoadType value = *reinterpret_cast<LoadType *>(ptr);
        LoadType xoredValue = SwapBytes(searchPattern ^ value);
        LoadType eq = (xoredValue - IndexOfConstants<CharType, LoadType>::XOR_SUB_MASK) &
                      ~(xoredValue | IndexOfConstants<CharType, LoadType>::NXOR_OR_MASK);
        if (eq != 0) {
            size_t idx = IndexOfConstants<CharType, LoadType>::VECTOR_SIZE - 1;
            idx = idx - Ctz(eq) / IndexOfConstants<CharType, LoadType>::BITS_PER_CHAR;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ASSERT(idx < (IndexOfConstants<CharType, LoadType>::VECTOR_SIZE) && ptr[idx] == character);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return ptr + idx - data;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    while (ptr >= data) {
        if (*ptr == character) {
            return ptr - data;
        }
        --ptr;
    }
    return -1;
}

template <>
inline uint64_t BuildSearchPattern<uint8_t, uint64_t>(uint8_t character)
{
    uint64_t mask = (static_cast<uint64_t>(character) << BITS_PER_BYTE) | character;
    mask = (mask << BITS_PER_UINT16) | mask;
    return (mask << BITS_PER_UINT32) | mask;
}

template <>
inline uint64_t BuildSearchPattern<uint16_t, uint64_t>(uint16_t character)
{
    uint64_t mask = (static_cast<uint64_t>(character) << BITS_PER_UINT16) | character;
    return (mask << BITS_PER_UINT32) | mask;
}

template <>
inline size_t ClzInSwappedBytes<uint64_t>(uint64_t val)
{
    // Don't use ReverseBytes as it may not be replaced with bswap
    return Clz(__builtin_bswap64(val));
}

template <>
inline uint64_t SwapBytes<uint64_t>(uint64_t val)
{
    return __builtin_bswap64(val);
}
#if defined(PANDA_TARGET_64) && defined(__SIZEOF_INT128__)
template <>
struct IndexOfConstants<uint16_t, PandaWideUintT> {
    static constexpr PandaWideUintT XOR_SUB_MASK =
        (static_cast<PandaWideUintT>(IndexOfConstants<uint16_t, uint64_t>::XOR_SUB_MASK) << BITS_PER_UINT64) |
        IndexOfConstants<uint16_t, uint64_t>::XOR_SUB_MASK;
    static constexpr PandaWideUintT NXOR_OR_MASK =
        (static_cast<PandaWideUintT>(IndexOfConstants<uint16_t, uint64_t>::NXOR_OR_MASK) << BITS_PER_UINT64) |
        IndexOfConstants<uint16_t, uint64_t>::NXOR_OR_MASK;
    static constexpr size_t VECTOR_SIZE = sizeof(PandaWideUintT) / sizeof(uint16_t);
    static constexpr size_t BITS_PER_CHAR = sizeof(uint16_t) * BITS_PER_BYTE;
};

template <>
inline PandaWideUintT BuildSearchPattern<uint16_t, PandaWideUintT>(uint16_t character)
{
    PandaWideUintT mask = (static_cast<uint64_t>(character) << BITS_PER_UINT16) | character;
    mask = (mask << BITS_PER_UINT32) | mask;
    return (mask << BITS_PER_UINT64) | mask;
}

template <>
inline size_t ClzInSwappedBytes<PandaWideUintT>(PandaWideUintT val)
{
    uint64_t hi = __builtin_bswap64(static_cast<uint64_t>(val >> BITS_PER_UINT64));
    uint64_t lo = __builtin_bswap64(static_cast<uint64_t>(val));
    if (lo == 0) {
        return BITS_PER_UINT64 + Clz(hi);
    }
    return Clz(lo);
}

template <>
inline PandaWideUintT SwapBytes<PandaWideUintT>(PandaWideUintT val)
{
    static_assert(sizeof(PandaWideUintT) == 16);
    uint64_t hi = static_cast<uint64_t>(val >> BITS_PER_UINT64);
    uint64_t lo = static_cast<uint64_t>(val);
    auto high = static_cast<PandaWideUintT>(__builtin_bswap64(lo)) << BITS_PER_UINT64;
    auto low = static_cast<PandaWideUintT>(__builtin_bswap64(hi));
    PandaWideUintT swapped = high | low;
    return swapped;
}
#endif

inline ALWAYS_INLINE int32_t Utf8StringIndexOfChar(uint8_t *data, size_t offset, size_t length, uint8_t character)
{
    if ((length - offset) >= IndexOfConstants<uint8_t, uint64_t>::VECTOR_SIZE * 2) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto pos = reinterpret_cast<uint8_t *>(memchr(data + offset, character, length - offset));
        return pos == nullptr ? -1 : pos - data;
    }
    return StringIndexOfSwar<uint8_t, uint64_t>(data, offset, length, character);
}

inline ALWAYS_INLINE int32_t Utf16StringIndexOfChar(uint16_t *data, size_t offset, size_t length, uint16_t character)
{
    if constexpr (sizeof(PandaWideUintT) >= sizeof(uint64_t)) {
        if ((length - offset) >= IndexOfConstants<uint16_t, PandaWideUintT>::VECTOR_SIZE * 2) {
            return StringIndexOfSwar<uint16_t, PandaWideUintT>(data, offset, length, character);
        }
    }
    return StringIndexOfSwar<uint16_t, uint64_t>(data, offset, length, character);
}

inline ALWAYS_INLINE int32_t Utf8StringLastIndexOfChar(uint8_t *data, size_t offset, size_t length, uint8_t ch)
{
    if ((length - offset) >= IndexOfConstants<uint8_t, uint64_t>::VECTOR_SIZE * 2) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto pos = reinterpret_cast<uint8_t *>(memrchr(data, ch, length - offset));
        return pos == nullptr ? -1 : pos - data;
    }
    return StringLastIndexOfSwar<uint8_t, uint64_t>(data, offset, length, ch);
}

inline ALWAYS_INLINE int32_t Utf16StringLastIndexOfChar(uint16_t *data, size_t offset, size_t length, uint16_t ch)
{
    if constexpr (sizeof(PandaWideUintT) >= sizeof(uint64_t)) {
        if ((length - offset) >= IndexOfConstants<uint16_t, PandaWideUintT>::VECTOR_SIZE * 2) {
            return StringIndexOfSwar<uint16_t, PandaWideUintT>(data, offset, length, ch);
        }
    }
    return StringLastIndexOfSwar<uint16_t, uint64_t>(data, offset, length, ch);
}

}  // namespace impl

// CC-OFFNXT(G.FUD.06) perf critical
inline int32_t StringIndexOfU16(void *str, uint16_t character, int32_t offset)
{
    auto string = reinterpret_cast<ark::coretypes::String *>(str);
    ASSERT(string != nullptr);
    ASSERT(string->IsLineString());
    ark::coretypes::LineString *lineString = ark::coretypes::String::Cast(string);

    auto length = lineString->GetLength();
    if (offset < 0) {
        offset = 0;
    } else if (static_cast<uint32_t>(offset) >= length) {
        return -1;
    }

    if (lineString->IsUtf16()) {
        return impl::Utf16StringIndexOfChar(lineString->GetDataUtf16(), offset, length, character);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (character > std::numeric_limits<uint8_t>::max()) {
        return -1;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return impl::Utf8StringIndexOfChar(lineString->GetDataMUtf8(), offset, length, character);
}

// CC-OFFNXT(G.FUD.06) perf critical
inline int32_t StringIndexOfU16(void *str, uint16_t character, int32_t offset, const LanguageContext &ctx)
{
    auto string = reinterpret_cast<ark::coretypes::String *>(str);
    ASSERT(string != nullptr);
    auto length = string->GetLength();
    // Empty string implies no match
    if (length == 0) {
        return -1;
    }
    if (offset < 0) {
        // All values with negative 'offset' are equivalent
        offset = 0;
    } else if (static_cast<uint32_t>(offset) >= length) {
        // No match if 'offset' is out of length
        return -1;
    }
    auto thread = ark::ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::String> stringHandle(thread, string);
    ark::coretypes::FlatStringInfo flatStr = ark::coretypes::FlatStringInfo::FlattenAllString(stringHandle, ctx);
    if (flatStr.IsUtf16()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return impl::Utf16StringIndexOfChar(flatStr.GetDataUtf16Writable(), offset, length, character);
    }
    if (character > std::numeric_limits<uint8_t>::max()) {
        return -1;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return impl::Utf8StringIndexOfChar(flatStr.GetDataUtf8Writable(), offset, length, character);
}

}  // namespace ark::intrinsics

#endif
