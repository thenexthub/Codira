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
#ifndef LIBPANDABASE_UTILS_BLOOM_FILTER_H
#define LIBPANDABASE_UTILS_BLOOM_FILTER_H

#include <cmath>
#include <string>
#include <vector>

#include "libarkbase/utils/bit_vector.h"
#include "libarkbase/utils/hash.h"

namespace ark {

using HashPair = std::pair<uint32_t, uint32_t>;

/*
  BloomFilter

  1. Basic Principle:
     - Uses a bit array bits[0..m-1]
     - k hash functions h1, h2, ..., hk
     - Inserting an element 'key': set bits[h_i(key) % m] = 1 for i = 0..k-1
     - Querying 'key':
       - If all bits[h_i(key) % m] are 1 -> key may exist (possible false positive)
       - If any bit is 0 -> key definitely does NOT exist (no false negatives)

  2. Double Hashing Trick:
     - h_i(key) = (h1(key) + i * h2(key)) % m, for i = 0..k-1
     - Only two hash computations are needed to generate k positions
     - Make sure h2 != 0, otherwise all positions will be identical

  3. BloomFilter Parameter Calculation:
     - n = expected number of elements
     - p = desired false positive probability
     - Bit array size: m = - (n * ln(p)) / (ln 2)^2
     - Number of hash functions: k = (m / n) * ln 2
     - Choosing m and k this way minimizes the false positive rate to the target p

  4. Characteristics:
     - No false negatives
     - False positives possible
     - Lookup for "non-existent" elements is extremely fast
     - Lookup for "existing" elements may trigger false positives, requiring verification in original data
*/

class BloomFilter {
public:
    explicit BloomFilter(size_t n, double_t p)
    {
        if (UNLIKELY(n == 0 || p <= 0 || p >= 1)) {
            UNREACHABLE();
        }

        constexpr double_t LOG_BASE2 = 2.0;
        const double_t ln2 = std::log(LOG_BASE2);
        auto m = static_cast<size_t>(std::ceil(-static_cast<double>(n) * std::log(p) / (ln2 * ln2)));
        numHashes_ = static_cast<size_t>(std::round(static_cast<double>(m) / n * ln2));

        bits_.resize(m);
    }

    void Add(const uint8_t *key)
    {
        HashPair hashes = CalHashPair(key);

        // use `double hashing trick` to quickly obtain K hashes.
        for (size_t i = 0; i < numHashes_; ++i) {
            size_t pos = CalPos(hashes, i);
            bits_[pos] = true;
        }
    }

    bool PossiblyContains(const uint8_t *key) const
    {
        HashPair hashes = CalHashPair(key);

        for (size_t i = 0; i < numHashes_; ++i) {
            size_t pos = CalPos(hashes, i);
            if (!bits_[pos]) {
                return false;  // must not exist
            }
        }

        return true;  // may exist
    }

    ~BloomFilter() = default;

    DEFAULT_COPY_SEMANTIC(BloomFilter);
    DEFAULT_MOVE_SEMANTIC(BloomFilter);

private:
    static constexpr uint32_t SEED2 = 0x9747b28cU;
    using Hash2 = MurmurHash32<SEED2>;

    static constexpr uint32_t DEFAULT_NON_ZERO_HASH2 = 0x5bd1e995U;

    uint32_t GetHash32String2(const uint8_t *mutf8String) const
    {
        return Hash2::GetHash32String(mutf8String);
    }

    // use two different seeds to calculate two hashes of key
    HashPair CalHashPair(const uint8_t *key) const
    {
        uint32_t hash1 = GetHash32String(key);
        uint32_t hash2 = GetHash32String2(key);
        if (UNLIKELY(hash2 == 0)) {
            hash2 = DEFAULT_NON_ZERO_HASH2;
        }

        return std::make_pair(hash1, hash2);
    }

    size_t CalPos(HashPair hashes, size_t i) const
    {
        uint32_t hash1 = hashes.first;
        uint32_t hash2 = hashes.second;
        size_t pos = (hash1 + i * hash2) % bits_.size();
        return pos;
    }

    BitVector<> bits_;  // size of bits: the m of BloomFilter
    size_t numHashes_;  // numHashes_: the k of BloomFilter
};
}  // namespace ark

#endif  // LIBPANDABASE_UTILS_BLOOM_FILTER_H
