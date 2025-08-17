//===-- language/Compability/Evaluate/logical.h ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_LOGICAL_H_
#define LANGUAGE_COMPABILITY_EVALUATE_LOGICAL_H_

#include "integer.h"
#include <cinttypes>

namespace language::Compability::evaluate::value {

template <int BITS, bool IS_LIKE_C = true> class Logical {
public:
  static constexpr int bits{BITS};
  using Word = Integer<bits>;

  // Module ISO_C_BINDING kind C_BOOL is LOGICAL(KIND=1) and must have
  // C's bit representation (.TRUE. -> 1, .FALSE. -> 0).
  static constexpr bool IsLikeC{BITS <= 8 || IS_LIKE_C};

  constexpr Logical() {} // .FALSE.
  template <int B, bool C>
  constexpr Logical(Logical<B, C> x) : word_{Represent(x.IsTrue())} {}
  constexpr Logical(bool truth) : word_{Represent(truth)} {}
  // A raw word, for DATA initialization
  constexpr Logical(Word &&w) : word_{std::move(w)} {}

  template <int B, bool C> constexpr Logical &operator=(Logical<B, C> x) {
    word_ = Represent(x.IsTrue());
    return *this;
  }

  Word word() const { return word_; }
  bool IsCanonical() const {
    return word_ == canonicalFalse || word_ == canonicalTrue;
  }

  // Fortran actually has only .EQV. & .NEQV. relational operations
  // for LOGICAL, but this template class supports more so that
  // it can be used with the STL for sorting and as a key type for
  // std::set<> & std::map<>.
  template <int B, bool C>
  constexpr bool operator<(const Logical<B, C> &that) const {
    return !IsTrue() && that.IsTrue();
  }
  template <int B, bool C>
  constexpr bool operator<=(const Logical<B, C> &) const {
    return !IsTrue();
  }
  template <int B, bool C>
  constexpr bool operator==(const Logical<B, C> &that) const {
    return IsTrue() == that.IsTrue();
  }
  template <int B, bool C>
  constexpr bool operator!=(const Logical<B, C> &that) const {
    return IsTrue() != that.IsTrue();
  }
  template <int B, bool C>
  constexpr bool operator>=(const Logical<B, C> &) const {
    return IsTrue();
  }
  template <int B, bool C>
  constexpr bool operator>(const Logical<B, C> &that) const {
    return IsTrue() && !that.IsTrue();
  }

  constexpr bool IsTrue() const {
    if constexpr (IsLikeC) {
      return !word_.IsZero();
    } else {
      return word_.BTEST(0);
    }
  }

  constexpr Logical NOT() const { return {word_.IEOR(canonicalTrue)}; }

  constexpr Logical AND(const Logical &that) const {
    return {word_.IAND(that.word_)};
  }

  constexpr Logical OR(const Logical &that) const {
    return {word_.IOR(that.word_)};
  }

  constexpr Logical EQV(const Logical &that) const { return NEQV(that).NOT(); }

  constexpr Logical NEQV(const Logical &that) const {
    return {word_.IEOR(that.word_)};
  }

private:
  static constexpr Word canonicalTrue{IsLikeC ? 1 : -std::uint64_t{1}};
  static constexpr Word canonicalFalse{0};
  static constexpr Word Represent(bool x) {
    return x ? canonicalTrue : canonicalFalse;
  }
  Word word_;
};

extern template class Logical<8>;
extern template class Logical<16>;
extern template class Logical<32>;
extern template class Logical<64>;
} // namespace language::Compability::evaluate::value
#endif // FORTRAN_EVALUATE_LOGICAL_H_
