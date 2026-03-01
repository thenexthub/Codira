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

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)

#ifndef COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_INL_H
#define COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_INL_H

#include <vector>

#include "objects/string/base_string.h"
#include "objects/string/line_string.h"
#include "objects/utils/utf_utils.h"
#include "securec.h"

namespace common {

inline size_t LineString::ComputeSizeUtf8(uint32_t utf8Len)
{
    return DATA_OFFSET + utf8Len;
}

inline size_t LineString::ComputeSizeUtf16(uint32_t utf16Len)
{
    return DATA_OFFSET + utf16Len * sizeof(uint16_t);
}

inline size_t LineString::ObjectSize(const BaseString *str)
{
    uint32_t length = str->GetLength();
    return str->IsUtf16() ? ComputeSizeUtf16(length) : ComputeSizeUtf8(length);
}

inline size_t LineString::DataSize(BaseString *str)
{
    uint32_t length = str->GetLength();
    return str->IsUtf16() ? length * sizeof(uint16_t) : length;
}

template <bool VERIFY>
uint16_t LineString::Get(int32_t index) const
{
    auto length = static_cast<int32_t>(GetLength());
    if constexpr (VERIFY) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }
    if (!IsUtf16()) {
        common::Span<const uint8_t> sp(GetDataUtf8(), length);
        return sp[index];
    }
    common::Span<const uint16_t> sp(GetDataUtf16(), length);
    return sp[index];
}

inline void LineString::Set(uint32_t index, uint16_t src)
{
    DCHECK_CC(index < GetLength());
    if (IsUtf8()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *(reinterpret_cast<uint8_t *>(GetData()) + index) = static_cast<uint8_t>(src);
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *(GetData() + index) = src;
    }
}

inline void LineString::Trim(uint32_t newLength)
{
    DCHECK_CC(IsLineString());
    uint32_t oldLength = GetLength();
    DCHECK_CC(oldLength >= newLength);
    if (newLength == oldLength) {
        return;
    }
    bool isUtf8 = IsUtf8();
    size_t sizeNew = isUtf8 ?
        LineString::ComputeSizeUtf8(newLength): LineString::ComputeSizeUtf16(newLength);
    size_t sizeOld = isUtf8 ?
        LineString::ComputeSizeUtf8(oldLength): LineString::ComputeSizeUtf16(oldLength);
    sizeNew = AlignmentUp(sizeNew, ALIGNMENT_8_BYTES);
    sizeOld = AlignmentUp(sizeOld, ALIGNMENT_8_BYTES);
    size_t trimBytes = sizeOld - sizeNew;
    if (trimBytes > 0) {
        uintptr_t newEndAddr = ToUintPtr(this) + sizeNew;
        DCHECK_CC((newEndAddr % ALIGNMENT_8_BYTES) == 0);
        BaseRuntime::FillFreeObject(reinterpret_cast<void*>(newEndAddr), trimBytes);
    }
    InitLengthAndFlags(newLength, isUtf8);
}

template<typename ReadBarrier>
void LineString::WriteData(ReadBarrier &&readBarrier, BaseString *src, uint32_t start, uint32_t destSize,
                           uint32_t length)
{
    DCHECK_CC(IsLineString());
    if (IsUtf8()) {
        DCHECK_CC(src->IsUtf8());
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), src, buf);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (length != 0 && memcpy_s(GetDataUtf8Writable() + start, destSize, data, length) != EOK) {
            UNREACHABLE_CC();
        }
    } else if (src->IsUtf8()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), src, buf);
        Span<uint16_t> to(GetDataUtf16Writable() + start, length);
        Span<const uint8_t> from(data, length);
        for (uint32_t i = 0; i < length; i++) {
            to[i] = from[i];
        }
    } else {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), src, buf);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (length != 0 && memcpy_s(GetDataUtf16Writable() + start, destSize * sizeof(uint16_t), data,
                                    length * sizeof(uint16_t)) != EOK) {
            UNREACHABLE_CC();
        }
    }
}

inline bool LineString::CanBeCompressed(const LineString *string)
{
    if (string->IsUtf8()) {
        return BaseString::CanBeCompressed(string->GetDataUtf8(), string->GetLength());
    }
    return BaseString::CanBeCompressed(string->GetDataUtf16(), string->GetLength());
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
LineString *LineString::CreateFromUtf8(Allocator &&allocator, const uint8_t *utf8Data, uint32_t utf8Len,
                                       bool canBeCompress)
{
    LineString *string = nullptr;
    if (canBeCompress) {
        string = Create(std::forward<Allocator>(allocator), utf8Len, true);
        DCHECK_CC(string != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::copy(utf8Data, utf8Data + utf8Len, LineString::Cast(string)->GetDataUtf8Writable());
    } else {
        auto utf16Len = UtfUtils::Utf8ToUtf16Size(utf8Data, utf8Len);
        string = Create(allocator, utf16Len, false);
        DCHECK_CC(string != nullptr);
        [[maybe_unused]] auto len = UtfUtils::ConvertRegionUtf8ToUtf16(
            utf8Data, LineString::Cast(string)->GetDataUtf16Writable(), utf8Len, utf16Len);
        DCHECK_CC(len == utf16Len);
    }

    DCHECK_CC(canBeCompress == LineString::CanBeCompressed(string) && "Bad input canBeCompress!");
    return string;
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
LineString *LineString::CreateFromUtf8CompressedSubString(Allocator &&allocator,
                                                          const ReadOnlyHandle<BaseString> string, uint32_t offset,
                                                          uint32_t utf8Len)
{
    LineString *subString = Create(std::forward<Allocator>(allocator), utf8Len, true);
    DCHECK_CC(subString != nullptr);

    auto *utf8Data = ReadOnlyHandle<LineString>::Cast(string)->GetDataUtf8() + offset;
    std::copy(utf8Data, utf8Data + utf8Len, subString->GetDataUtf8Writable());
    DCHECK_CC(LineString::CanBeCompressed(subString) && "String cannot be compressed!");
    return subString;
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
LineString *LineString::CreateFromUtf16(Allocator &&allocator, const uint16_t *utf16Data, uint32_t utf16Len,
                                        bool canBeCompress)
{
    LineString *string = Create(std::forward<Allocator>(allocator), utf16Len, canBeCompress);
    DCHECK_CC(string != nullptr);

    if (canBeCompress) {
        CopyChars(string->GetDataUtf8Writable(), utf16Data, utf16Len);
    } else {
        uint32_t len = utf16Len * (sizeof(uint16_t) / sizeof(uint8_t));
        if (memcpy_s(string->GetDataUtf16Writable(), len, utf16Data, len) != EOK) {
            UNREACHABLE_CC();
        }
    }

    DCHECK_CC(canBeCompress == LineString::CanBeCompressed(string) && "Bad input canBeCompress!");
    return string;
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
LineString *LineString::Create(Allocator &&allocator, size_t length, bool compressed)
{
    size_t size = compressed ? LineString::ComputeSizeUtf8(length) : LineString::ComputeSizeUtf16(length);
    BaseObject *obj = std::invoke(std::forward<Allocator>(allocator), size, ObjectType::LINE_STRING);
    LineString *string = LineString::Cast(obj);
    string->InitLengthAndFlags(length, compressed);
    string->SetMixHashcode(0);
    return string;
}

}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_INL_H

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)

