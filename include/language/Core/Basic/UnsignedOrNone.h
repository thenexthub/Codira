//===- UnsignedOrNone.h - simple optional index-----*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines language::Core::UnsignedOrNone.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_UNSIGNED_OR_NONE_H
#define LANGUAGE_CORE_BASIC_UNSIGNED_OR_NONE_H

#include <cassert>
#include <optional>

namespace language::Core {

struct UnsignedOrNone {
  constexpr UnsignedOrNone(std::nullopt_t) : Rep(0) {}
  UnsignedOrNone(unsigned Val) : Rep(Val + 1) { assert(operator bool()); }
  UnsignedOrNone(int) = delete;

  constexpr static UnsignedOrNone fromInternalRepresentation(unsigned Rep) {
    return {std::nullopt, Rep};
  }
  constexpr unsigned toInternalRepresentation() const { return Rep; }

  explicit constexpr operator bool() const { return Rep != 0; }
  unsigned operator*() const {
    assert(operator bool());
    return Rep - 1;
  }

  friend constexpr bool operator==(UnsignedOrNone LHS, UnsignedOrNone RHS) {
    return LHS.Rep == RHS.Rep;
  }
  friend constexpr bool operator!=(UnsignedOrNone LHS, UnsignedOrNone RHS) {
    return LHS.Rep != RHS.Rep;
  }

private:
  constexpr UnsignedOrNone(std::nullopt_t, unsigned Rep) : Rep(Rep) {};

  unsigned Rep;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_UNSIGNED_OR_NONE_H
