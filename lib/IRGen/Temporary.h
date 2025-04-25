//===--- Temporary.h - A temporary allocation -------------------*- C++ -*-===//
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
//  This file defines the Temporary and TemporarySet classes.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_TEMPORARY_H
#define SWIFT_IRGEN_TEMPORARY_H

#include "Address.h"
#include "language/SIL/SILType.h"
#include <vector>

namespace language {
namespace irgen {

class IRGenFunction;

/// A temporary allocation.
class Temporary {
public:
  StackAddress Addr;
  SILType Type;

  void destroy(IRGenFunction &IGF) const;
};

class TemporarySet {
  std::vector<Temporary> Stack;
  bool HasBeenCleared = false;

public:
  TemporarySet() = default;

  TemporarySet(TemporarySet &&) = default;
  TemporarySet &operator=(TemporarySet &&) = default;

  // Make this move-only to reduce chances of double-destroys.  We can't
  // get too strict with this, though, because we may need to destroy
  // the same set of temporaries along multiple control-flow paths.
  TemporarySet(const TemporarySet &) = delete;
  TemporarySet &operator=(const TemporarySet &) = delete;

  void add(Temporary temp) {
    Stack.push_back(temp);
  }

  /// Destroy all the temporaries.
  void destroyAll(IRGenFunction &IGF) const;

  /// Remove all the temporaries from this set.  This does not destroy
  /// the temporaries.
  void clear() {
    assert(!HasBeenCleared && "already cleared");
    HasBeenCleared = true;
    Stack.clear();
  }

  /// Has clear() been called on this set?
  bool hasBeenCleared() const {
    return HasBeenCleared;
  }
};

} // end namespace irgen
} // end namespace language

#endif
