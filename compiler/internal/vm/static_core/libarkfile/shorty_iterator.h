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

#ifndef LIBPANDAFILE_SHORTY_ITERATOR_H_
#define LIBPANDAFILE_SHORTY_ITERATOR_H_

#include <cstdint>
#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"

namespace ark::panda_file {
class ShortyIterator {
public:
    ShortyIterator() = default;

    explicit ShortyIterator(const uint16_t *shortyPtr) : shortyPtr_(shortyPtr)
    {
        shorty_ = *shortyPtr_++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        element_ = shorty_ & ELEMENT_MASK;
        elemIdx_ = 0;
        ASSERT(element_ != 0);
    }

    ~ShortyIterator() = default;

    DEFAULT_COPY_SEMANTIC(ShortyIterator);
    DEFAULT_MOVE_SEMANTIC(ShortyIterator);

    Type operator*() const
    {
        return Type(static_cast<Type::TypeId>(element_));
    }

    bool operator==(const ShortyIterator &it) const
    {
        return shortyPtr_ == it.shortyPtr_ && elemIdx_ == it.elemIdx_;
    }

    bool operator!=(const ShortyIterator &it) const
    {
        return !(*this == it);
    }

    ShortyIterator &operator++()
    {
        if (element_ == 0) {
            return *this;
        }
        IncrementWithoutCheck();
        if (element_ == 0) {
            *this = ShortyIterator();
        }
        return *this;
    }

    ShortyIterator operator++(int)  // NOLINT(cert-dcl21-cpp)
    {
        ShortyIterator prev = *this;
        ++(*this);
        return prev;
    }

    // Current method made for high performance iterator usage
    // !!! Do not use it in general way !!!
    ALWAYS_INLINE inline void IncrementWithoutCheck()
    {
        ASSERT(element_ != 0);

        ++elemIdx_;
        if (elemIdx_ == NUM_ELEMENTS_PER_16BIT) {
            shorty_ = *shortyPtr_++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            elemIdx_ = 0;
        } else {
            shorty_ >>= NUM_BITS_PER_ELEMENT;
        }
        element_ = shorty_ & ELEMENT_MASK;
    }

private:
    static constexpr uint32_t NUM_ELEMENTS_PER_16BIT = 4;
    static constexpr uint32_t NUM_BITS_PER_ELEMENT = 4;
    static constexpr uint32_t ELEMENT_MASK = 0xF;

    const uint16_t *shortyPtr_ {nullptr};
    uint16_t shorty_ {0};
    uint16_t element_ {0};
    uint16_t elemIdx_ {0};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_SHORTY_ITERATOR_H_
