//===--- BasicBlockDatastructures.h -----------------------------*- C++ -*-===//
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
// This file defines efficient data structures for working with BasicBlocks.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SIL_BASICBLOCKDATASTRUCTURES_H
#define SWIFT_SIL_BASICBLOCKDATASTRUCTURES_H

#include "language/SIL/StackList.h"
#include "language/SIL/BasicBlockBits.h"
#include <deque>

namespace language {

/// An implementation of `llvm::SetVector<SILBasicBlock *,
///                                       StackList<SILBasicBlock *>,
///                                       BasicBlockSet>`.
///
/// Unfortunately it's not possible to use `llvm::SetVector` directly because
/// the BasicBlockSet and StackList constructors needs a `SILFunction` argument.
///
/// Note: This class does not provide a `remove` method intentionally, because
/// it would have a O(n) complexity.
class BasicBlockSetVector {
  StackList<SILBasicBlock *> vector;
  BasicBlockSet set;
  
public:
  using iterator = typename StackList<SILBasicBlock *>::iterator;

  BasicBlockSetVector(SILFunction *function) : vector(function), set(function) {}

  iterator begin() const { return vector.begin(); }
  iterator end() const { return vector.end(); }

  llvm::iterator_range<iterator> getRange() const {
    return llvm::make_range(begin(), end());
  }

  bool empty() const { return vector.empty(); }

  bool contains(SILBasicBlock *block) const { return set.contains(block); }

  /// Returns true if \p block was not contained in the set before inserting.
  bool insert(SILBasicBlock *block) {
    if (set.insert(block)) {
      vector.push_back(block);
      return true;
    }
    return false;
  }
};

/// A utility for processing basic blocks in a worklist that uses
/// last-in-first-out order.
///
/// It is basically a combination of a stack list and a block set. It can be
/// used for typical worklist-processing algorithms and is recommended in
/// nearly all situations as its implementation is well-optimized.
class BasicBlockWorklist {
  StackList<SILBasicBlock *> worklist;
  BasicBlockSet visited;
  
public:
  /// Construct an empty worklist.
  BasicBlockWorklist(SILFunction *function)
    : worklist(function), visited(function) {}

  /// Initialize the worklist with \p initialBlock.
  BasicBlockWorklist(SILBasicBlock *initialBlock)
      : BasicBlockWorklist(initialBlock->getParent()) {
    push(initialBlock);
  }

  /// Whether there are any remaining blocks to process.
  bool empty() { return worklist.empty(); }

  /// Pops the last added element from the worklist or returns null, if the
  /// worklist is empty.
  SILBasicBlock *pop() {
    if (worklist.empty())
      return nullptr;
    return worklist.pop_back_val();
  }

  /// Pushes \p block onto the worklist if \p block has never been push before.
  bool pushIfNotVisited(SILBasicBlock *block) {
    if (visited.insert(block)) {
      worklist.push_back(block);
      return true;
    }
    return false;
  }

  /// Like `pushIfNotVisited`, but requires that \p block has never been on the
  /// worklist before.
  void push(SILBasicBlock *block) {
    assert(!visited.contains(block));
    visited.insert(block);
    worklist.push_back(block);
  }

  /// Like `pop`, but marks the returned block as "unvisited". This means, that
  /// the block can be pushed onto the worklist again.
  SILBasicBlock *popAndForget() {
    if (worklist.empty())
      return nullptr;
    SILBasicBlock *block = worklist.pop_back_val();
    visited.erase(block);
    return block;
  }

  /// Returns true if \p block was visited, i.e. has been added to the worklist.
  bool isVisited(SILBasicBlock *block) const { return visited.contains(block); }
};


/// A utility for processing basic blocks in a work-queue that uses
/// first-in-first-out order.
///
/// It is basically a combination of a std::deque and a block set. It can be
/// used for certain kinds of worklist-processing algorithms.
///
/// When in doubt, you should reach for a BasicBlockWorklist instead.
class BasicBlockWorkqueue {
  std::deque<SILBasicBlock*> workqueue;
  BasicBlockSet visited;

public:
  /// Construct an empty workqueue.
  BasicBlockWorkqueue(SILFunction *function)
      : workqueue(), visited(function) {}

  /// Initialize the workqueue with \p initialBlock.
  BasicBlockWorkqueue(SILBasicBlock *initialBlock)
      : BasicBlockWorkqueue(initialBlock->getParent()) {
    push(initialBlock);
  }

  /// Pops the next element from the front of the workqueue or returns null,
  /// if the workqueue is empty.
  SILBasicBlock *pop() {
    if (workqueue.empty())
      return nullptr;
    SILBasicBlock *block = workqueue.front();
    workqueue.pop_front();
    return block;
  }

  /// Pushes \p block onto the end of the workqueue if \p block has never been
  /// pushed before.
  bool pushIfNotVisited(SILBasicBlock *block) {
    if (visited.insert(block)) {
      workqueue.push_back(block);
      return true;
    }
    return false;
  }

  /// Like `pushIfNotVisited`, but requires that \p block has never been on the
  /// workqueue before.
  void push(SILBasicBlock *block) {
    assert(!visited.contains(block));
    visited.insert(block);
    workqueue.push_back(block);
  }

  /// Like `pop`, but marks the returned block as "unvisited". This means, that
  /// the block can be pushed onto the queue again.
  SILBasicBlock *popAndForget() {
    SILBasicBlock *block = pop();
    if (block)
      visited.erase(block);
    return block;
  }

  /// Returns true if \p block was visited, i.e. has been added to the workqueue
  bool isVisited(SILBasicBlock *block) const { return visited.contains(block); }
};

} // namespace language

#endif
