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

#ifndef PANDA_BIT_MEMORY_STREAM_H
#define PANDA_BIT_MEMORY_STREAM_H

#include "libarkbase/utils/bit_memory_region.h"
#include "libarkbase/utils/bit_utils.h"

#include <array>
#include <ios>

namespace ark {

template <typename Container>
class BitMemoryStreamOut {
public:
    explicit BitMemoryStreamOut(Container *data) : BitMemoryStreamOut(data, 0) {}
    BitMemoryStreamOut(Container *data, size_t offset) : data_(data), offset_(offset) {}

    void EnsureSpace(size_t length)
    {
        data_->resize(RoundUp(BitsToBytesRoundUp(offset_ + length), sizeof(uint32_t)));
    }

    void Write(size_t value, size_t length)
    {
        if (length != 0) {
            ASSERT(length <= (sizeof(value) * BITS_PER_BYTE));
            EnsureSpace(length);
            BitMemoryRegion region(data_->data(), offset_, length);
            region.Write(value, 0, length);
            offset_ += length;
        }
    }

    void Write(uint32_t *ptr, size_t payloadLength, size_t length)
    {
        ASSERT(payloadLength <= length);
        if (payloadLength != 0) {
            static constexpr size_t BITS_PER_WORD = BITS_PER_UINT32;
            EnsureSpace(length);
            BitMemoryRegion region(data_->data(), offset_, length);
            size_t i = 0;
            for (; i < payloadLength / BITS_PER_WORD; i++) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                region.Write(ptr[i], i * BITS_PER_WORD, BITS_PER_WORD);
            }
            size_t remainingSize = payloadLength % BITS_PER_WORD;
            if (remainingSize != 0) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                region.Write(ptr[i], i * BITS_PER_WORD, payloadLength % BITS_PER_WORD);
            }
        }
        offset_ += length;
    }

    virtual ~BitMemoryStreamOut() = default;

    NO_COPY_SEMANTIC(BitMemoryStreamOut);
    NO_MOVE_SEMANTIC(BitMemoryStreamOut);

private:
    Container *data_ {nullptr};
    size_t offset_ {0};
};

class BitMemoryStreamIn : private BitMemoryRegion<const uint8_t> {
public:
    using BitMemoryRegion::BitMemoryRegion;

    explicit BitMemoryStreamIn(const uint8_t *data)
        : BitMemoryRegion<const uint8_t>(data, std::numeric_limits<uint32_t>::max())
    {
    }

    template <typename T>
    T Read(size_t length)
    {
        ASSERT(length <= (sizeof(T) * BITS_PER_BYTE));
        T res = BitMemoryRegion::Read<T>(0, length);
        Advance(length);
        return res;
    }

    BitMemoryRegion ReadRegion(size_t length)
    {
        auto res = Subregion(0, length);
        Advance(length);
        return res;
    }

    ~BitMemoryStreamIn() override = default;

    NO_COPY_SEMANTIC(BitMemoryStreamIn);
    NO_MOVE_SEMANTIC(BitMemoryStreamIn);
};

}  // namespace ark

#endif  // PANDA_BIT_MEMORY_STREAM_H
