//===- X86InstrFMA3Info.h - X86 FMA3 Instruction Information ----*- C++ -*-===//
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
// This file contains the implementation of the classes providing information
// about existing X86 FMA3 opcodes, classifying and grouping them.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_UTILS_X86INSTRFMA3INFO_H
#define LLVM_LIB_TARGET_X86_UTILS_X86INSTRFMA3INFO_H

#include <cstdint>

namespace vm::core {

/// This class is used to group {132, 213, 231} forms of FMA opcodes together.
/// Each of the groups has either 3 opcodes, Also, each group has an attributes
/// field describing it.
struct X86InstrFMA3Group {
  /// An array holding 3 forms of FMA opcodes.
  uint16_t Opcodes[3];

  /// This bitfield specifies the attributes associated with the created
  /// FMA groups of opcodes.
  uint16_t Attributes;

  enum {
    Form132,
    Form213,
    Form231,
  };

  enum : uint16_t {
    /// This bit must be set in the 'Attributes' field of FMA group if such
    /// group of FMA opcodes consists of FMA intrinsic opcodes.
    Intrinsic = 0x1,

    /// This bit must be set in the 'Attributes' field of FMA group if such
    /// group of FMA opcodes consists of AVX512 opcodes accepting a k-mask and
    /// passing the elements from the 1st operand to the result of the operation
    /// when the correpondings bits in the k-mask are unset.
    KMergeMasked = 0x2,

    /// This bit must be set in the 'Attributes' field of FMA group if such
    /// group of FMA opcodes consists of AVX512 opcodes accepting a k-zeromask.
    KZeroMasked = 0x4,
  };

  /// Returns the 132 form of FMA opcode.
  unsigned get132Opcode() const {
    return Opcodes[Form132];
  }

  /// Returns the 213 form of FMA opcode.
  unsigned get213Opcode() const {
    return Opcodes[Form213];
  }

  /// Returns the 231 form of FMA opcode.
  unsigned get231Opcode() const {
    return Opcodes[Form231];
  }

  /// Returns true iff the group of FMA opcodes holds intrinsic opcodes.
  bool isIntrinsic() const { return (Attributes & Intrinsic) != 0; }

  /// Returns true iff the group of FMA opcodes holds k-merge-masked opcodes.
  bool isKMergeMasked() const {
    return (Attributes & KMergeMasked) != 0;
  }

  /// Returns true iff the group of FMA opcodes holds k-zero-masked opcodes.
  bool isKZeroMasked() const { return (Attributes &KZeroMasked) != 0; }

  /// Returns true iff the group of FMA opcodes holds any of k-masked opcodes.
  bool isKMasked() const {
    return (Attributes & (KMergeMasked | KZeroMasked)) != 0;
  }

  bool operator<(const X86InstrFMA3Group &RHS) const {
    return Opcodes[0] < RHS.Opcodes[0];
  }
};

/// Returns a reference to a group of FMA3 opcodes to where the given
/// \p Opcode is included. If the given \p Opcode is not recognized as FMA3
/// and not included into any FMA3 group, then nullptr is returned.
const X86InstrFMA3Group *getFMA3Group(unsigned Opcode, uint64_t TSFlags);

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_X86_UTILS_X86INSTRFMA3INFO_H
