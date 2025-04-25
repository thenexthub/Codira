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

#ifndef SWIFT_BASIC_SMALLPTRSETVECTOR_H
#define SWIFT_BASIC_SMALLPTRSETVECTOR_H

#include "language/Basic/LLVM.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

namespace language {

/// A SetVector that performs no allocations if smaller than a certain
/// size. Uses a SmallPtrSet/SmallVector internally.
template <typename T, unsigned VectorSize, unsigned SetSize = VectorSize>
class SmallPtrSetVector : public llvm::SetVector<T, SmallVector<T, VectorSize>,
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
/// This matches llvm's implementation for SmallSetVector.
template <typename T, unsigned VectorSize, unsigned SetSize = VectorSize>
inline void swap(swift::SmallPtrSetVector<T, VectorSize, SetSize> &LHS,
                 swift::SmallPtrSetVector<T, VectorSize, SetSize> &RHS) {
  LHS.swap(RHS);
}

} // end namespace std

#endif
