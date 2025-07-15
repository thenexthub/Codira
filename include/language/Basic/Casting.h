//===--- Casting.h - Helpers for casting ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_CASTING_H
#define LANGUAGE_BASIC_CASTING_H

#include <type_traits>

namespace language {

/// Cast between two function types. Use in place of std::bit_cast, which
/// doesn't work on ARM64e with address-discriminated signed function types.
///
/// Address-discriminated ptrauth attributes can only be applied to values with
/// a defined storage location, such as struct fields, since their value is
/// inherently tied to their address. They can't be applied to function
/// paremeters. When passing such a value to a templated function, the ptrauth
/// attribute disappears from the inferred type.
///
/// bit_cast takes the source by reference, which means that the ptrauth
/// attribute remains on the inferred type, and the value is not trivially
/// copyable.
///
/// function_cast instead takes the source by value, avoiding that issue and
/// ensuring that passed-in function pointers are always trivially copyable.
template <typename Destination, typename Source>
Destination function_cast(Source source) {
  static_assert(sizeof(Destination) == sizeof(Source),
                "Source and destination must be the same size");
  static_assert(std::is_trivially_copyable_v<Source>,
                "The source type must be trivially constructible");
  static_assert(std::is_trivially_copyable_v<Destination>,
                "The destination type must be trivially constructible");

#if __has_feature(ptrauth_calls)
  // Use reinterpret_cast here so we perform any necessary auth-and-sign.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
  return reinterpret_cast<Destination>(source);
#pragma clang diagnostic pop
#else
  Destination destination;
  memcpy(&destination, &source, sizeof(source));
  return destination;
#endif
}

}

#endif
