/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/bitmap.h"

#include <atomic>

namespace ark::mem {

void Bitmap::CopyTo(Bitmap *dest) const
{
    ASSERT(dest->bitmap_.SizeBytes() == bitmap_.SizeBytes());
    [[maybe_unused]] auto err =
        memmove_s(dest->bitmap_.Data(), dest->bitmap_.SizeBytes(), bitmap_.Data(), bitmap_.SizeBytes());
    ASSERT(err == EOK);
}

void Bitmap::ClearBitsInRange(size_t begin, size_t end)
{
    CheckBitRange(begin, end);
    if (GetWordIdx(end) == GetWordIdx(begin)) {  // [begin, end] in the same word
        ClearRangeWithinWord(begin, end);
        return;
    }

    auto beginRoundup = RoundUp(begin, BITSPERWORD);
    auto fnRounddown = [](BitmapWordType val) -> BitmapWordType {
        constexpr BitmapWordType MASK = ~((static_cast<BitmapWordType>(1) << LOG_BITSPERWORD) - 1);
        return val & MASK;
    };
    auto endRounddown = fnRounddown(end);
    ClearRangeWithinWord(begin, beginRoundup);
    ClearWords(GetWordIdx(beginRoundup), GetWordIdx(endRounddown));
    ClearRangeWithinWord(endRounddown, end);
}

bool Bitmap::AtomicTestAndSetBit(size_t bitOffset)
{
    CheckBitOffset(bitOffset);
    auto wordIdx = GetWordIdx(bitOffset);
    auto *wordAddr = reinterpret_cast<std::atomic<BitmapWordType> *>(&bitmap_[wordIdx]);
    auto mask = GetBitMask(bitOffset);
    BitmapWordType oldWord;
    do {
        // Atomic with acquire order reason: data race with word_addr with dependecies on reads after the load which
        // should become visible
        oldWord = wordAddr->load(std::memory_order_acquire);
        if ((oldWord & mask) != 0) {
            return true;
        }
    } while (!wordAddr->compare_exchange_weak(oldWord, oldWord | mask, std::memory_order_seq_cst));
    return false;
}

bool Bitmap::AtomicTestAndClearBit(size_t bitOffset)
{
    CheckBitOffset(bitOffset);
    auto wordIdx = GetWordIdx(bitOffset);
    auto *wordAddr = reinterpret_cast<std::atomic<BitmapWordType> *>(&bitmap_[wordIdx]);
    auto mask = GetBitMask(bitOffset);
    BitmapWordType oldWord;
    do {
        // Atomic with acquire order reason: data race with word_addr with dependecies on reads after the load which
        // should become visible
        oldWord = wordAddr->load(std::memory_order_acquire);
        if ((oldWord & mask) == 0) {
            return false;
        }
    } while (!wordAddr->compare_exchange_weak(oldWord, oldWord & (~mask), std::memory_order_seq_cst));
    return true;
}

bool Bitmap::AtomicTestBit(size_t bitOffset)
{
    CheckBitOffset(bitOffset);
    auto wordIdx = GetWordIdx(bitOffset);
    auto *wordAddr = reinterpret_cast<std::atomic<BitmapWordType> *>(&bitmap_[wordIdx]);
    auto mask = GetBitMask(bitOffset);
    // Atomic with acquire order reason: data race with word_addr with dependecies on reads after the load which should
    // become visible
    BitmapWordType word = wordAddr->load(std::memory_order_acquire);
    return (word & mask) != 0;
}

}  // namespace ark::mem
