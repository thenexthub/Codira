//===-- lib/Evaluate/character.h --------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_CHARACTER_H_
#define LANGUAGE_COMPABILITY_EVALUATE_CHARACTER_H_

#include "language/Compability/Evaluate/type.h"
#include <string>

// Provides implementations of intrinsic functions operating on character
// scalars.

namespace language::Compability::evaluate {

template <int KIND> class CharacterUtils {
  using Character = Scalar<Type<TypeCategory::Character, KIND>>;
  using CharT = typename Character::value_type;

public:
  // CHAR also implements ACHAR under assumption that character encodings
  // contain ASCII
  static Character CHAR(std::uint64_t code) {
    return Character{{static_cast<CharT>(code)}};
  }

  // ICHAR also implements IACHAR under assumption that character encodings
  // contain ASCII
  static std::int64_t ICHAR(const Character &c) {
    CHECK(c.length() == 1);
    // Convert first to an unsigned integer type to avoid sign extension
    return static_cast<common::HostUnsignedIntType<(8 * KIND)>>(c[0]);
  }

  static Character NEW_LINE() { return Character{{NewLine()}}; }

  static Character ADJUSTL(const Character &str) {
    auto pos{str.find_first_not_of(Space())};
    if (pos != Character::npos && pos != 0) {
      return Character{str.substr(pos) + Character(pos, Space())};
    }
    // else empty or only spaces, or no leading spaces
    return str;
  }

  static Character ADJUSTR(const Character &str) {
    auto pos{str.find_last_not_of(Space())};
    if (pos != Character::npos && pos != str.length() - 1) {
      auto delta{str.length() - 1 - pos};
      return Character{Character(delta, Space()) + str.substr(0, pos + 1)};
    }
    // else empty or only spaces, or no trailing spaces
    return str;
  }

  static ConstantSubscript INDEX(
      const Character &str, const Character &substr, bool back = false) {
    auto pos{back ? str.rfind(substr) : str.find(substr)};
    return static_cast<ConstantSubscript>(pos == str.npos ? 0 : pos + 1);
  }

  static ConstantSubscript SCAN(
      const Character &str, const Character &set, bool back = false) {
    auto pos{back ? str.find_last_of(set) : str.find_first_of(set)};
    return static_cast<ConstantSubscript>(pos == str.npos ? 0 : pos + 1);
  }

  static ConstantSubscript VERIFY(
      const Character &str, const Character &set, bool back = false) {
    auto pos{back ? str.find_last_not_of(set) : str.find_first_not_of(set)};
    return static_cast<ConstantSubscript>(pos == str.npos ? 0 : pos + 1);
  }

  // Resize adds spaces on the right if the new size is bigger than the
  // original, or by trimming the rightmost characters otherwise.
  static Character Resize(const Character &str, std::size_t newLength) {
    auto oldLength{str.length()};
    if (newLength > oldLength) {
      return str + Character(newLength - oldLength, Space());
    } else {
      return str.substr(0, newLength);
    }
  }

  static ConstantSubscript LEN_TRIM(const Character &str) {
    auto j{str.length()};
    for (; j >= 1; --j) {
      if (str[j - 1] != ' ') {
        break;
      }
    }
    return static_cast<ConstantSubscript>(j);
  }

  static Character REPEAT(const Character &str, ConstantSubscript ncopies) {
    Character result;
    if (!str.empty() && ncopies > 0) {
      result.reserve(ncopies * str.size());
      while (ncopies-- > 0) {
        result += str;
      }
    }
    return result;
  }

  static Character TRIM(const Character &str) {
    return str.substr(0, LEN_TRIM(str));
  }

private:
  // Following helpers assume that character encodings contain ASCII
  static constexpr CharT Space() { return 0x20; }
  static constexpr CharT NewLine() { return 0x0a; }
};

} // namespace language::Compability::evaluate

#endif // FORTRAN_EVALUATE_CHARACTER_H_
