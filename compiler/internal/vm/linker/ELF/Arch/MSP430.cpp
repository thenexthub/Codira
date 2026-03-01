//===- MSP430.cpp ---------------------------------------------------------===//
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
// The MSP430 is a 16-bit microcontroller RISC architecture. The instruction set
// has only 27 core instructions orthogonally augmented with a variety
// of addressing modes for source and destination operands. Entire address space
// of MSP430 is 64KB (the extended MSP430X architecture is not considered here).
// A typical MSP430 MCU has several kilobytes of RAM and ROM, plenty
// of peripherals and is generally optimized for a low power consumption.
//
//===----------------------------------------------------------------------===//

#include "Symbols.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class MSP430 final : public TargetInfo {
public:
  MSP430(Ctx &);
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
};
} // namespace

MSP430::MSP430(Ctx &ctx) : TargetInfo(ctx) {
  // mov.b #0, r3
  trapInstr = {0x43, 0x43, 0x43, 0x43};
}

RelExpr MSP430::getRelExpr(RelType type, const Symbol &s,
                           const uint8_t *loc) const {
  switch (type) {
  case R_MSP430_10_PCREL:
  case R_MSP430_16_PCREL:
  case R_MSP430_16_PCREL_BYTE:
  case R_MSP430_2X_PCREL:
  case R_MSP430_RL_PCREL:
  case R_MSP430_SYM_DIFF:
    return R_PC;
  default:
    return R_ABS;
  }
}

void MSP430::relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const {
  switch (rel.type) {
  case R_MSP430_8:
    checkIntUInt(ctx, loc, val, 8, rel);
    *loc = val;
    break;
  case R_MSP430_16:
  case R_MSP430_16_PCREL:
  case R_MSP430_16_BYTE:
  case R_MSP430_16_PCREL_BYTE:
    checkIntUInt(ctx, loc, val, 16, rel);
    write16le(loc, val);
    break;
  case R_MSP430_32:
    checkIntUInt(ctx, loc, val, 32, rel);
    write32le(loc, val);
    break;
  case R_MSP430_10_PCREL: {
    int16_t offset = ((int16_t)val >> 1) - 1;
    checkInt(ctx, loc, offset, 10, rel);
    write16le(loc, (read16le(loc) & 0xFC00) | (offset & 0x3FF));
    break;
  }
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unrecognized relocation " << rel.type;
  }
}

void elf::setMSP430TargetInfo(Ctx &ctx) { ctx.target.reset(new MSP430(ctx)); }
