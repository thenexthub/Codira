//===- OptionalEnum.h - Space-efficient variant for enum values -*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_BASIC_OPTIONALENUM_H
#define SWIFT_BASIC_OPTIONALENUM_H

#include "language/Basic/type_traits.h"
#include <type_traits>

namespace language {
  template<typename T>
  class OptionalEnum {
    using underlying_type = typename std::underlying_type<T>::type;
    static_assert(IsTriviallyCopyable<T>::value, "type is not trivial");
    static_assert(std::is_integral<underlying_type>::value,
                  "underlying type is not integral");
  public:
    using value_type = T;
    using storage_type = typename std::make_unsigned<underlying_type>::type;

  private:
    storage_type Storage;

  public:
    /// Construct an empty instance.
    OptionalEnum() : Storage(0) { }

    /// Construct an empty instance.
    /*implicit*/ OptionalEnum(std::nullopt_t) : OptionalEnum() {}

    /// Construct an instance containing a value of type \c T constructed with
    /// the given argument.
    template<typename U>
    /*implicit*/ OptionalEnum(
        U &&value,
        typename std::enable_if<std::is_convertible<U, T>::value>::type * = {})
      : Storage(static_cast<storage_type>(T{std::forward<U>(value)}) + 1) {
      assert(hasValue() && "cannot store this value");
    }

    /// Construct the enum from its raw integral representation.
    ///
    /// This can be used to interoperate with PointerIntPair.
    template<typename I>
    explicit OptionalEnum(
        I rawValue,
        typename std::enable_if<std::is_integral<I>::value>::type * = {})
      : Storage(rawValue) {
      assert((uintptr_t)rawValue == (uintptr_t)(intptr_t)*this);
    }

    void reset() {
      Storage = 0;
    }

    bool hasValue() const { return Storage != 0; }
    explicit operator bool() const { return hasValue(); }

    T getValue() const {
      assert(hasValue());
      return static_cast<value_type>(Storage - 1);
    }
    T operator*() { return getValue(); }

    template <typename U>
    constexpr T getValueOr(U &&value) const {
      return hasValue() ? getValue() : value;
    }

    /// Converts the enum to its raw storage value, for interoperation with
    /// PointerIntPair.
    explicit operator intptr_t() const {
      assert(Storage == (storage_type)(intptr_t)Storage);
      return (intptr_t)Storage;
    }

    // Provide comparison operators for the optional value.

    friend bool operator==(const OptionalEnum &lhs, const OptionalEnum &rhs) {
      return (!lhs && !rhs) || (lhs && rhs && lhs.getValue() == rhs.getValue());
    }

    friend bool operator!=(const OptionalEnum &lhs, const OptionalEnum &rhs) {
      return !(lhs == rhs);
    }

    friend bool operator<(const OptionalEnum &lhs, const OptionalEnum &rhs) {
      return rhs && (!lhs || lhs.getValue() < rhs.getValue());
    }

    friend bool operator>(const OptionalEnum &lhs, const OptionalEnum &rhs) {
      return rhs < lhs;
    }

    friend bool operator<=(const OptionalEnum &lhs, const OptionalEnum &rhs) {
      return !(rhs < lhs);
    }

    friend bool operator>=(const OptionalEnum &lhs, const OptionalEnum &rhs) {
      return !(lhs < rhs);
    }
  };
} // end namespace language
  
#endif // SWIFT_BASIC_OPTIONALENUM_H
