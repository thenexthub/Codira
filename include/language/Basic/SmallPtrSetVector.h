//===--- SmallPtrSetVector.h ----------------------------------------------===//
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

#ifndef LANGUAGE_BASIC_SMALLPTRSETVECTOR_H
#define LANGUAGE_BASIC_SMALLPTRSETVECTOR_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"

namespace language {

/// A SetVector that performs no allocations if smaller than a certain
/// size. Uses a SmallPtrSet/SmallVector internally.
template <typename T, unsigned VectorSize, unsigned SetSize = VectorSize>
class SmallPtrSetVector : public toolchain::SetVector<T, SmallVector<T, VectorSize>,
                                                 SmallPtrSet<T, SetSize>> {
public:
  SmallPtrSetVector() = default;

  /// Initialize a SmallPtrSetVector with a range of elements
  template <typename It> SmallPtrSetVector(It Start, It End) {
    this->insert(Start, End);
  }
};

} // namespace language

namespace std {

/// Implement std::swap in terms of SmallSetVector swap.
///
/// This matches toolchain's implementation for SmallSetVector.
template <typename T, unsigned VectorSize, unsigned SetSize = VectorSize>
inline void swap(language::SmallPtrSetVector<T, VectorSize, SetSize> &LHS,
                 language::SmallPtrSetVector<T, VectorSize, SetSize> &RHS) {
  LHS.swap(RHS);
}

} // end namespace std

#endif
