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

#ifndef PANDA_VERIF_PARSER_CHARSET_H_
#define PANDA_VERIF_PARSER_CHARSET_H_

#include "libarkbase/macros.h"

#include <cstdint>

namespace ark::parser {

template <typename Char>
class Charset {
public:
    constexpr bool operator()(Char chartype) const
    {
        auto c = static_cast<uint8_t>(chartype);
        // NOLINTNEXTLINE(readability-magic-numbers)
        return (bitmap_[(c) >> 0x6U] & (0x1ULL << ((c)&0x3FU))) != 0;
    }

    Charset() = default;
    Charset(const Charset &c) = default;
    Charset(Charset &&c) = default;
    Charset &operator=(const Charset &c) = default;
    Charset &operator=(Charset &&c) = default;
    ~Charset() = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr Charset(Char *s)
    {
        ASSERT(s != nullptr);
        while (*s) {
            auto c = static_cast<uint8_t>(*s);
            // NB!: 1ULL (64bit) required, bug in clang optimizations on -O2 and higher
            // NOLINTNEXTLINE(readability-magic-numbers)
            bitmap_[(c) >> 0x6U] = static_cast<uint64_t>(bitmap_[(c) >> 0x6U] | (0x1ULL << ((c)&0x3FU)));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ++s;
        }
    }

    constexpr Charset operator+(const Charset &c) const
    {
        Charset cs;
        cs.bitmap_[0x0] = bitmap_[0x0] | c.bitmap_[0x0];
        cs.bitmap_[0x1] = bitmap_[0x1] | c.bitmap_[0x1];
        cs.bitmap_[0x2] = bitmap_[0x2] | c.bitmap_[0x2];
        cs.bitmap_[0x3] = bitmap_[0x3] | c.bitmap_[0x3];
        return cs;
    }

    constexpr Charset operator-(const Charset &c) const
    {
        Charset cs;
        cs.bitmap_[0x0] = bitmap_[0x0] & ~c.bitmap_[0x0];
        cs.bitmap_[0x1] = bitmap_[0x1] & ~c.bitmap_[0x1];
        cs.bitmap_[0x2] = bitmap_[0x2] & ~c.bitmap_[0x2];
        cs.bitmap_[0x3] = bitmap_[0x3] & ~c.bitmap_[0x3];
        return cs;
    }

    constexpr Charset operator!() const
    {
        Charset cs;
        cs.bitmap_[0x0] = ~bitmap_[0x0];
        cs.bitmap_[0x1] = ~bitmap_[0x1];
        cs.bitmap_[0x2] = ~bitmap_[0x2];
        cs.bitmap_[0x3] = ~bitmap_[0x3];
        return cs;
    }

private:
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint64_t bitmap_[4] = {
        0x0ULL,
    };
};

}  // namespace ark::parser

#endif  // PANDA_VERIF_PARSER_CHARSET_H_
