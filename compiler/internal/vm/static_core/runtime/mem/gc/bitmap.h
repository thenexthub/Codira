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

#ifndef PANDA_MEM_GC_BITMAP_H
#define PANDA_MEM_GC_BITMAP_H

#include <securec.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <atomic>

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/math_helpers.h"
#include "libarkbase/utils/span.h"

namespace ark::mem {

/// Abstract base class. Constructor/destructor are protected. No virtual function to avoid dynamic polymorphism.
class Bitmap {
public:
    using BitmapWordType = uintptr_t;

    size_t Size() const
    {
        return bitsize_;
    }

    void ClearAllBits()
    {
        memset_s(bitmap_.Data(), bitmap_.SizeBytes(), 0, bitmap_.SizeBytes());
    }

    void CopyTo(Bitmap *dest) const;

    Span<BitmapWordType> GetBitMap()
    {
        return bitmap_;
    }

    size_t GetSetBitCount()
    {
        size_t countMarkBits = 0;
        for (BitmapWordType word : bitmap_) {
            countMarkBits += static_cast<size_t>(Popcount(word));
        }
        return countMarkBits;
    }

    static const size_t BITSPERBYTE = 8;
    static const size_t BITSPERWORD = BITSPERBYTE * sizeof(BitmapWordType);
    static constexpr size_t LOG_BITSPERBYTE = ark::helpers::math::GetIntLog2(static_cast<uint64_t>(BITSPERBYTE));
    static constexpr size_t LOG_BITSPERWORD = ark::helpers::math::GetIntLog2(static_cast<uint64_t>(BITSPERWORD));

protected:
    /**
     * @brief Set the bit indexed by bit_offset.
     * @param bit_offset - index of the bit to set.
     */
    void SetBit(size_t bitOffset)
    {
        CheckBitOffset(bitOffset);
        bitmap_[GetWordIdx(bitOffset)] |= GetBitMask(bitOffset);
    }

    /**
     * @brief Clear the bit indexed by bit_offset.
     * @param bit_offset - index of the bit to clear.
     */
    void ClearBit(size_t bitOffset)
    {
        CheckBitOffset(bitOffset);
        bitmap_[GetWordIdx(bitOffset)] &= ~GetBitMask(bitOffset);
    }

    /**
     * @brief Test the bit indexed by bit_offset.
     * @param bit_offset - index of the bit to test.
     * @return Returns value of indexed bit.
     */
    bool TestBit(size_t bitOffset) const
    {
        CheckBitOffset(bitOffset);
        return (bitmap_[GetWordIdx(bitOffset)] & GetBitMask(bitOffset)) != 0;
    }

    /**
     * @brief Atomically set bit indexed by bit_offset. If the bit is not set, set it atomically. Otherwise, do nothing.
     * @param bit_offset - index of the bit to set.
     * @return Returns old value of the bit.
     */
    bool AtomicTestAndSetBit(size_t bitOffset);

    /**
     * @brief Atomically clear bit corresponding to addr. If the bit is set, clear it atomically. Otherwise, do nothing.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     * @return Returns old value of the bit.
     */
    bool AtomicTestAndClearBit(size_t bitOffset);

    /**
     * @brief Atomically test bit corresponding to addr.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     * @return Returns the value of the bit.
     */
    bool AtomicTestBit(size_t bitOffset);

    /**
     * @brief Iterates over all bits of bitmap sequentially.
     * @tparam VisitorType
     * @param visitor - function pointer or functor.
     */
    template <typename VisitorType>
    void IterateOverBits(const VisitorType &visitor)
    {
        IterateOverBitsInRange(0, Size(), visitor);
    }

    /**
     * @brief Iterates over marked bits in range [begin, end) sequentially.
     * Finish iteration if the visitor returns false.
     * @tparam atomic - true if want to set bit atomically.
     * @tparam VisitorType
     * @param begin - beginning index of the range, inclusive.
     * @param end - end index of the range, exclusive.
     * @param visitor - function pointer or functor.
     */
    // default value exists only with backward-compatibility with js_runtime, should be removed in the future
    template <bool ATOMIC = false, typename VisitorType>
    void IterateOverSetBitsInRange(size_t begin, size_t end, const VisitorType &visitor)
    {
        CheckBitRange(begin, end);
        if (UNLIKELY(begin == end)) {
            return;
        }

        auto bitmapWord = GetBitmapWord<ATOMIC>(begin);
        auto offsetWithinWord = GetBitIdxWithinWord(begin);
        // first word, clear bits before begin
        bitmapWord &= GetRangeBitMask(offsetWithinWord, BITSPERWORD);
        auto offsetWordBegin = GetWordIdx(begin) * BITSPERWORD;
        const bool rightAligned = (GetBitIdxWithinWord(end) == 0);
        const auto offsetLastWordBegin = GetWordIdx(end) * BITSPERWORD;
        do {
            if (offsetWordBegin == offsetLastWordBegin && !rightAligned) {
                // last partial word, clear bits after right boundary
                auto mask = GetRangeBitMask(0, GetBitIdxWithinWord(end));
                bitmapWord &= mask;
            }
            // loop over bits of bitmap_word
            while (offsetWithinWord < BITSPERWORD) {
                if (bitmapWord == 0) {
                    break;
                }
                offsetWithinWord = static_cast<size_t>(Ctz(bitmapWord));
                if (!visitor(offsetWordBegin + offsetWithinWord)) {
                    return;
                }
                bitmapWord &= ~GetBitMask(offsetWithinWord);
            }

            offsetWordBegin += BITSPERWORD;
            if (offsetWordBegin >= end) {
                break;
            }

            bitmapWord = GetBitmapWord<ATOMIC>(offsetWordBegin);
            offsetWithinWord = 0;
        } while (true);
    }

    /**
     * @brief Iterates over marked bits of bitmap sequentially.
     * Finish iteration if the visitor returns false.
     * @tparam atomic - true if want to iterate with atomic reading.
     * @tparam VisitorType
     * @param visitor - function pointer or functor.
     */
    template <bool ATOMIC, typename VisitorType>
    void IterateOverSetBits(const VisitorType &visitor)
    {
        IterateOverSetBitsInRange<ATOMIC, VisitorType>(0, Size(), visitor);
    }

    /**
     * @brief Iterates over all bits in range [begin, end) sequentially.
     * @tparam VisitorType
     * @param begin - beginning index of the range, inclusive.
     * @param end - end index of the range, exclusive.
     * @param visitor - function pointer or functor.
     */
    template <typename VisitorType>
    void IterateOverBitsInRange(size_t begin, size_t end, const VisitorType &visitor)
    {
        CheckBitRange(begin, end);
        for (size_t i = begin; i < end; ++i) {
            visitor(i);
        }
    }

    /**
     * @brief Clear all bits in range [begin, end).
     * @param begin - beginning index of the range, inclusive.
     * @param end - end index of the range, exclusive.
     */
    void ClearBitsInRange(size_t begin, size_t end);

    /**
     * @brief Set all bits in range [begin, end). [begin, end) must be within a BitmapWord.
     * @param begin - beginning index of the range, inclusive.
     * @param end - end index of the range, exclusive.
     */
    void SetRangeWithinWord(size_t begin, size_t end)
    {
        if (LIKELY(begin != end)) {
            ModifyRangeWithinWord<true>(begin, end);
        }
    }

    /**
     * @brief Clear all bits in range [begin, end). [begin, end) must be within a BitmapWord.
     * @param begin - beginning index of the range, inclusive.
     * @param end - end index of the range, exclusive.
     */
    void ClearRangeWithinWord(size_t begin, size_t end)
    {
        if (LIKELY(begin != end)) {
            ModifyRangeWithinWord<false>(begin, end);
        }
    }

    /**
     * @brief Set all BitmapWords in index range [begin, end).
     * @param begin - beginning BitmapWord index of the range, inclusive.
     * @param end - end BitmapWord index of the range, exclusive.
     */
    void SetWords([[maybe_unused]] size_t wordBegin, [[maybe_unused]] size_t wordEnd)
    {
        ASSERT(wordBegin <= wordEnd);
        if (UNLIKELY(wordBegin == wordEnd)) {
            return;
        }
        memset_s(&bitmap_[wordBegin], (wordEnd - wordBegin) * sizeof(BitmapWordType), ~static_cast<unsigned char>(0),
                 (wordEnd - wordBegin) * sizeof(BitmapWordType));
    }

    /**
     * @brief Clear all BitmapWords in index range [begin, end).
     * @param begin - beginning BitmapWord index of the range, inclusive.
     * @param end - end BitmapWord index of the range, exclusive.
     */
    void ClearWords([[maybe_unused]] size_t wordBegin, [[maybe_unused]] size_t wordEnd)
    {
        ASSERT(wordBegin <= wordEnd);
        if (UNLIKELY(wordBegin == wordEnd)) {
            return;
        }
        memset_s(&bitmap_[wordBegin], (wordEnd - wordBegin) * sizeof(BitmapWordType), static_cast<unsigned char>(0),
                 (wordEnd - wordBegin) * sizeof(BitmapWordType));
    }

    template <bool ATOMIC>
    size_t FindHighestPrecedingOrSameBit(size_t begin)
    {
        auto wordBeginOffset = GetWordIdx(begin) * BITSPERWORD;
        auto offsetWithinWord = GetBitIdxWithinWord(begin);
        auto mask = ~static_cast<BitmapWordType>(0) >> (BITSPERWORD - offsetWithinWord - 1);
        auto bitmapWord = GetBitmapWord<ATOMIC>(begin) & mask;

        while (true) {
            if (bitmapWord != 0) {
                offsetWithinWord = BITSPERWORD - static_cast<size_t>(Clz(bitmapWord)) - 1;
                return wordBeginOffset + offsetWithinWord;
            }

            if (wordBeginOffset < BITSPERWORD) {
                break;
            }

            wordBeginOffset -= BITSPERWORD;
            bitmapWord = GetBitmapWord<ATOMIC>(wordBeginOffset);
        }

        return begin;
    }

    explicit Bitmap(BitmapWordType *bitmap, size_t bitsize)
        : bitmap_(bitmap, (AlignUp(bitsize, BITSPERWORD) >> LOG_BITSPERWORD)), bitsize_(bitsize)
    {
    }
    ~Bitmap() = default;
    // do we need special copy/move constructor?
    NO_COPY_SEMANTIC(Bitmap);
    NO_MOVE_SEMANTIC(Bitmap);

private:
    Span<BitmapWordType> bitmap_;
    size_t bitsize_ = 0;

    /**
     * @brief Compute word index from bit index.
     * @param bit_offset - bit index.
     * @return Returns BitmapWord Index of bit_offset.
     */
    static size_t GetWordIdx(size_t bitOffset)
    {
        return bitOffset >> LOG_BITSPERWORD;
    }

    /**
     * @brief Compute bit index within a BitmapWord from bit index.
     * @param bit_offset - bit index.
     * @return Returns bit index within a BitmapWord.
     */
    size_t GetBitIdxWithinWord(size_t bitOffset) const
    {
        CheckBitOffset(bitOffset);
        constexpr auto BIT_INDEX_MASK = static_cast<size_t>((1ULL << LOG_BITSPERWORD) - 1);
        return bitOffset & BIT_INDEX_MASK;
    }

    /**
     * @brief Compute bit mask from bit index.
     * @param bit_offset - bit index.
     * @return Returns bit mask of bit_offset.
     */
    BitmapWordType GetBitMask(size_t bitOffset) const
    {
        return 1ULL << GetBitIdxWithinWord(bitOffset);
    }

    /**
     * @brief Compute bit mask of range [begin_within_word, end_within_word).
     * @param begin_within_word - beginning index within word, in range [0, BITSPERWORD).
     * @param end_within_word - end index within word, in range [0, BITSPERWORD]. Make sure end_within_word is
     * BITSPERWORD(instead of 0) if you want to cover from certain bit to last. [0, 0) is the only valid case when
     * end_within_word is 0.
     * @return Returns bit mask.
     */
    BitmapWordType GetRangeBitMask(size_t beginWithinWord, size_t endWithinWord) const
    {
        ASSERT(beginWithinWord < BITSPERWORD);
        ASSERT(endWithinWord <= BITSPERWORD);
        ASSERT(beginWithinWord <= endWithinWord);
        auto endMask = (endWithinWord == BITSPERWORD) ? ~static_cast<BitmapWordType>(0) : GetBitMask(endWithinWord) - 1;
        return endMask - (GetBitMask(beginWithinWord) - 1);
    }

    /// @brief Check if bit_offset is valid.
    void CheckBitOffset([[maybe_unused]] size_t bitOffset) const
    {
        ASSERT(bitOffset <= Size());
    }

    /**
     * @brief According to SET, set or clear range [begin, end) within a BitmapWord.
     * @param begin - beginning global bit index.
     * @param end - end global bit index.
     */
    template <bool SET>
    ALWAYS_INLINE void ModifyRangeWithinWord(size_t begin, size_t end)
    {
        CheckBitRange(begin, end);

        if (UNLIKELY(begin == end)) {
            return;
        }

        BitmapWordType mask;
        if (end % BITSPERWORD == 0) {
            ASSERT(GetWordIdx(end) - GetWordIdx(begin) == 1);
            mask = GetRangeBitMask(GetBitIdxWithinWord(begin), BITSPERWORD);
        } else {
            ASSERT(GetWordIdx(end) == GetWordIdx(begin));
            mask = GetRangeBitMask(GetBitIdxWithinWord(begin), GetBitIdxWithinWord(end));
        }

        if (SET) {
            bitmap_[GetWordIdx(begin)] |= mask;
        } else {
            bitmap_[GetWordIdx(begin)] &= ~mask;
        }
    }

    /// @brief Check if bit range [begin, end) is valid.
    void CheckBitRange([[maybe_unused]] size_t begin, [[maybe_unused]] size_t end) const
    {
        ASSERT(begin < Size());
        ASSERT(end <= Size());
        ASSERT(begin <= end);
    }

    template <bool ATOMIC>
    BitmapWordType GetBitmapWord(size_t bitOffset)
    {
        auto index = GetWordIdx(bitOffset);
        if constexpr (!ATOMIC) {
            return bitmap_[index];
        }

        auto *wordAddr = reinterpret_cast<std::atomic<BitmapWordType> *>(&bitmap_[index]);
        // Atomic with seq_cst order reason: datarace with AtomicTestAndSet
        return wordAddr->load(std::memory_order_seq_cst);
    }
};

/**
 * Memory bitmap, binding a continuous range of memory to a bitmap.
 * One bit represents BYTESPERCHUNK bytes of memory.
 */
template <size_t BYTESPERCHUNK = 1, typename PointerType = ObjectPointerType>
class MemBitmap : public Bitmap {
public:
    explicit MemBitmap(void *memAddr, size_t heapSize, void *bitmapAddr)
        : Bitmap(static_cast<BitmapWordType *>(bitmapAddr), heapSize / BYTESPERCHUNK),
          beginAddr_(ToPointerType(memAddr)),
          endAddr_(beginAddr_ + heapSize)
    {
    }
    NO_COPY_SEMANTIC(MemBitmap);
    NO_MOVE_SEMANTIC(MemBitmap);

    /**
     * @brief Reinitialize the MemBitmap for new memory range.
     * The size of range will be the same as the initial
     * because we reuse the same bitmap storage.
     * @param mem_addr - start addr of the new range.
     */
    void ReInitializeMemoryRange(void *memAddr)
    {
        beginAddr_ = ToPointerType(memAddr);
        endAddr_ = beginAddr_ + MemSizeInBytes();
        Bitmap::ClearAllBits();
    }

    inline static constexpr size_t GetBitMapSizeInByte(size_t heapSize)
    {
        ASSERT(heapSize % BYTESPERCHUNK == 0);
        size_t bitSize = heapSize / BYTESPERCHUNK;
        return (AlignUp(bitSize, BITSPERWORD) >> Bitmap::LOG_BITSPERWORD) * sizeof(BitmapWordType);
    }

    ~MemBitmap() = default;

    size_t MemSizeInBytes() const
    {
        return Size() * BYTESPERCHUNK;
    }

    inline std::pair<uintptr_t, uintptr_t> GetHeapRange()
    {
        return {beginAddr_, endAddr_};
    }

    /**
     * @brief Set bit corresponding to addr.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     */
    void Set(void *addr)
    {
        CheckAddrValidity(addr);
        SetBit(AddrToBitOffset(ToPointerType(addr)));
    }

    /**
     * @brief Clear bit corresponding to addr.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     */
    void Clear(void *addr)
    {
        CheckAddrValidity(addr);
        ClearBit(AddrToBitOffset(ToPointerType(addr)));
    }

    /// @brief Clear bits corresponding to addr range [begin, end).
    ALWAYS_INLINE void ClearRange(void *begin, void *end)
    {
        CheckHalfClosedHalfOpenAddressRange(begin, end);
        ClearBitsInRange(AddrToBitOffset(ToPointerType(begin)), EndAddrToBitOffset(ToPointerType(end)));
    }

    /**
     * @brief Test bit corresponding to addr.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     */
    bool Test(const void *addr) const
    {
        CheckAddrValidity(addr);
        return TestBit(AddrToBitOffset(ToPointerType(addr)));
    }

    /**
     * @brief Test bit corresponding to addr if addr is valid.
     * @return value of indexed bit if addr is valid. If addr is invalid then false
     */
    bool TestIfAddrValid(const void *addr) const
    {
        if (IsAddrValid(addr)) {
            return TestBit(AddrToBitOffset(ToPointerType(addr)));
        }
        return false;
    }

    /**
     * @brief Atomically set bit corresponding to addr. If the bit is not set, set it atomically. Otherwise, do nothing.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     * @return Returns old value of the bit.
     */
    bool AtomicTestAndSet(void *addr)
    {
        CheckAddrValidity(addr);
        return AtomicTestAndSetBit(AddrToBitOffset(ToPointerType(addr)));
    }

    /**
     * @brief Atomically clear bit corresponding to addr. If the bit is set, clear it atomically. Otherwise, do nothing.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     * @return Returns old value of the bit.
     */
    bool AtomicTestAndClear(void *addr)
    {
        CheckAddrValidity(addr);
        return AtomicTestAndClearBit(AddrToBitOffset(ToPointerType(addr)));
    }

    /**
     * @brief Atomically test bit corresponding to addr.
     * @param addr - addr must be aligned to BYTESPERCHUNK.
     * @return Returns the value of the bit.
     */
    bool AtomicTest(const void *addr)
    {
        CheckAddrValidity(addr);
        return AtomicTestBit(AddrToBitOffset(ToPointerType(addr)));
    }

    /// @brief Find first marked chunk.
    template <bool ATOMIC = false>
    void *FindFirstMarkedChunks()
    {
        void *firstMarked = nullptr;
        IterateOverSetBits<ATOMIC>([&firstMarked, this](size_t bitOffset) {
            firstMarked = BitOffsetToAddr(bitOffset);
            return false;
        });
        return firstMarked;
    }

    /// @brief Iterates over marked chunks of memory sequentially.
    template <bool ATOMIC = false, typename MemVisitor>
    void IterateOverMarkedChunks(const MemVisitor &visitor)
    {
        IterateOverSetBits<ATOMIC>([&visitor, this](size_t bitOffset) {
            visitor(BitOffsetToAddr(bitOffset));
            return true;
        });
    }

    /// @brief Iterates over all chunks of memory sequentially.
    template <typename MemVisitor>
    void IterateOverChunks(const MemVisitor &visitor)
    {
        IterateOverBits([&visitor, this](size_t bitOffset) { visitor(BitOffsetToAddr(bitOffset)); });
    }

    /// @brief Iterates over marked chunks of memory in range [begin, end) sequentially.
    template <bool ATOMIC = false, typename MemVisitor, typename T = void *>
    void IterateOverMarkedChunkInRange(void *begin, void *end, const MemVisitor &visitor)
    {
        IterateOverMarkedChunkInRangeInterruptible<ATOMIC>(begin, end, [&visitor](void *obj) {
            visitor(obj);
            return true;
        });
    }

    /**
     * @brief Iterates over marked chunks of memory in range [begin, end) sequentially and stops iteration if visitor
     * returns false
     */
    template <bool ATOMIC = false, typename MemVisitor, typename T = void *>
    void IterateOverMarkedChunkInRangeInterruptible(void *begin, void *end, const MemVisitor &visitor)
    {
        CheckHalfClosedHalfOpenAddressRange(begin, end);
        IterateOverSetBitsInRange<ATOMIC>(FindHighestPrecedingOrSameBit<ATOMIC>(AddrToBitOffset(ToPointerType(begin))),
                                          EndAddrToBitOffset(ToPointerType(end)), [&visitor, this](size_t bitOffset) {
                                              return visitor(BitOffsetToAddr(bitOffset));
                                          });
    }

    /// @brief Call visitor for single allocated object in humongous region
    template <bool ATOMIC, typename MemVisitor>
    void CallForMarkedChunkInHumongousRegion(void *begin, const MemVisitor &visitor)
    {
        bool marked;
        if constexpr (ATOMIC) {
            marked = AtomicTest(begin);
        } else {
            marked = Test(begin);
        }
        if (marked) {
            visitor(begin);
        }
    }

    /// @brief Iterates over all chunks of memory in range [begin, end) sequentially.
    template <typename MemVisitor>
    void IterateOverChunkInRange(void *begin, void *end, const MemVisitor &visitor)
    {
        CheckHalfClosedHalfOpenAddressRange(begin, end);
        IterateOverBitsInRange(AddrToBitOffset(ToPointerType(begin)), EndAddrToBitOffset(ToPointerType(end)),
                               [&visitor, this](size_t bitOffset) { visitor(BitOffsetToAddr(bitOffset)); });
    }

    bool IsAddrInRange(const void *addr) const
    {
        return addr >= ToVoidPtr(beginAddr_) && addr < ToVoidPtr(endAddr_);
    }

    template <class T>
    static constexpr PointerType ToPointerType(T *val)
    {
        return static_cast<PointerType>(ToUintPtr(val));
    }

private:
    /// @brief Computes bit offset from addr.
    size_t AddrToBitOffset(PointerType addr) const
    {
        return (addr - beginAddr_) / BYTESPERCHUNK;
    }

    size_t EndAddrToBitOffset(PointerType addr) const
    {
        return (AlignUp(addr, BYTESPERCHUNK) - beginAddr_) / BYTESPERCHUNK;
    }

    /// @brief Computes address from bit offset.
    void *BitOffsetToAddr(size_t bitOffset) const
    {
        return ToVoidPtr(beginAddr_ + bitOffset * BYTESPERCHUNK);
    }

    /// @brief Check if addr is valid.
    void CheckAddrValidity([[maybe_unused]] const void *addr) const
    {
        ASSERT(IsAddrInRange(addr));
        ASSERT((ToPointerType(addr) - beginAddr_) % BYTESPERCHUNK == 0);
    }

    /**
     * @brief Check if addr is valid with returned value.
     * @return true if addr is valid
     */
    bool IsAddrValid(const void *addr) const
    {
        return IsAddrInRange(addr) && (ToPointerType(addr) - beginAddr_) % BYTESPERCHUNK == 0;
    }

    /// @brief Check if [begin, end) is a valid address range.
    void CheckHalfClosedHalfOpenAddressRange([[maybe_unused]] void *begin, [[maybe_unused]] void *end) const
    {
        CheckAddrValidity(begin);
        ASSERT(ToPointerType(end) >= beginAddr_);
        ASSERT(ToPointerType(end) <= endAddr_);
        ASSERT(ToPointerType(begin) <= ToPointerType(end));
    }

    PointerType beginAddr_ {0};
    PointerType endAddr_ {0};
};

using MarkBitmap = MemBitmap<DEFAULT_ALIGNMENT_IN_BYTES>;

}  // namespace ark::mem
#endif  // PANDA_MEM_GC_BITMAP_H
