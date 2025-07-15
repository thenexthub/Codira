//===--- GraphNodeWorklist.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_GRAPHNODEWORKLIST_H
#define LANGUAGE_BASIC_GRAPHNODEWORKLIST_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"

/// Worklist of pointer-like things that have an invalid default value. This not
/// only avoids duplicates in the worklist, but also avoids revisiting
/// already-popped nodes. This makes it suitable for DAG traversal. This can
/// also be used within hybrid worklist/recursive traversal by recording the
/// size of the worklist at each level of recursion.
///
/// The primary API has two methods: initialize() and pop(). Others are provided
/// for flexibility.
template <typename T, unsigned SmallSize>
struct GraphNodeWorklist {
  toolchain::SmallPtrSet<T, SmallSize> nodeVisited;
  toolchain::SmallVector<T, SmallSize> nodeVector;

  GraphNodeWorklist() = default;

  GraphNodeWorklist(const GraphNodeWorklist &) = delete;

  void initialize(T t) {
    clear();
    insert(t);
  }

  template <typename R>
  void initializeRange(R &&range) {
    clear();
    nodeVisited.insert(range.begin(), range.end());
    nodeVector.append(range.begin(), range.end());
  }

  T pop() { return empty() ? T() : nodeVector.pop_back_val(); }

  bool empty() const { return nodeVector.empty(); }

  unsigned size() const { return nodeVector.size(); }

  void clear() {
    nodeVector.clear();
    nodeVisited.clear();
  }

  void insert(T t) {
    if (nodeVisited.insert(t).second)
      nodeVector.push_back(t);
  }
};

#endif // LANGUAGE_BASIC_GRAPHNODEWORKLIST_H
