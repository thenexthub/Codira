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

#ifndef PANDA_LIBPANDABASE_UTILS_EXPECTED_H_
#define PANDA_LIBPANDABASE_UTILS_EXPECTED_H_

#include <type_traits>
#include <variant>

#include "libarkbase/macros.h"

namespace ark {

template <class E>
class Unexpected final {
public:
    explicit Unexpected(E e) noexcept(std::is_nothrow_move_constructible_v<E>) : e_(std::move(e)) {}

    const E &Value() const &noexcept
    {
        return e_;
    }
    E &Value() &noexcept
    {
        return e_;
    }
    E &&Value() &&noexcept
    {
        return std::move(e_);
    }

    ~Unexpected() = default;

    DEFAULT_COPY_SEMANTIC(Unexpected);
    NO_MOVE_SEMANTIC(Unexpected);

private:
    E e_;
};

class ExpectedConfig final {
public:
#ifdef NDEBUG
    static constexpr bool RELEASE = true;
#else
    static constexpr bool RELEASE = false;
#endif
};

// Simplified implementation of proposed std::expected
// Note that as with std::expected, program that tries to instantiate
// Expected with T or E for a reference type is ill-formed.
template <class T, class E>
class Expected final {
public:
    template <typename U = T, typename = std::enable_if_t<std::is_default_constructible_v<U>>>
    Expected() noexcept : v_(T())
    {
    }
    // The following constructors are non-explicit to be aligned with std::expected
    // NOLINTNEXTLINE(google-explicit-constructor)
    Expected(T v) noexcept(std::is_nothrow_move_constructible_v<T>) : v_(std::in_place_index<VALUE_INDEX>, std::move(v))
    {
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    Expected(Unexpected<E> e) noexcept(std::is_nothrow_move_constructible_v<E>)
        : v_(std::in_place_index<ERROR_INDEX>, std::move(e.Value()))
    {
    }

    bool HasValue() const noexcept
    {
        return v_.index() == VALUE_INDEX;
    }
    explicit operator bool() const noexcept
    {
        return HasValue();
    }

    const E &Error() const &noexcept(ExpectedConfig::RELEASE)
    {
        ASSERT(!HasValue());
        return std::get<ERROR_INDEX>(v_);
    }
    // NOLINTNEXTLINE(bugprone-exception-escape)
    E &Error() &noexcept(ExpectedConfig::RELEASE)
    {
        ASSERT(!HasValue());
        return std::get<ERROR_INDEX>(v_);
    }
    E &&Error() &&noexcept(ExpectedConfig::RELEASE)
    {
        ASSERT(!HasValue());
        return std::move(std::get<ERROR_INDEX>(v_));
    }

    const T &Value() const &noexcept(ExpectedConfig::RELEASE)
    {
        ASSERT(HasValue());
        return std::get<VALUE_INDEX>(v_);
    }
    // NOTE(aemelenko): Delete next line when the issue 388 is resolved
    // NOLINTNEXTLINE(bugprone-exception-escape)
    T &Value() &noexcept(ExpectedConfig::RELEASE)
    {
        ASSERT(HasValue());
        return std::get<VALUE_INDEX>(v_);
    }
    T &&Value() &&noexcept(ExpectedConfig::RELEASE)
    {
        ASSERT(HasValue());
        return std::move(std::get<VALUE_INDEX>(v_));
    }
    const T &operator*() const &noexcept(ExpectedConfig::RELEASE)
    {
        return Value();
    }
    // NOLINTNEXTLINE(bugprone-exception-escape)
    T &operator*() &noexcept(ExpectedConfig::RELEASE)
    {
        return Value();
    }
    T &&operator*() &&noexcept(ExpectedConfig::RELEASE)
    {
        return std::move(*this).Value();
    }

    const T *operator->() const noexcept(ExpectedConfig::RELEASE)
    {
        auto ptr = std::get_if<VALUE_INDEX>(&v_);
        ASSERT(ptr != nullptr);
        return ptr;
    }
    T *operator->() noexcept(ExpectedConfig::RELEASE)
    {
        auto ptr = std::get_if<VALUE_INDEX>(&v_);
        ASSERT(ptr != nullptr);
        return ptr;
    }

    template <class U = T>
    T ValueOr(U &&v) const &
    {
        if (HasValue()) {
            return Value();
        }
        return std::forward<U>(v);
    }
    template <class U = T>
    T ValueOr(U &&v) &&
    {
        if (HasValue()) {
            return Value();
        }
        return std::forward<U>(v);
    }

    ~Expected() = default;

    DEFAULT_COPY_SEMANTIC(Expected);
    DEFAULT_MOVE_SEMANTIC(Expected);

private:
    static constexpr size_t VALUE_INDEX = 0;
    static constexpr size_t ERROR_INDEX = 1;
    std::variant<T, E> v_;
};

}  // namespace ark

#endif  // PANDA_LIBPANDABASE_UTILS_EXPECTED_H_
