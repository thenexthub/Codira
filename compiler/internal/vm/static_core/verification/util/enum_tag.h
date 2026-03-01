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

#ifndef PANDA_VERIFIER_UTIL_TAG_FOR_ENUM_
#define PANDA_VERIFIER_UTIL_TAG_FOR_ENUM_

#include <cstddef>

#include "libarkbase/utils/bit_utils.h"

#include "libarkbase/macros.h"

namespace ark::verifier {

template <size_t I_NUM, typename Enum, Enum... ITEMS>
class TagForEnumNumerated {
protected:
    static constexpr size_t SIZE = 0ULL;

    static size_t GetIndexFor([[maybe_unused]] Enum /* unused */)
    {
        UNREACHABLE();
    }

    static Enum GetValueFor([[maybe_unused]] size_t /* unused */)
    {
        UNREACHABLE();
    }
};

// TagForEnumNumerated needs because recursive numeration with Size - 1 gives wrong ordering
template <size_t I_NUM, typename Enum, Enum I, Enum... ITEMS>
class TagForEnumNumerated<I_NUM, Enum, I, ITEMS...> : public TagForEnumNumerated<I_NUM + 1ULL, Enum, ITEMS...> {
    using Base = TagForEnumNumerated<I_NUM + 1ULL, Enum, ITEMS...>;

protected:
    static constexpr size_t SIZE = Base::SIZE + 1ULL;

    static size_t GetIndexFor(Enum e)
    {
        if (e == I) {
            return I_NUM;
        }
        return Base::GetIndexFor(e);
    }

    static Enum GetValueFor(size_t tag)
    {
        if (tag == I_NUM) {
            return I;
        }
        return Base::GetValueFor(tag);
    }
};

template <typename Enum, Enum... ITEMS>
class TagForEnum : public TagForEnumNumerated<0ULL, Enum, ITEMS...> {
    using Base = TagForEnumNumerated<0ULL, Enum, ITEMS...>;

public:
    using Type = Enum;

    static constexpr size_t SIZE = Base::SIZE;
    static constexpr size_t BITS = sizeof(size_t) * 8ULL - ark::Clz(SIZE);

    static size_t GetIndexFor(Enum e)
    {
        return Base::GetIndexFor(e);
    }

    static Enum GetValueFor(size_t tag)
    {
        return Base::GetValueFor(tag);
    }
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_UTIL_TAG_FOR_ENUM_