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

#include "dtoa_helper.h"
#include "libarkbase/macros.h"
#include "libarkbase/globals.h"

// Almost copy of ets_runtime/ecmascript/base/dtoa_helper.cpp
namespace ark::ets::intrinsics::helpers {
#ifndef UINT64_C2
static constexpr uint64_t ConstructLong(uint32_t high32, uint32_t low32)
{
    return ((static_cast<uint64_t>(high32) << BITS_PER_UINT32) | static_cast<uint64_t>(low32));
}
#define UINT64_C2 ConstructLong
#endif

DtoaHelper::DiyFp DtoaHelper::GetCachedPower(int e)
{
    // dk must be positive, so can do ceiling in positive
    double dk = (Q - e) * D_1_LOG2_10 + CACHED_POWERS_OFFSET - 1;
    int k = static_cast<int>(dk);
    if (dk - k > 0.0) {
        k++;
    }
    auto index = (static_cast<uint32_t>(k) >> 3U) + 1;             // 3: parameter
    k_ = -(MIN_DECIMAL_EXPONENT + static_cast<int>(index << 3U));  // 3: parameter
    return GetCachedPowerByIndex(index);
}

DtoaHelper::DiyFp DtoaHelper::GetCachedPowerByIndex(std::size_t index)
{
    // 10^-348, 10^-340, ..., 10^340
    static constexpr auto CACHED_POWERS_F = std::array {
        UINT64_C2(0xfa8fd5a0, 0x081c0288),  UINT64_C2(0xbaaee17f, 0xa23ebf76), UINT64_C2(0x8b16fb20, 0x3055ac76),
        UINT64_C2(0xcf42894a, 0x5dce35ea),  UINT64_C2(0x9a6bb0aa, 0x55653b2d), UINT64_C2(0xe61acf03, 0x3d1a45df),
        UINT64_C2(0xab70fe17, 0xc79ac6ca),  UINT64_C2(0xff77b1fc, 0xbebcdc4f), UINT64_C2(0xbe5691ef, 0x416bd60c),
        UINT64_C2(0x8dd01fad, 0x907ffc3c),  UINT64_C2(0xd3515c28, 0x31559a83), UINT64_C2(0x9d71ac8f, 0xada6c9b5),
        UINT64_C2(0xea9c2277, 0x23ee8bcb),  UINT64_C2(0xaecc4991, 0x4078536d), UINT64_C2(0x823c1279, 0x5db6ce57),
        UINT64_C2(0xc2109436, 0x4dfb5637),  UINT64_C2(0x9096ea6f, 0x3848984f), UINT64_C2(0xd77485cb, 0x25823ac7),
        UINT64_C2(0xa086cfcd, 0x97bf97f4),  UINT64_C2(0xef340a98, 0x172aace5), UINT64_C2(0xb23867fb, 0x2a35b28e),
        UINT64_C2(0x84c8d4df, 0xd2c63f3b),  UINT64_C2(0xc5dd4427, 0x1ad3cdba), UINT64_C2(0x936b9fce, 0xbb25c996),
        UINT64_C2(0xdbac6c24, 0x7d62a584),  UINT64_C2(0xa3ab6658, 0x0d5fdaf6), UINT64_C2(0xf3e2f893, 0xdec3f126),
        UINT64_C2(0xb5b5ada8, 0xaaff80b8),  UINT64_C2(0x87625f05, 0x6c7c4a8b), UINT64_C2(0xc9bcff60, 0x34c13053),
        UINT64_C2(0x964e858c, 0x91ba2655),  UINT64_C2(0xdff97724, 0x70297ebd), UINT64_C2(0xa6dfbd9f, 0xb8e5b88f),
        UINT64_C2(0xf8a95fcf, 0x88747d94),  UINT64_C2(0xb9447093, 0x8fa89bcf), UINT64_C2(0x8a08f0f8, 0xbf0f156b),
        UINT64_C2(0xcdb02555, 0x653131b6),  UINT64_C2(0x993fe2c6, 0xd07b7fac), UINT64_C2(0xe45c10c4, 0x2a2b3b06),
        UINT64_C2(0xaa242499, 0x697392d3),  UINT64_C2(0xfd87b5f2, 0x8300ca0e), UINT64_C2(0xbce50864, 0x92111aeb),
        UINT64_C2(0x8cbccc09, 0x6f5088cc),  UINT64_C2(0xd1b71758, 0xe219652c), UINT64_C2(0x9c400000, 0x00000000),
        UINT64_C2(0xe8d4a510, 0x00000000),  UINT64_C2(0xad78ebc5, 0xac620000), UINT64_C2(0x813f3978, 0xf8940984),
        UINT64_C2(0xc097ce7b, 0xc90715b3),  UINT64_C2(0x8f7e32ce, 0x7bea5c70), UINT64_C2(0xd5d238a4, 0xabe98068),
        UINT64_C2(0x9f4f2726, 0x179a2245),  UINT64_C2(0xed63a231, 0xd4c4fb27), UINT64_C2(0xb0de6538, 0x8cc8ada8),
        UINT64_C2(0x83c7088e, 0x1a'ab65db), UINT64_C2(0xc45d1df9, 0x42711d9a), UINT64_C2(0x924d692c, 0xa61be758),
        UINT64_C2(0xda01ee64, 0x1a708dea),  UINT64_C2(0xa26da399, 0x9aef774a), UINT64_C2(0xf209787b, 0xb47d6b85),
        UINT64_C2(0xb454e4a1, 0x79dd1877),  UINT64_C2(0x865b8692, 0x5b9bc5c2), UINT64_C2(0xc83553c5, 0xc8965d3d),
        UINT64_C2(0x952ab45c, 0xfa97a0b3),  UINT64_C2(0xde469fbd, 0x99a05fe3), UINT64_C2(0xa59bc234, 0xdb398c25),
        UINT64_C2(0xf6c69a72, 0xa3989f5c),  UINT64_C2(0xb7dcbf53, 0x54e9bece), UINT64_C2(0x88fcf317, 0xf22241e2),
        UINT64_C2(0xcc20ce9b, 0xd35c78a5),  UINT64_C2(0x98165af3, 0x7b2153df), UINT64_C2(0xe2a0b5dc, 0x971f303a),
        UINT64_C2(0xa8d9d153, 0x5ce3b396),  UINT64_C2(0xfb9b7cd9, 0xa4a7443c), UINT64_C2(0xbb764c4c, 0xa7a44410),
        UINT64_C2(0x8bab8eef, 0xb6409c1a),  UINT64_C2(0xd01fef10, 0xa657842c), UINT64_C2(0x9b10a4e5, 0xe9913129),
        UINT64_C2(0xe7109bfb, 0xa19c0c9d),  UINT64_C2(0xac2820d9, 0x623bf429), UINT64_C2(0x80444b5e, 0x7aa7cf85),
        UINT64_C2(0xbf21e440, 0x03acdd2d),  UINT64_C2(0x8e679c2f, 0x5e44ff8f), UINT64_C2(0xd433179d, 0x9c8cb841),
        UINT64_C2(0x9e19db92, 0xb4e31ba9),  UINT64_C2(0xeb96bf6e, 0xbadf77d9), UINT64_C2(0xaf87023b, 0x9bf0ee6b)};
    static constexpr auto CACHED_POWERS_E =
        std::array {-1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007, -980, -954, -927, -901, -874, -847,
                    -821,  -794,  -768,  -741,  -715,  -688,  -661,  -635,  -608,  -582, -555, -529, -502, -475, -449,
                    -422,  -396,  -369,  -343,  -316,  -289,  -263,  -236,  -210,  -183, -157, -130, -103, -77,  -50,
                    -24,   3,     30,    56,    83,    109,   136,   162,   189,   216,  242,  269,  295,  322,  348,
                    375,   402,   428,   455,   481,   508,   534,   561,   588,   614,  641,  667,  694,  720,  747,
                    774,   800,   827,   853,   880,   907,   933,   960,   986,   1013, 1039, 1066};
    return DtoaHelper::DiyFp(CACHED_POWERS_F[index], static_cast<int16_t>(CACHED_POWERS_E[index]));
}

void DtoaHelper::GrisuRound(uint64_t delta, uint64_t rest, uint64_t tenKappa, uint64_t distance)
{
    while (rest < distance && delta - rest >= tenKappa &&
           (rest + tenKappa < distance || distance - rest > rest + tenKappa - distance)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        buffer_[length_ - 1]--;
        rest += tenKappa;
    }
}

constexpr auto MAX_DIGITS = 9;
int DtoaHelper::CountDecimalDigit32(uint32_t n)
{
    ASSERT(n < POW10[MAX_DIGITS]);
    for (int i = 1; i < MAX_DIGITS; i++) {
        if (n < POW10[i]) {
            return i;
        }
    }
    return MAX_DIGITS;
}

uint32_t DtoaHelper::PopDigit(int kappa, uint32_t &p1)
{
    uint32_t d;
    switch (kappa) {
        case MAX_DIGITS:  // 9: number of decimal digits
            d = p1 / TEN8POW;
            p1 %= TEN8POW;
            break;
        case 8:  // 8: number of decimal digits
            d = p1 / TEN7POW;
            p1 %= TEN7POW;
            break;
        case 7:  // 7: number of decimal digits
            d = p1 / TEN6POW;
            p1 %= TEN6POW;
            break;
        case 6:  // 6: number of decimal digits
            d = p1 / TEN5POW;
            p1 %= TEN5POW;
            break;
        case 5:  // 5: number of decimal digits
            d = p1 / TEN4POW;
            p1 %= TEN4POW;
            break;
        case 4:  // 4: number of decimal digits
            d = p1 / TEN3POW;
            p1 %= TEN3POW;
            break;
        case 3:  // 3: number of decimal digits
            d = p1 / TEN2POW;
            p1 %= TEN2POW;
            break;
        case 2:  // 2: number of decimal digits
            d = p1 / TEN;
            p1 %= TEN;
            break;
        case 1:  // 1: number of decimal digits
            d = p1;
            p1 = 0;
            break;
        default:
            UNREACHABLE();
    }
    return d;
}

void DtoaHelper::DigitGen(const DiyFp &w, const DiyFp &mp, uint64_t delta)
{
    ASSERT(mp.e_ <= 0);
    const DiyFp one(uint64_t(1) << static_cast<uint32_t>(-mp.e_), mp.e_);
    const DiyFp distance = mp - w;
    ASSERT(one.e_ <= 0);
    auto p1 = static_cast<uint32_t>(mp.f_ >> static_cast<uint32_t>(-one.e_));
    uint64_t p2 = mp.f_ & (one.f_ - 1);
    int kappa = CountDecimalDigit32(p1);  // kappa in [0, 9]
    length_ = 0;
    while (kappa > 0) {
        uint32_t d = PopDigit(kappa, p1);
        if (d != 0 || length_ != 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            buffer_[length_++] = static_cast<char>('0' + static_cast<char>(d));
        }
        kappa--;
        uint64_t tmp = (static_cast<uint64_t>(p1) << static_cast<uint32_t>(-one.e_)) + p2;
        if (tmp <= delta) {
            k_ += kappa;
            GrisuRound(delta, tmp, POW10[kappa] << static_cast<uint32_t>(-one.e_), distance.f_);
            return;
        }
    }

    // kappa = 0
    do {
        p2 *= TEN;
        delta *= TEN;
        char d = static_cast<char>(p2 >> static_cast<uint32_t>(-one.e_));
        if (d != 0 || length_ != 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            buffer_[length_++] = static_cast<char>('0' + d);
        }
        p2 &= one.f_ - 1;
        kappa--;
    } while (p2 >= delta);
    k_ += kappa;
    int index = -kappa;
    GrisuRound(delta, p2, one.f_, distance.f_ * (index < INDEX ? POW10[index] : 0));
}

// Grisu2  algorithm use the extra capacity of the used integer type to shorten the produced output
void DtoaHelper::Grisu(double value)
{
    const DiyFp v(value);
    DiyFp mMinus;
    DiyFp mPlus;
    v.NormalizedBoundaries(&mMinus, &mPlus);

    const DiyFp cached = GetCachedPower(mPlus.e_);
    const DiyFp w = v.Normalize() * cached;
    DiyFp wPlus = mPlus * cached;
    DiyFp wMinus = mMinus * cached;
    wMinus.f_++;
    wPlus.f_--;
    DigitGen(w, wPlus, wPlus.f_ - wMinus.f_);
}

void DtoaHelper::Dtoa(double value)
{
    // Exceptional case such as NAN, 0.0, negative... are processed in DoubleToEcmaString
    // So use Dtoa should avoid Exceptional case.
    ASSERT(value > 0);
    Grisu(value);
    point_ = length_ + k_;
}
}  // namespace ark::ets::intrinsics::helpers