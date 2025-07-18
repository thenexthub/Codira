//===--- NodeDatastructures.h -----------------------------------*- C++ -*-===//
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
// This file defines efficient data structures for working with Nodes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_NODEDATASTRUCTURES_H
#define LANGUAGE_SIL_NODEDATASTRUCTURES_H

#include "language/SIL/NodeBits.h"
#include "language/SIL/StackList.h"

namespace language {

/// An implementation of `toolchain::SetVector<SILNode *,
///                                       StackList<SILNode *>,
///                                       NodeSet>`.
///
/// Unfortunately it's not possible to use `toolchain::SetVector` directly because
/// the NodeSet and StackList constructors needs a `SILFunction` argument.
///
/// Note: This class does not provide a `remove` method intentionally, because
/// it would have a O(n) complexity.
class NodeSetVector {
  StackList<SILNode *> vector;
  NodeSet set;

public:
  using iterator = typename StackList<SILNode *>::iterator;

  NodeSetVector(SILFunction *function) : vector(function), set(function) {}

  iterator begin() const { return vector.begin(); }
  iterator end() const { return vector.end(); }

  toolchain::iterator_range<iterator> getRange() const {
    return toolchain::make_range(begin(), end());
  }

  bool empty() const { return vector.empty(); }

  bool contains(SILNode *node) const { return set.contains(node); }

  /// Returns true if \p value was not contained in the set before inserting.
  bool insert(SILNode *node) {
    if (set.insert(node)) {
      vector.push_back(node);
      return true;
    }
    return false;
  }
};

/// An implementation of `toolchain::SetVector<SILInstruction *,
///                                       StackList<SILInstruction *>,
///                                       InstructionSet>`.
///
/// Unfortunately it's not possible to use `toolchain::SetVector` directly because
/// the InstructionSet and StackList constructors needs a `SILFunction`
/// argument.
///
/// Note: This class does not provide a `remove` method intentionally, because
/// it would have a O(n) complexity.
class InstructionSetVector {
  StackList<SILInstruction *> vector;
  InstructionSet set;

public:
  using iterator = typename StackList<SILInstruction *>::iterator;

  InstructionSetVector(SILFunction *function)
      : vector(function), set(function) {}

  iterator begin() const { return vector.begin(); }
  iterator end() const { return vector.end(); }

  toolchain::iterator_range<iterator> getRange() const {
    return toolchain::make_range(begin(), end());
  }

  bool empty() const { return vector.empty(); }

  bool contains(SILInstruction *instruction) const {
    return set.contains(instruction);
  }

  /// Returns true if \p instruction was not contained in the set before
  /// inserting.
  bool insert(SILInstruction *instruction) {
    if (set.insert(instruction)) {
      vector.push_back(instruction);
      return true;
    }
    return false;
  }
};

/// A utility for processing instructions in a worklist.
///
/// It is basically a combination of an instruction vector and an instruction
/// set. It can be used for typical worklist-processing algorithms.
class InstructionWorklist {
  StackList<SILInstruction *> worklist;
  InstructionSet visited;

public:
  /// Construct an empty worklist.
  InstructionWorklist(SILFunction *function)
      : worklist(function), visited(function) {}

  /// Initialize the worklist with \p initialBlock.
  InstructionWorklist(SILInstruction *initialInstruction)
      : InstructionWorklist(initialInstruction->getFunction()) {
    push(initialInstruction);
  }

  /// Pops the last added element from the worklist or returns null, if the
  /// worklist is empty.
  SILInstruction *pop() {
    if (worklist.empty())
      return nullptr;
    return worklist.pop_back_val();
  }

  /// Pushes \p instruction onto the worklist if \p instruction has never been
  /// push before.
  bool pushIfNotVisited(SILInstruction *instruction) {
    if (visited.insert(instruction)) {
      worklist.push_back(instruction);
      return true;
    }
    return false;
  }

  /// Like `pushIfNotVisited`, but requires that \p instruction has never been
  /// on the worklist before.
  void push(SILInstruction *instruction) {
    assert(!visited.contains(instruction));
    visited.insert(instruction);
    worklist.push_back(instruction);
  }

  /// Like `pop`, but marks the returned instruction as "unvisited". This means,
  /// that the instruction can be pushed onto the worklist again.
  SILInstruction *popAndForget() {
    if (worklist.empty())
      return nullptr;
    SILInstruction *instruction = worklist.pop_back_val();
    visited.erase(instruction);
    return instruction;
  }

  /// Returns true if \p instruction was visited, i.e. has been added to the
  /// worklist.
  bool isVisited(SILInstruction *instruction) const {
    return visited.contains(instruction);
  }
};

/// An implementation of `toolchain::SetVector<SILValue,
///                                       StackList<SILValue>,
///                                       ValueSet>`.
///
/// Unfortunately it's not possible to use `toolchain::SetVector` directly because
/// the ValueSet and StackList constructors needs a `SILFunction` argument.
///
/// Note: This class does not provide a `remove` method intentionally, because
/// it would have a O(n) complexity.
class ValueSetVector {
  StackList<SILValue> vector;
  ValueSet set;

public:
  using iterator = typename StackList<SILValue>::iterator;

  ValueSetVector(SILFunction *function) : vector(function), set(function) {}

  iterator begin() const { return vector.begin(); }
  iterator end() const { return vector.end(); }

  toolchain::iterator_range<iterator> getRange() const {
    return toolchain::make_range(begin(), end());
  }

  bool empty() const { return vector.empty(); }

  bool contains(SILValue value) const { return set.contains(value); }

  /// Returns true if \p value was not contained in the set before inserting.
  bool insert(SILValue value) {
    if (set.insert(value)) {
      vector.push_back(value);
      return true;
    }
    return false;
  }
};

/// A utility for processing values in a worklist.
///
/// It is basically a combination of a value vector and a value set. It can be
/// used for typical worklist-processing algorithms.
class ValueWorklist {
  StackList<SILValue> worklist;
  ValueSet visited;

public:
  /// Construct an empty worklist.
  ValueWorklist(SILFunction *function)
      : worklist(function), visited(function) {}

  /// Initialize the worklist with \p initialValue.
  ValueWorklist(SILValue initialValue)
      : ValueWorklist(initialValue->getFunction()) {
    push(initialValue);
  }

  /// Pops the last added element from the worklist or returns null, if the
  /// worklist is empty.
  SILValue pop() {
    if (worklist.empty())
      return nullptr;
    return worklist.pop_back_val();
  }

  /// Pushes \p value onto the worklist if \p value has never been push before.
  bool pushIfNotVisited(SILValue value) {
    if (visited.insert(value)) {
      worklist.push_back(value);
      return true;
    }
    return false;
  }

  /// Like `pushIfNotVisited`, but requires that \p value has never been on the
  /// worklist before.
  void push(SILValue value) {
    assert(!visited.contains(value));
    visited.insert(value);
    worklist.push_back(value);
  }

  /// Like `pop`, but marks the returned value as "unvisited". This means, that
  /// the value can be pushed onto the worklist again.
  SILValue popAndForget() {
    if (worklist.empty())
      return nullptr;
    SILValue value = worklist.pop_back_val();
    visited.erase(value);
    return value;
  }

  /// Returns true if \p value was visited, i.e. has been added to the worklist.
  bool isVisited(SILValue value) const { return visited.contains(value); }
};

} // namespace language

#endif
