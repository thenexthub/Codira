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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_DTOA_HELPER
#define PANDA_PLUGINS_ETS_RUNTIME_DTOA_HELPER

#include <cstdint>
#include <cstddef>
#include <array>
#include "libarkbase/globals.h"
#include "libarkbase/macros.h"
#include "include/coretypes/tagged_value.h"

// Almost copy of ets_runtime/ecmascript/base/dtoa_helper.h

namespace ark::ets::intrinsics::helpers {
class DtoaHelper {
public:
    explicit DtoaHelper(char *buffer) : buffer_(buffer) {}
    void Dtoa(double value);

    int GetPoint()
    {
        return point_;
    }

    int GetDigits()
    {
        return length_;
    }

private:
    static constexpr int CACHED_POWERS_OFFSET = 348;
    static constexpr double D_1_LOG2_10 = 0x1.34413509f79ffp-2;  // 1 / lg(10)
    static constexpr int Q = -61;
    static constexpr int INDEX = 20;
    static constexpr int MIN_DECIMAL_EXPONENT = -348;
    static constexpr auto POW10 = std::array {1ULL,
                                              10ULL,
                                              100ULL,
                                              1000ULL,
                                              10000ULL,
                                              100000ULL,
                                              1000000ULL,
                                              10000000ULL,
                                              100000000ULL,
                                              1000000000ULL,
                                              10000000000ULL,
                                              100000000000ULL,
                                              1000000000000ULL,
                                              10000000000000ULL,
                                              100000000000000ULL,
                                              1000000000000000ULL,
                                              10000000000000000ULL,
                                              100000000000000000ULL,
                                              1000000000000000000ULL,
                                              10000000000000000000ULL};

    static constexpr uint32_t TEN = 10;
    static constexpr uint32_t TEN2POW = 100;
    static constexpr uint32_t TEN3POW = 1000;
    static constexpr uint32_t TEN4POW = 10000;
    static constexpr uint32_t TEN5POW = 100000;
    static constexpr uint32_t TEN6POW = 1000000;
    static constexpr uint32_t TEN7POW = 10000000;
    static constexpr uint32_t TEN8POW = 100000000;

    // DiyFp is a floating-point number type, consists of a uint64 significand and one integer exponent.
    class DiyFp {
    public:
        DiyFp() : f_(), e_() {}
        DiyFp(uint64_t fp, int exp) : f_(fp), e_(exp) {}

        explicit DiyFp(double d)
        {
            union {
                double d;
                uint64_t u64;
            } u = {d};

            int biasedE =
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                static_cast<int>((u.u64 & coretypes::DOUBLE_EXPONENT_MASK) >> coretypes::DOUBLE_SIGNIFICAND_SIZE);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            uint64_t significand = (u.u64 & coretypes::DOUBLE_SIGNIFICAND_MASK);
            if (biasedE != 0) {
                f_ = significand + coretypes::DOUBLE_HIDDEN_BIT;
                e_ = biasedE - DP_EXPONENT_BIAS;
            } else {
                f_ = significand;
                e_ = DP_MIN_EXPONENT + 1;
            }
        }

        DiyFp operator-(const DiyFp &rhs) const
        {
            return DiyFp(f_ - rhs.f_, e_);
        }

        DiyFp operator*(const DiyFp &rhs) const
        {
            constexpr uint64_t M32 = UINT32_MAX;
            const uint64_t a = f_ >> BITS_PER_UINT32;
            const uint64_t b = f_ & M32;
            const uint64_t c = rhs.f_ >> BITS_PER_UINT32;
            const uint64_t d = rhs.f_ & M32;
            const uint64_t ac = a * c;
            const uint64_t bc = b * c;
            const uint64_t ad = a * d;
            const uint64_t bd = b * d;
            uint64_t tmp = (bd >> BITS_PER_UINT32) + (ad & M32) + (bc & M32);
            tmp += 1U << ROUND_BITS;  // mult_round
            return DiyFp(ac + (ad >> BITS_PER_UINT32) + (bc >> BITS_PER_UINT32) + (tmp >> BITS_PER_UINT32),
                         e_ + rhs.e_ + BITS_PER_UINT64);
        }

        DiyFp Normalize() const
        {
            DiyFp res = *this;
            while ((res.f_ & coretypes::DOUBLE_HIDDEN_BIT) == 0) {
                res.f_ <<= 1U;
                res.e_--;
            }
            res.f_ <<= (DIY_SIGNIFICAND_SIZE - coretypes::DOUBLE_SIGNIFICAND_SIZE - 1);
            res.e_ = res.e_ - (DIY_SIGNIFICAND_SIZE - coretypes::DOUBLE_SIGNIFICAND_SIZE - 1);
            return res;
        }

        DiyFp NormalizeBoundary() const
        {
            DiyFp res = *this;
            while ((res.f_ & (coretypes::DOUBLE_HIDDEN_BIT << 1U)) == 0) {
                res.f_ <<= 1U;
                res.e_--;
            }
            res.f_ <<= (DIY_SIGNIFICAND_SIZE - coretypes::DOUBLE_SIGNIFICAND_SIZE - 2);         // 2: parameter
            res.e_ = res.e_ - (DIY_SIGNIFICAND_SIZE - coretypes::DOUBLE_SIGNIFICAND_SIZE - 2);  // 2: parameter
            return res;
        }

        void NormalizedBoundaries(DiyFp *minus, DiyFp *plus) const
        {
            DiyFp pl = DiyFp((f_ << 1U) + 1, e_ - 1).NormalizeBoundary();
            DiyFp mi = (f_ == coretypes::DOUBLE_HIDDEN_BIT) ? DiyFp((f_ << 2U) - 1, e_ - 2)
                                                            : DiyFp((f_ << 1U) - 1, e_ - 1);  // 2: parameter
            ASSERT(mi.e_ >= pl.e_);
            mi.f_ <<= static_cast<uint32_t>(mi.e_ - pl.e_);
            mi.e_ = pl.e_;
            *plus = pl;
            *minus = mi;
        }

        static constexpr uint32_t ROUND_BITS = BITS_PER_UINT32 - 1;
        static constexpr uint32_t DIY_SIGNIFICAND_SIZE = BITS_PER_UINT64;
        static constexpr int32_t DP_EXPONENT_BIAS =
            coretypes::DOUBLE_EXPONENT_BIAS + coretypes::DOUBLE_SIGNIFICAND_SIZE;
        static constexpr int32_t DP_MAX_EXPONENT = coretypes::DOUBLE_EXPONENT_MAX - DP_EXPONENT_BIAS;
        static constexpr int32_t DP_MIN_EXPONENT = -DP_EXPONENT_BIAS;
        static constexpr int32_t DP_DENORMAL_EXPONENT = -DP_EXPONENT_BIAS + 1;

    private:
        uint64_t f_;
        int e_;
        friend class DtoaHelper;
    };

    DiyFp GetCachedPower(int e);
    static DiyFp GetCachedPowerByIndex(std::size_t index);
    void GrisuRound(uint64_t delta, uint64_t rest, uint64_t tenKappa, uint64_t distance);
    static int CountDecimalDigit32(uint32_t n);
    static uint32_t PopDigit(int kappa, uint32_t &p1);
    void DigitGen(const DiyFp &w, const DiyFp &mp, uint64_t delta);
    void Grisu(double value);

private:
    char *buffer_;
    int length_ {};
    int point_ {};
    int k_ {};
};
}  // namespace ark::ets::intrinsics::helpers
#endif  // PANDA_PLUGINS_ETS_RUNTIME_DTOA_HELPER