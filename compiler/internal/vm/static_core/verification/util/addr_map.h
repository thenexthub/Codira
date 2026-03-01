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

#ifndef PANDA_VERIFIER_ADDR_MAP_HPP_
#define PANDA_VERIFIER_ADDR_MAP_HPP_

#include "libarkbase/macros.h"
#include "range.h"
#include "bit_vector.h"

#include <cstdint>

namespace ark::verifier {
class AddrMap {
public:
    AddrMap() = delete;
    AddrMap(const void *startPtr, const void *endPtr)
        : AddrMap(reinterpret_cast<uintptr_t>(startPtr), reinterpret_cast<uintptr_t>(endPtr))
    {
    }

    AddrMap(const void *startPtr, size_t size)
        : AddrMap(reinterpret_cast<uintptr_t>(startPtr), reinterpret_cast<uintptr_t>(startPtr) + size - 1)
    {
    }

    ~AddrMap() = default;

    DEFAULT_MOVE_SEMANTIC(AddrMap);
    DEFAULT_COPY_SEMANTIC(AddrMap);

    bool IsInAddressSpace(const void *ptr) const
    {
        return IsInAddressSpace(reinterpret_cast<uintptr_t>(ptr));
    }

    template <typename PtrType>
    PtrType AddrStart() const
    {
        return reinterpret_cast<PtrType>(addrRange_.Start());
    }

    template <typename PtrType>
    PtrType AddrEnd() const
    {
        return reinterpret_cast<PtrType>(addrRange_.Finish());
    }

    bool Mark(const void *addrPtr)
    {
        return Mark(addrPtr, addrPtr);
    }

    bool Mark(const void *addrStartPtr, const void *addrEndPtr)
    {
        return Mark(reinterpret_cast<uintptr_t>(addrStartPtr), reinterpret_cast<uintptr_t>(addrEndPtr));
    }

    void Clear()
    {
        Clear(addrRange_.Start(), addrRange_.Finish());
    }

    bool Clear(const void *addrPtr)
    {
        return Clear(addrPtr, addrPtr);
    }

    bool Clear(const void *addrStartPtr, const void *addrEndPtr)
    {
        return Clear(reinterpret_cast<uintptr_t>(addrStartPtr), reinterpret_cast<uintptr_t>(addrEndPtr));
    }

    bool HasMark(const void *addrPtr) const
    {
        return HasMarks(addrPtr, addrPtr);
    }

    bool HasMarks(const void *addrStartPtr, const void *addrEndPtr) const
    {
        return HasMarks(reinterpret_cast<uintptr_t>(addrStartPtr), reinterpret_cast<uintptr_t>(addrEndPtr));
    }

    bool HasCommonMarks(const AddrMap &rhs) const
    {
        // NOTE: work with different addr spaces
        ASSERT(addrRange_ == rhs.addrRange_);
        return BitVector::LazyAndThenIndicesOf<true>(bitMap_, rhs.bitMap_)().IsValid();
    }

    template <typename PtrType>
    bool GetFirstCommonMark(const AddrMap &rhs, PtrType *ptr) const
    {
        // NOTE: work with different addr spaces
        ASSERT(addrRange_ == rhs.addrRange_);
        Index<size_t> idx = BitVector::LazyAndThenIndicesOf<true>(bitMap_, rhs.bitMap_)();
        if (idx.IsValid()) {
            size_t offset = idx;
            *ptr = reinterpret_cast<PtrType>(addrRange_.IndexOf(offset));
            return true;
        }
        return false;
    }

    // NOTE(vdyadov): optimize this function, push blocks enumeration to bit vector level
    //                and refactor it to work with words and ctlz like intrinsics
    template <typename PtrType, typename Callback>
    void EnumerateMarkedBlocks(Callback cb) const
    {
        PtrType start = nullptr;
        PtrType end = nullptr;
        for (auto addr : addrRange_) {
            uintptr_t bitOffset = addrRange_.OffsetOf(addr);
            if (start == nullptr) {
                if (bitMap_[bitOffset]) {
                    start = reinterpret_cast<PtrType>(addr);
                }
            } else {
                if (bitMap_[bitOffset]) {
                    continue;
                }
                end = reinterpret_cast<PtrType>(addr - 1);
                if (!cb(start, end)) {
                    return;
                }
                start = nullptr;
            }
        }
        if (start != nullptr) {
            end = reinterpret_cast<PtrType>(addrRange_.Finish());
            cb(start, end);
        }
    }

    void InvertMarks()
    {
        bitMap_.Invert();
    }

    template <typename PtrType, typename Handler>
    void EnumerateMarksInScope(const void *addrStartPtr, const void *addrEndPtr, Handler handler) const
    {
        EnumerateMarksInScope<PtrType>(reinterpret_cast<uintptr_t>(addrStartPtr),
                                       reinterpret_cast<uintptr_t>(addrEndPtr), std::move(handler));
    }

private:
    AddrMap(uintptr_t addrFrom, uintptr_t addrTo) : addrRange_ {addrFrom, addrTo}, bitMap_ {addrRange_.Length()} {}

    bool IsInAddressSpace(uintptr_t addr) const
    {
        return addrRange_.Contains(addr);
    }

    bool Mark(uintptr_t addrFrom, uintptr_t addrTo)
    {
        if (!addrRange_.Contains(addrFrom) || !addrRange_.Contains(addrTo)) {
            return false;
        }
        ASSERT(addrFrom <= addrTo);
        bitMap_.Set(addrRange_.OffsetOf(addrFrom), addrRange_.OffsetOf(addrTo));
        return true;
    }

    bool Clear(uintptr_t addrFrom, uintptr_t addrTo)
    {
        if (!addrRange_.Contains(addrFrom) || !addrRange_.Contains(addrTo)) {
            return false;
        }
        ASSERT(addrFrom <= addrTo);
        bitMap_.Clr(addrRange_.OffsetOf(addrFrom), addrRange_.OffsetOf(addrTo));
        return true;
    }

    bool HasMarks(uintptr_t addrFrom, uintptr_t addrTo) const
    {
        if (!addrRange_.Contains(addrFrom) || !addrRange_.Contains(addrTo)) {
            return false;
        }
        ASSERT(addrFrom <= addrTo);
        Index<size_t> firstMarkIdx =
            bitMap_.LazyIndicesOf<1>(addrRange_.OffsetOf(addrFrom), addrRange_.OffsetOf(addrTo))();
        return firstMarkIdx.IsValid();
    }

    template <typename PtrType, typename Handler>
    void EnumerateMarksInScope(uintptr_t addrFrom, uintptr_t addrTo, Handler handler) const
    {
        ASSERT(addrFrom <= addrTo);
        auto from = addrRange_.OffsetOf(addrRange_.PutInBounds(addrFrom));
        auto to = addrRange_.OffsetOf(addrRange_.PutInBounds(addrTo));
        // upper range point is excluded
        for (uintptr_t idx = from; idx < to; idx++) {
            if (bitMap_[idx]) {
                if (!handler(reinterpret_cast<PtrType>(addrRange_.IndexOf(idx)))) {
                    return;
                }
            }
        }
    }

private:
    Range<uintptr_t> addrRange_;
    BitVector bitMap_;
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_ADDR_MAP_HPP_
