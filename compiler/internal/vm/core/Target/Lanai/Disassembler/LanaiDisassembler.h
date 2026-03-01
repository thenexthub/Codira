//===- LanaiDisassembler.cpp - Disassembler for Lanai -----------*- C++ -*-===//
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
// This file is part of the Lanai Disassembler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LANAI_DISASSEMBLER_LANAIDISASSEMBLER_H
#define LLVM_LIB_TARGET_LANAI_DISASSEMBLER_LANAIDISASSEMBLER_H

#include "vm/core/MC/MCDisassembler/MCDisassembler.h"

namespace vm::core {

class LanaiDisassembler : public MCDisassembler {
public:
  LanaiDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx);

  ~LanaiDisassembler() override = default;

  // getInstruction - See MCDisassembler.
  MCDisassembler::DecodeStatus
  getInstruction(MCInst &Instr, uint64_t &Size, ArrayRef<uint8_t> Bytes,
                 uint64_t Address, raw_ostream &CStream) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LANAI_DISASSEMBLER_LANAIDISASSEMBLER_H
