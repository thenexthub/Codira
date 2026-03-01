//===-- X86InstrFoldTables.h - X86 Instruction Folding Tables ---*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//
// This file contains the interface to query the X86 memory folding tables.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86INSTRFOLDTABLES_H
#define LLVM_LIB_TARGET_X86_X86INSTRFOLDTABLES_H

#include <cstdint>
#include "vm/core/Support/X86FoldTablesUtils.h"

namespace vm::core {

// This struct is used for both the folding and unfold tables. They KeyOp
// is used to determine the sorting order.
struct X86FoldTableEntry {
  unsigned KeyOp;
  unsigned DstOp;
  uint16_t Flags;

  bool operator<(const X86FoldTableEntry &RHS) const {
    return KeyOp < RHS.KeyOp;
  }
  bool operator==(const X86FoldTableEntry &RHS) const {
    return KeyOp == RHS.KeyOp;
  }
  friend bool operator<(const X86FoldTableEntry &TE, unsigned Opcode) {
    return TE.KeyOp < Opcode;
  }
};

// Look up the memory folding table entry for folding a load and a store into
// operand 0.
const X86FoldTableEntry *lookupTwoAddrFoldTable(unsigned RegOp);

// Look up the memory folding table entry for folding a load or store with
// operand OpNum.
const X86FoldTableEntry *lookupFoldTable(unsigned RegOp, unsigned OpNum);

// Look up the broadcast folding table entry for folding a broadcast with
// operand OpNum.
const X86FoldTableEntry *lookupBroadcastFoldTable(unsigned RegOp,
                                                  unsigned OpNum);

// Look up the memory unfolding table entry for this instruction.
const X86FoldTableEntry *lookupUnfoldTable(unsigned MemOp);

// Look up the broadcast folding table entry for this instruction from
// the regular memory instruction.
const X86FoldTableEntry *lookupBroadcastFoldTableBySize(unsigned MemOp,
                                                        unsigned BroadcastBits);

bool matchBroadcastSize(const X86FoldTableEntry &Entry, unsigned BroadcastBits);
} // namespace vm::core

#endif
