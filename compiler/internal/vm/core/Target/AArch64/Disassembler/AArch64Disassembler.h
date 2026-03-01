//===- AArch64Disassembler.h - Disassembler for AArch64 ---------*- C++ -*-===//
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
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_DISASSEMBLER_AARCH64DISASSEMBLER_H
#define LLVM_LIB_TARGET_AARCH64_DISASSEMBLER_AARCH64DISASSEMBLER_H

#include "vm/core/MC/MCDisassembler/MCDisassembler.h"
#include "vm/core/MC/MCInstrInfo.h"

namespace vm::core {

class AArch64Disassembler : public MCDisassembler {
  std::unique_ptr<const MCInstrInfo> const MCII;

public:
  AArch64Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                      MCInstrInfo const *MCII)
      : MCDisassembler(STI, Ctx), MCII(MCII) {}

  ~AArch64Disassembler() override = default;

  MCDisassembler::DecodeStatus
  getInstruction(MCInst &Instr, uint64_t &Size, ArrayRef<uint8_t> Bytes,
                 uint64_t Address, raw_ostream &CStream) const override;

  uint64_t suggestBytesToSkip(ArrayRef<uint8_t> Bytes,
                              uint64_t Address) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AARCH64_DISASSEMBLER_AARCH64DISASSEMBLER_H
