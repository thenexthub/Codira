//===------------ ProfileCounter.h - PGO Propfile counter -------*- C++ -*-===//
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
//
/// \file Declares ProfileCounter, a convenient type for PGO
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_PROFILECOUNTER_H
#define LANGUAGE_BASIC_PROFILECOUNTER_H

#include <cassert>
#include <cstdint>

namespace language {
/// A class designed to be smaller than using Optional<uint64_t> for PGO
class ProfileCounter {
private:
  uint64_t count;

public:
  explicit constexpr ProfileCounter() : count(UINT64_MAX) {}
  constexpr ProfileCounter(uint64_t Count) : count(Count) {
    if (Count == UINT64_MAX) {
      count = UINT64_MAX - 1;
    }
  }

  bool hasValue() const { return count != UINT64_MAX; }
  uint64_t getValue() const {
    assert(hasValue());
    return count;
  }
  explicit operator bool() const { return hasValue(); }

  /// Saturating addition of another counter to this one, meaning that overflow
  /// is avoided. If overflow would have happened, this function returns true
  /// and the maximum representable value will be set in this counter.
  bool add_saturating(ProfileCounter other) {
    assert(hasValue() && other.hasValue());

    // Will we go over the max representable value by adding other?
    if (count > ((UINT64_MAX-1) - other.count)) {
      count = UINT64_MAX - 1;
      return true;
    }

    count += other.count;
    return false;
  }
};
} // end namespace language

#endif // LANGUAGE_BASIC_PROFILECOUNTER_H
