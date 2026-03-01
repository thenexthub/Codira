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

#ifndef PANDA_VERIFIER_UTIL_FLAGS_H_
#define PANDA_VERIFIER_UTIL_FLAGS_H_

#include "libarkbase/macros.h"

#include <cstdint>

namespace ark::verifier {
template <typename UInt, typename Enum, Enum...>
class FlagsForEnum;

template <typename UInt, typename Enum, Enum FLAG>
class FlagsForEnum<UInt, Enum, FLAG> {
public:
    class ConstBit {
    public:
        ConstBit(UInt bitMask, const UInt &givenFlags) : mask_ {bitMask}, flags_ {givenFlags} {};
        ConstBit() = delete;
        ~ConstBit() = default;

        NO_COPY_SEMANTIC(ConstBit);
        NO_MOVE_SEMANTIC(ConstBit);

        // NOLINTNEXTLINE(google-explicit-constructor)
        operator bool() const
        {
            return (flags_ & mask_) != 0;
        }

    protected:
        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        const UInt mask_;
        const UInt &flags_;
        // NOLINTEND(misc-non-private-member-variables-in-classes)
    };

    class Bit : public ConstBit {
    public:
        Bit(UInt bitMask, UInt &givenFlags) : ConstBit {bitMask, givenFlags} {};
        ~Bit() = default;
        NO_COPY_SEMANTIC(Bit);
        NO_MOVE_SEMANTIC(Bit);

        Bit &operator=(bool b)
        {
            UInt &properFlags = const_cast<UInt &>(ConstBit::flags_);
            if (b) {
                properFlags |= ConstBit::mask_;
            } else {
                properFlags &= ~ConstBit::mask_;
            }
            return *this;
        }
    };

    template <typename Handler>
    void EnumerateFlags(Handler &&handler) const
    {
        if (ConstBit {MASK, flags_}) {
            handler(FLAG);
        }
    }

#ifndef NDEBUG
    ConstBit operator[](Enum f) const
    {
        ASSERT(f == FLAG);
        return {MASK, flags_};
    }

    Bit operator[](Enum f)
    {
        ASSERT(f == FLAG);
        return {MASK, flags_};
    }
#else
    ConstBit operator[](Enum /* unused */) const
    {
        return {MASK, flags_};
    }

    Bit operator[](Enum /* unused */)
    {
        return {MASK, flags_};
    }
#endif

protected:
    constexpr static UInt MASK = static_cast<UInt>(1);
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    UInt flags_ {0};
};

template <typename UInt, typename Enum, Enum FLAG, Enum... REST>
class FlagsForEnum<UInt, Enum, FLAG, REST...> : public FlagsForEnum<UInt, Enum, REST...> {
    using Base = FlagsForEnum<UInt, Enum, REST...>;

public:
    typename Base::ConstBit operator[](Enum f) const
    {
        if (f == FLAG) {
            return {MASK, Base::flags_};
        }
        return Base::operator[](f);
    }

    typename Base::Bit operator[](Enum f)
    {
        if (f == FLAG) {
            return {MASK, Base::flags_};
        }
        return Base::operator[](f);
    }

    template <typename Handler>
    void EnumerateFlags(Handler &&handler) const
    {
        if (typename Base::ConstBit {MASK, Base::flags_} && !handler(FLAG)) {
            return;
        }
        Base::template EnumerateFlags<Handler>(std::forward<Handler>(handler));
    }

protected:
    constexpr static UInt MASK = Base::MASK << static_cast<UInt>(1);
    static_assert(MASK != 0, "too many flags for UInt size");
};
}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_UTIL_FLAGS_H_
