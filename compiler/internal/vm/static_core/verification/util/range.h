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

#ifndef PANDA_VERIFIER_UTIL_RANGE_HPP_
#define PANDA_VERIFIER_UTIL_RANGE_HPP_

#include <limits>
#include <string>
#include <algorithm>
#include <iterator>

namespace ark::verifier {

template <typename... T>
class Range;

template <typename Int>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Range<Int> {
public:
    class Iterator {
    public:
        using value_type = Int;                               // NOLINT(readability-identifier-naming)
        using pointer = Int *;                                // NOLINT(readability-identifier-naming)
        using reference = Int &;                              // NOLINT(readability-identifier-naming)
        using difference_type = std::make_signed_t<Int>;      // NOLINT(readability-identifier-naming)
        using iterator_category = std::forward_iterator_tag;  // NOLINT(readability-identifier-naming)

        // NOLINTNEXTLINE(google-explicit-constructor)
        Iterator(const Int val) : val_ {val} {}
        Iterator() = default;
        Iterator(const Iterator &) = default;
        Iterator(Iterator &&) = default;
        Iterator &operator=(const Iterator &) = default;
        Iterator &operator=(Iterator &&) = default;
        ~Iterator() = default;

        // NOLINTNEXTLINE(cert-dcl21-cpp)
        Iterator operator++(int)
        {
            Iterator old {*this};
            ++val_;
            return old;
        }
        Iterator &operator++()
        {
            ++val_;
            return *this;
        }
        // NOLINTNEXTLINE(cert-dcl21-cpp)
        Iterator operator--(int)
        {
            Iterator old {*this};
            --val_;
            return old;
        }
        Iterator &operator--()
        {
            --val_;
            return *this;
        }
        bool operator==(const Iterator &rhs)
        {
            return val_ == rhs.val_;
        }
        bool operator!=(const Iterator &rhs)
        {
            return val_ != rhs.val_;
        }
        Int operator*()
        {
            return val_;
        }

    private:
        Int val_ = std::numeric_limits<Int>::min();
    };
    template <typename Container>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Range(const Container &cont) : from_ {0}, to_ {cont.size() - 1}
    {
    }
    Range(const Int from, const Int to) : from_ {std::min(from, to)}, to_ {std::max(from, to)} {}
    Range() = default;
    ~Range() = default;

    Iterator begin() const  // NOLINT(readability-identifier-naming)
    {
        return {from_};
    }
    Iterator cbegin() const  // NOLINT(readability-identifier-naming)
    {
        return {from_};
    }
    Iterator end() const  // NOLINT(readability-identifier-naming)
    {
        return {to_ + 1};
    }
    Iterator cend() const  // NOLINT(readability-identifier-naming)
    {
        return {to_ + 1};
    }
    Range BasedAt(Int point) const
    {
        return Range {point, point + to_ - from_};
    }
    bool Contains(Int point) const
    {
        return point >= from_ && point <= to_;
    }
    Int PutInBounds(Int point) const
    {
        if (point < from_) {
            return from_;
        }
        if (point > to_) {
            return to_;
        }
        return point;
    }
    size_t Length() const
    {
        return to_ - from_ + 1;
    }
    Int OffsetOf(Int val) const
    {
        return val - from_;
    }
    Int IndexOf(Int offset) const
    {
        return offset + from_;
    }
    Int Start() const
    {
        return from_;
    }
    Int Finish() const
    {
        return to_;
    }
    bool operator==(const Range &rhs) const
    {
        return from_ == rhs.from_ && to_ == rhs.to_;
    }

private:
    Int from_;
    Int to_;
};

template <typename Int>
Range(Int, Int, typename std::enable_if<std::is_integral<Int>::value, bool>::type b = true) -> Range<Int>;
}  // namespace ark::verifier

namespace std {  // NOLINT(cert-dcl58-cpp)
template <typename Int>
string to_string(const ark::verifier::Range<Int> &range)  // NOLINT(readability-identifier-naming)
{
    return string {"[ "} + to_string(range.Start()) + " .. " + to_string(range.Finish()) + " ]";
}
}  // namespace std

#endif  // !PANDA_VERIFIER_UTIL_RANGE_HPP_
