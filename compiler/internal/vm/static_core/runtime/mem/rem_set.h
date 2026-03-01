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

#ifndef PANDA_MEM_GC_G1_REM_SET_H
#define PANDA_MEM_GC_G1_REM_SET_H

#include <limits>
#include <runtime/include/mem/panda_containers.h>

namespace ark::mem {

namespace test {
class RemSetTest;
}  // namespace test

class RemSetLockConfig {
public:
    using CommonLock = os::memory::RecursiveMutex;
    using DummyLock = os::memory::DummyLock;
};

class Region;

/// @brief Set in the Region. To record the regions and cards reference to this region.
template <typename LockConfigT = RemSetLockConfig::CommonLock>
class RemSet {
public:
    RemSet();
    ~RemSet();

    NO_COPY_SEMANTIC(RemSet);
    NO_MOVE_SEMANTIC(RemSet);

    template <bool NEED_LOCK = true>
    void AddRef(const ObjectHeader *fromObjAddr, size_t offset);
    template <bool NEED_LOCK = true>
    void AddRef(const void *fromAddr);

    template <typename RegionPred, typename MemVisitor>
    void Iterate(const RegionPred &regionPred, const MemVisitor &visitor) const;
    template <typename Visitor>
    void IterateOverObjects(const Visitor &visitor) const;

    void Clear();

    template <bool NEED_LOCK = true>
    void InvalidateRegion(Region *invalidRegion);

    void RemoveInvalidRegions();

    size_t Size() const
    {
        return bitmaps_.size();
    }

    void Merge(RemSet<> *other);
    PandaUnorderedSet<Region *> GetDirtyRegions();

    /**
     * Used in the barrier. Record the reference from the region of objAddr to the region of valueAddr.
     * @param objAddr - address of the object
     * @param offset - offset in the object where value is stored
     * @param valueAddr - address of the reference in the field
     */
    template <bool NEED_LOCK = true>
    static void AddRefWithAddr(const ObjectHeader *objAddr, size_t offset, const ObjectHeader *valueAddr);

    /**
     * Used in the barrier. Record the reference from the region of fromAddr to the region of valueAddr.
     * @param fromAddr - pointer to the reference in the object where value is stored
     * @param valueAddr - address of the reference in the field
     */
    template <bool NEED_LOCK = true>
    static void AddRefWithAddr(const void *fromAddr, const ObjectHeader *valueAddr);

    void Dump(std::ostream &out);

    template <typename Visitor>
    void VisitBitmaps(const Visitor &visitor) const;

    static size_t GetIdxInBitmap(uintptr_t addr, uintptr_t bitmapBeginAddr);

    class Bitmap {
    public:
        static constexpr size_t GetBitmapSizeInBytes()
        {
            return sizeof(bitmap_);
        }

        static constexpr size_t GetNumBits()
        {
            return GetBitmapSizeInBytes() * BITS_PER_BYTE;
        }

        void Set(size_t idx)
        {
            size_t elemIdx = idx / ELEM_BITS;
            ASSERT(elemIdx < SIZE);
            size_t bitOffset = idx - elemIdx * ELEM_BITS;
            bitmap_[elemIdx] |= 1ULL << bitOffset;
        }

        bool Check(size_t idx) const
        {
            size_t elemIdx = idx / ELEM_BITS;
            ASSERT(elemIdx < SIZE);
            size_t bitOffset = idx - elemIdx * ELEM_BITS;
            return (bitmap_[elemIdx] & (1ULL << bitOffset)) != 0;
        }

        void AddBits(const Bitmap &other)
        {
            for (size_t i = 0; i < SIZE; ++i) {
                bitmap_[i] |= other.bitmap_[i];
            }
        }

        template <typename Visitor>
        void Iterate(const MemRange &range, const Visitor &visitor) const
        {
            size_t memSize = (range.GetEndAddress() - range.GetStartAddress()) / GetNumBits();
            uintptr_t startAddr = range.GetStartAddress();
            for (size_t i = 0; i < SIZE; ++i) {
                uintptr_t addr = startAddr + i * memSize * ELEM_BITS;
                uint64_t elem = bitmap_[i];
                while (elem > 0) {
                    if (elem & 1ULL) {
                        visitor(MemRange(addr, addr + memSize));
                    }
                    elem >>= 1ULL;
                    addr += memSize;
                }
            }
        }

    private:
        static constexpr size_t SIZE = 8U;
        static constexpr size_t ELEM_BITS = std::numeric_limits<uint64_t>::digits;
        std::array<uint64_t, SIZE> bitmap_ {0};
    };

private:
    LockConfigT remSetLock_;
    static constexpr float DEFAULT_LOAD_FACTOR = 0.7F;
    // NOTE(alovkov): make value a Set?
    PandaUnorderedMap<uintptr_t, Bitmap> bitmaps_;

    friend class test::RemSetTest;
};

class GlobalRemSet {
public:
    template <typename RegionContainer, typename RegionPred, typename MemVisitor>
    void ProcessRemSets(const RegionContainer &cont, const RegionPred &regionPred, const MemVisitor &visitor);

    template <typename MemVisitor>
    bool IterateOverUniqueRange(Region *region, MemRange range, const MemVisitor &visitor);

private:
    template <typename RegionPred>
    void FillBitmap(const RemSet<> &remSet, const RegionPred &regionPred);

    template <typename MemVisitor>
    void IterateOverBits(const MemVisitor &visitor) const;

    PandaUnorderedMap<uintptr_t, RemSet<>::Bitmap> bitmaps_;
};

}  // namespace ark::mem
#endif  // PANDA_MEM_GC_G1_REM_SET_H
