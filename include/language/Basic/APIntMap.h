//===--- APIntMap.h - A map with APInts as the keys -------------*- C++ -*-===//
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
// LLVM does not allow arbitrary APInts to be the keys of a DenseMap because
// APInts are only comparable if they have the same bit-width.  This map
// implementation assumes that its keys will always be constrained to their
// minimum width, so it's not a general-purpose structure, but it does work.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_APINTMAP_H
#define LANGUAGE_BASIC_APINTMAP_H

#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/DenseMap.h"
#include "language/Basic/Toolchain.h"

namespace language {

struct WidthPreservingAPIntDenseMapInfo {
  // For the special values, we use -1 with a bit-width that isn't minimal
  // for the value, then use a parser that always produces values with
  // minimal bit-widths so that we don't get a conflict.
  static inline APInt getEmptyKey() {
    return APInt::getAllOnes(/*bitwidth*/2);
  }
  static inline APInt getTombstoneKey() {
    return APInt::getAllOnes(/*bitwidth*/3);
  }

  static unsigned getHashValue(const APInt &Key) {
    return toolchain::DenseMapInfo<APInt>::getHashValue(Key);
  }

  static bool isEqual(const APInt &LHS, const APInt &RHS) {
    return toolchain::DenseMapInfo<APInt>::isEqual(LHS, RHS);
  }
};

template <class Value>
using APIntMap = toolchain::DenseMap<APInt, Value, WidthPreservingAPIntDenseMapInfo>;

}

#endif
