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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_INL_H
#define COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_INL_H

#include "securec.h"
#include "common_interfaces/objects/string/base_string.h"
#include "common_interfaces/objects/string/sliced_string.h"

namespace common {
template <typename Allocator, typename WriteBarrier, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>,
          objects_traits::enable_if_is_write_barrier<WriteBarrier>>
SlicedString *SlicedString::Create(Allocator &&allocator, WriteBarrier &&writeBarrier,
                                   ReadOnlyHandle<BaseString> parent)
{
    SlicedString *slicedString = SlicedString::Cast(
        std::invoke(std::forward<Allocator>(allocator), SlicedString::SIZE, ObjectType::SLICED_STRING));
    slicedString->SetMixHashcode(0);
    slicedString->SetParent(std::forward<WriteBarrier>(writeBarrier), parent.GetBaseObject());
    return slicedString;
}

inline uint32_t SlicedString::GetStartIndex() const
{
    uint32_t bits = GetStartIndexAndFlags();
    return StartIndexBits::Decode(bits);
}

inline void SlicedString::SetStartIndex(uint32_t startIndex)
{
    DCHECK_CC(startIndex <= SlicedString::MAX_STRING_LENGTH);
    uint32_t bits = GetStartIndexAndFlags();
    uint32_t newVal = StartIndexBits::Update(bits, startIndex);
    SetStartIndexAndFlags(newVal);
}

inline bool SlicedString::GetHasBackingStore() const
{
    uint32_t bits = GetStartIndexAndFlags();
    return HasBackingStoreBit::Decode(bits);
}

inline void SlicedString::SetHasBackingStore(bool hasBackingStore)
{
    uint32_t bits = GetStartIndexAndFlags();
    uint32_t newVal = HasBackingStoreBit::Update(bits, hasBackingStore);
    SetStartIndexAndFlags(newVal);
}

// Minimum length for a sliced string
template <bool VERIFY, typename ReadBarrier>
uint16_t SlicedString::Get(ReadBarrier &&readBarrier, int32_t index) const
{
    auto length = static_cast<int32_t>(GetLength());
    if constexpr (VERIFY) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }
    LineString *parent = LineString::Cast(GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
    DCHECK_CC(parent->IsLineString());
    if (parent->IsUtf8()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        common::Span<const uint8_t> sp(parent->GetDataUtf8() + GetStartIndex(), length);
        return sp[index];
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    common::Span<const uint16_t> sp(parent->GetDataUtf16() + GetStartIndex(), length);
    return sp[index];
}
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_INL_H