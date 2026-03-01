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

#ifndef COMPILER_OPTIMIZER_CODEGEN_FAST_DIVISOR_H
#define COMPILER_OPTIMIZER_CODEGEN_FAST_DIVISOR_H

#include <cstddef>
#include <cstdint>

#include "compiler/optimizer/code_generator/type_info.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/math_helpers.h"

namespace ark::compiler {
/*
 * These helper classes calculate special values (i.e. magic, shift) for fast division/modulo by (u)int32/(u)int64
 * constant. Then user replaces heavy div/mod operations with much lighter mul/shift/add/sub. The algorithms are
 * described in "Hacker's Delight" book by Henry S. Warren (Chapter 10: Integer Division By Constants).
 */

class FastConstSignedDivisor {
public:
    FastConstSignedDivisor(int64_t divisor, size_t bitWidth)
    {
        static_assert(sizeof(uint64_t) == DOUBLE_WORD_SIZE_BYTES);
        ASSERT(divisor <= -2L || divisor >= 2L);
        ASSERT(bitWidth == WORD_SIZE || bitWidth == DOUBLE_WORD_SIZE);
        // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
        uint64_t highOne = static_cast<uint64_t>(1U) << (bitWidth - 1U);
        auto ad = bit_cast<uint64_t>(helpers::math::AbsOrMin(divisor));
        uint64_t t = highOne + (bit_cast<uint64_t>(divisor) >> (DOUBLE_WORD_SIZE - 1U));
        uint64_t anc = t - 1U - t % ad;
        auto p = static_cast<int64_t>(bitWidth - 1U);
        uint64_t q1 = highOne / anc;
        uint64_t r1 = highOne - q1 * anc;
        uint64_t q2 = highOne / ad;
        uint64_t r2 = highOne - q2 * ad;
        uint64_t delta = 0U;

        do {
            ++p;
            q1 *= 2U;
            r1 *= 2U;
            if (r1 >= anc) {
                ++q1;
                r1 -= anc;
            }
            q2 *= 2U;
            r2 *= 2U;
            if (r2 >= ad) {
                ++q2;
                r2 -= ad;
            }
            delta = ad - r2;
        } while (q1 < delta || (q1 == delta && r1 == 0));

        magic_ = static_cast<int64_t>(q2) + 1U;
        if (divisor < 0) {
            magic_ = -magic_;
        }
        if (bitWidth == WORD_SIZE) {
            magic_ = static_cast<int64_t>(down_cast<int32_t>(magic_));
        }
        shift_ = p - static_cast<int64_t>(bitWidth);
    }

    int64_t GetMagic() const
    {
        return magic_;
    }

    int64_t GetShift() const
    {
        return shift_;
    }

private:
    int64_t magic_ {0L};
    int64_t shift_ {0L};
};

class FastConstUnsignedDivisor {
public:
    FastConstUnsignedDivisor(uint64_t divisor, size_t bitWidth)
    {
        static_assert(sizeof(uint64_t) == DOUBLE_WORD_SIZE_BYTES);
        ASSERT(divisor != 0U);
        ASSERT(bitWidth >= 1U);
        ASSERT(bitWidth <= DOUBLE_WORD_SIZE);
        uint64_t sizeMinusOne = static_cast<uint64_t>(bitWidth) - 1U;
        // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
        uint64_t highOne = static_cast<uint64_t>(1U) << sizeMinusOne;
        uint64_t nc = ((highOne - 1U) | highOne) - (-divisor % divisor);
        auto p = static_cast<int64_t>(sizeMinusOne);
        uint64_t q1 = highOne / nc;
        uint64_t r1 = highOne - q1 * nc;
        uint64_t q2 = (highOne - 1U) / divisor;
        uint64_t r2 = (highOne - 1U) - q2 * divisor;

        uint64_t delta = 0U;
        do {
            ++p;
            if (r1 >= nc - r1) {
                q1 = 2U * q1 + 1U;
                r1 = 2U * r1 - nc;
            } else {
                q1 *= 2U;
                r1 *= 2U;
            }

            if (r2 + 1U >= divisor - r2) {
                if (q2 >= (highOne - 1U)) {
                    add_ = true;
                }
                q2 = 2U * q2 + 1U;
                r2 = 2U * r2 + 1U - divisor;
            } else {
                if (q2 >= highOne) {
                    add_ = true;
                }
                q2 *= 2U;
                r2 = 2U * r2 + 1;
            }

            delta = divisor - 1U - r2;
        } while ((p < 2L * static_cast<int64_t>(bitWidth)) && (q1 < delta || (q1 == delta && r1 == 0U)));

        magic_ = q2 + 1U;
        if (bitWidth == WORD_SIZE) {
            magic_ &= std::numeric_limits<uint32_t>::max();
        }
        shift_ = static_cast<uint64_t>(p - static_cast<int64_t>(bitWidth));
    }

    uint64_t GetMagic() const
    {
        return magic_;
    }

    uint64_t GetShift() const
    {
        return shift_;
    }

    bool GetAdd() const
    {
        return add_;
    }

private:
    uint64_t magic_ {0U};
    uint64_t shift_ {0U};
    bool add_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_FAST_DIVISOR_H
