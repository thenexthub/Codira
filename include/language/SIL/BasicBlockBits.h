//===--- BasicBlockBits.h - SILBasicBlock bit utilities ---------*- C++ -*-===//
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
// This file defines utilities for BasicBlock bit fields and sets.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SIL_BASICBLOCKBITS_H
#define SWIFT_SIL_BASICBLOCKBITS_H

#include "language/SIL/SILBitfield.h"

namespace language {

/// Utility to add a custom bitfield to a function's basic blocks.
///
/// This can be used by transforms to store temporary flags or tiny values per
/// basic block.
/// The bits are stored in a 32 bit field within each basic block (\see
/// SILBasicBlock::customBits) which is very efficient: no memory allocation
/// is needed, no hash set or map is needed for lookup and there is no
/// initialization cost (in contrast to BasicBlockData which needs to iterate
/// over all blocks at initialization).
///
/// Invariants:
/// * BasicBlockBitfield instances must be allocated and deallocated
///   following a strict stack discipline, because bit-positions in
///   SILBasicBlock::customBits are "allocated" and "freed" with a stack-allocation
///   algorithm. This means, it's fine to use a BasicBlockBitfield as (or in)
///   local variables, e.g. in transformations. But it's not possible to store
///   a BasicBlockBitfield in an Analysis.
/// * The total number of bits which are alive at the same time must not exceed
///   32 (the size of SILBasicBlock::customBits).
class BasicBlockBitfield : public SILBitfield<BasicBlockBitfield, SILBasicBlock> {
  template <class, class> friend class SILBitfield;

  BasicBlockBitfield *insertInto(SILFunction *function) {
    BasicBlockBitfield *oldParent = function->newestAliveBlockBitfield;
    function->newestAliveBlockBitfield = this;
    return oldParent;
  }

public:
  BasicBlockBitfield(SILFunction *function, int size) :
    SILBitfield(function, size, insertInto(function)) {}

  ~BasicBlockBitfield() {
    assert(function->newestAliveBlockBitfield == this &&
           "BasicBlockBitfield destructed too early");
    function->newestAliveBlockBitfield = parent;
  }
};

/// A BasicBlockBitfield containing a single bit - a flag.
class BasicBlockFlag {
  BasicBlockBitfield bit;

public:
  BasicBlockFlag(SILFunction *function) : bit(function, 1) {}

  SILFunction *getFunction() const { return bit.getFunction(); }
  
  bool get(SILBasicBlock *block) const { return (bool)bit.get(block); }

  void set(SILBasicBlock *block, bool value = true) {
    bit.set(block, (unsigned)value);
  }
  void reset(SILBasicBlock *block) { bit.set(block, 0); }

  /// Sets the flag and returns the old value.
  bool testAndSet(SILBasicBlock *block) {
    bool oldValue = get(block);
    set(block);
    return oldValue;
  }
};

/// A BasicBlockFlag with a set-like API.
class BasicBlockSet {
  BasicBlockFlag flag;

public:
  using Element = SILBasicBlock *;

  BasicBlockSet(SILFunction *function) : flag(function) {}

  SILFunction *getFunction() const { return flag.getFunction(); }

  bool contains(SILBasicBlock *block) const { return flag.get(block); }

  /// Returns true if \p block was not contained in the set before inserting.
  bool insert(SILBasicBlock *block) { return !flag.testAndSet(block); }

  void erase(SILBasicBlock *block) { flag.reset(block); }
};

using BasicBlockSetWithSize = KnownSizeSet<BasicBlockSet>;

} // namespace language

#endif
