//===-- ARMUnwindOpAsm.h - ARM Unwind Opcodes Assembler ---------*- C++ -*-===//
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
// This file declares the unwind opcode assembler for ARM exception handling
// table.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_MCTARGETDESC_ARMUNWINDOPASM_H
#define LLVM_LIB_TARGET_ARM_MCTARGETDESC_ARMUNWINDOPASM_H

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include <cstddef>
#include <cstdint>

namespace vm::core {

class MCSymbol;

class UnwindOpcodeAssembler {
private:
  SmallVector<uint8_t, 32> Ops;
  SmallVector<unsigned, 8> OpBegins;
  bool HasPersonality = false;

public:
  UnwindOpcodeAssembler() {
    OpBegins.push_back(0);
  }

  /// Reset the unwind opcode assembler.
  void Reset() {
    Ops.clear();
    OpBegins.clear();
    OpBegins.push_back(0);
    HasPersonality = false;
  }

  /// Set the personality
  void setPersonality(const MCSymbol *Per) {
    HasPersonality = true;
  }

  /// Emit unwind opcodes for .save directives
  void EmitRegSave(uint32_t RegSave);

  /// Emit unwind opcodes for .vsave directives
  void EmitVFPRegSave(uint32_t VFPRegSave);

  /// Emit unwind opcodes to copy address from source register to $sp.
  void EmitSetSP(uint16_t Reg);

  /// Emit unwind opcodes to add $sp with an offset.
  void EmitSPOffset(int64_t Offset);

  /// Emit unwind raw opcodes
  void EmitRaw(const SmallVectorImpl<uint8_t> &Opcodes) {
    toolchain::append_range(Ops, Opcodes);
    OpBegins.push_back(OpBegins.back() + Opcodes.size());
  }

  /// Finalize the unwind opcode sequence for emitBytes()
  void Finalize(unsigned &PersonalityIndex,
                SmallVectorImpl<uint8_t> &Result);

private:
  void EmitInt8(unsigned Opcode) {
    Ops.push_back(Opcode & 0xff);
    OpBegins.push_back(OpBegins.back() + 1);
  }

  void EmitInt16(unsigned Opcode) {
    Ops.push_back((Opcode >> 8) & 0xff);
    Ops.push_back(Opcode & 0xff);
    OpBegins.push_back(OpBegins.back() + 2);
  }

  void emitBytes(const uint8_t *Opcode, size_t Size) {
    Ops.insert(Ops.end(), Opcode, Opcode + Size);
    OpBegins.push_back(OpBegins.back() + Size);
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_MCTARGETDESC_ARMUNWINDOPASM_H
