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

#ifndef PANDA_VERIFIER_UTIL_INDEX_HPP_
#define PANDA_VERIFIER_UTIL_INDEX_HPP_

#include "libarkbase/macros.h"
#include "verification/util/hash.h"

#include <limits>

namespace ark::verifier {
// similar to std::optional, but much more effective for numeric types
// when not all range of values is required and some value may be used
// as dedicated invalid value
template <typename Int, const Int INVALID = std::numeric_limits<Int>::max()>
class Index {
public:
    Index() : value_ {INVALID} {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    Index(Int val) : value_ {val}
    {
        ASSERT(IsValid());
    }
    Index &operator=(Int val)
    {
        value_ = val;
        ASSERT(IsValid());
        return *this;
    }
    Index(const Index &) = default;
    Index(Index &&idx) : value_ {idx.value_}
    {
        idx.Invalidate();
    }
    Index &operator=(const Index &) = default;
    Index &operator=(Index &&idx)
    {
        value_ = idx.value_;
        idx.Invalidate();
        return *this;
    }
    ~Index() = default;

    bool operator==(const Index &other) const
    {
        return value_ == other.value_;
    };
    bool operator!=(const Index &other) const
    {
        return !(*this == other);
    };

    void Invalidate()
    {
        value_ = INVALID;
    }

    bool IsValid() const
    {
        return value_ != INVALID;
    }

    // for contextual conversion in if/while/etc.
    explicit operator bool() const
    {
        return IsValid();
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Int() const
    {
        ASSERT(IsValid());
        return value_;
    }

    Int operator*() const
    {
        ASSERT(IsValid());
        return value_;
    }

    template <typename T>
    explicit operator T() const
    {
        ASSERT(IsValid());
        // NOTE: check that T is capable of holding valid range of value
        return static_cast<T>(value_);
    }

private:
    Int value_;
    template <typename T>
    friend struct std::hash;
};
}  // namespace ark::verifier

namespace std {
template <typename Int, const Int I>
struct hash<ark::verifier::Index<Int, I>> {
    size_t operator()(const ark::verifier::Index<Int, I> &i) const noexcept
    {
        return ark::verifier::StdHash(i.value_);
    }
};
}  // namespace std

#endif  // !PANDA_VERIFIER_UTIL_INDEX_HPP_
