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

#ifndef PANDA_VERIFIER_UTIL_OPTIONAL_REF_H
#define PANDA_VERIFIER_UTIL_OPTIONAL_REF_H

#include "libarkbase/macros.h"

#include <functional>
// included for nullopt_t
#include <optional>

namespace ark::verifier {

template <typename T>
struct OptionalRef;

template <typename T>
struct OptionalConstRef {
public:
    OptionalConstRef() : value_ {nullptr} {}
    // These are intentionally implicit for consistency with std::optional
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalConstRef(std::nullopt_t /* unused */) : value_ {nullptr} {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalConstRef(const T &reference) : value_ {const_cast<T *>(&reference)} {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalConstRef(std::reference_wrapper<const T> reference) : OptionalConstRef(reference.get()) {}
    explicit OptionalConstRef(const T *ptr) : value_ {const_cast<T *>(ptr)} {}
    ~OptionalConstRef() = default;

    bool HasRef() const
    {
        return value_ != nullptr;
    }

    const T &Get() const
    {
        ASSERT(HasRef());
        return *value_;
    }

    const T &operator*() const
    {
        return Get();
    }

    const T *AsPointer() const
    {
        return &Get();
    }

    const T *operator->() const
    {
        return AsPointer();
    }

    std::reference_wrapper<const T> AsWrapper() const
    {
        return Get();
    }

    /// @brief The unsafe direction of const_cast, though without UB. Try to avoid using.
    OptionalRef<T> Unconst() const
    {
        return OptionalRef<T> {value_};
    }

    DEFAULT_COPY_SEMANTIC(OptionalConstRef);
    DEFAULT_MOVE_SEMANTIC(OptionalConstRef);

protected:
    explicit OptionalConstRef(T *ptr) : value_ {ptr} {}
    // not const to let OptionalRef inherit this without unsafe behavior
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    T *value_;
};

/**
 * @brief Morally equivalent to std::optional<std::reference_wrapper<T>>, but nicer to use and takes less memory
 *
 * @tparam T The referenced type
 */
template <typename T>
struct OptionalRef : public OptionalConstRef<T> {
public:
    using OptionalConstRef<T>::OptionalConstRef;
    using OptionalConstRef<T>::Get;
    using OptionalConstRef<T>::AsPointer;
    using OptionalConstRef<T>::AsWrapper;
    using OptionalConstRef<T>::operator->;
    using OptionalConstRef<T>::operator*;
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalRef(T &reference) : OptionalConstRef<T>(&reference) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    OptionalRef(std::reference_wrapper<T> reference) : OptionalRef(reference.get()) {}
    ~OptionalRef() = default;

    T &Get()
    {
        ASSERT(OptionalConstRef<T>::HasRef());
        return *OptionalConstRef<T>::value_;  // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
    }

    T &operator*()
    {
        return Get();
    }

    T *AsPointer()
    {
        return &Get();
    }

    T *operator->()
    {
        return AsPointer();
    }

    std::reference_wrapper<T> AsWrapper()
    {
        return Get();
    }

    DEFAULT_COPY_SEMANTIC(OptionalRef);
    DEFAULT_MOVE_SEMANTIC(OptionalRef);

private:
    // makes the inherited protected constructor public
    friend struct OptionalConstRef<T>;
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_UTIL_OPTIONAL_REF_H
