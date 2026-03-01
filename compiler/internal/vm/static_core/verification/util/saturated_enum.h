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

#ifndef PANDA_VERIFIER_UTIL_SATURATED_ENUM_H_
#define PANDA_VERIFIER_UTIL_SATURATED_ENUM_H_

#include "libarkbase/macros.h"

#include <utility>

namespace ark::verifier {

/*
 NOTE: default value?
 possible options:
  1. initial value UNDEFINED and any op will lead to fatal error, add constructors etc for proper initialization
    pros: more safety and robustness to programmer errors, cons: more code, more complexity, etc
  2. initial value is least significant one.
    pros: simplicity, cons: less robust, does not help to detect logic errors in program
*/

template <typename Enum, Enum...>
class SaturatedEnum;

template <typename Enum, Enum E>
class SaturatedEnum<Enum, E> {
public:
    SaturatedEnum &operator=(Enum e)
    {
        value_ = e;
        return *this;
    }

    SaturatedEnum &operator|=(Enum e)
    {
        Set(e);
        return *this;
    }

    bool operator[](Enum e) const
    {
        return Check(e, false);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Enum() const
    {
        return value_;
    }

    template <typename Handler>
    void EnumerateValues(Handler &&handler) const
    {
        Enumerate(std::forward<Handler>(handler), false);
    }

protected:
#ifndef NDEBUG
    bool Check(Enum e, bool prevSet) const
#else
    bool Check([[maybe_unused]] Enum e, bool prevSet) const
#endif
    {
        // to catch missed enum members
        ASSERT(e == E);
        return prevSet || value_ == E;
    }

    void Set(Enum e)
    {
        ASSERT(e == E);
        value_ = e;
    }

    template <typename Handler>
    void Enumerate(Handler &&handler, bool prevSet) const
    {
        prevSet = prevSet || (value_ == E);
        if (prevSet) {
            handler(E);
        }
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Enum value_ {E};
};

template <typename Enum, Enum E, Enum... REST>
class SaturatedEnum<Enum, E, REST...> : public SaturatedEnum<Enum, REST...> {
    using Base = SaturatedEnum<Enum, REST...>;

public:
    SaturatedEnum &operator=(Enum e)
    {
        Base::operator=(e);
        return *this;
    }

    SaturatedEnum &operator|=(Enum e)
    {
        Set(e);
        return *this;
    }

    bool operator[](Enum e) const
    {
        return Check(e, false);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Enum() const
    {
        return Base::value_;
    }

    template <typename Handler>
    void EnumerateValues(Handler &&handler) const
    {
        Enumerate(std::forward<Handler>(handler), false);
    }

protected:
    bool Check(Enum e, bool prevSet) const
    {
        prevSet = prevSet || (Base::value_ == E);
        if (e == E) {
            return prevSet;
        }
        return Base::Check(e, prevSet);
    }

    void Set(Enum e)
    {
        if (Base::value_ == E) {
            return;
        }
        if (e == E) {
            Base::operator=(e);
            return;
        }
        Base::Set(e);
    }

    template <typename Handler>
    void Enumerate(Handler &&handler, bool prevSet) const
    {
        prevSet = prevSet || (Base::value_ == E);
        if (prevSet && !handler(E)) {
            return;
        }
        Base::template Enumerate<Handler>(std::forward<Handler>(handler), prevSet);
    }
};

}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_UTIL_SATURATED_ENUM_H_
