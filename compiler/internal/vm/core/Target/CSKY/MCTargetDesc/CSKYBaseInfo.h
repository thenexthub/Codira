//===-- CSKYBaseInfo.h - Top level definitions for CSKY ---*- C++ -*-------===//
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
// This file contains small standalone helper functions and enum definitions for
// the CSKY target useful for the compiler back-end and the MC libraries.
// As such, it deliberately does not include references to LLVM core
// code gen types, passes, etc..
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYBASEINFO_H
#define LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYBASEINFO_H

#include "MCTargetDesc/CSKYMCTargetDesc.h"
#include "vm/core/MC/MCInstrDesc.h"

namespace vm::core {

// CSKYII - This namespace holds all of the target specific flags that
// instruction info tracks. All definitions must match CSKYInstrFormats.td.
namespace CSKYII {

enum AddrMode {
  AddrModeNone = 0,
  AddrMode32B = 1,   // ld32.b, ld32.bs, st32.b, st32.bs, +4kb
  AddrMode32H = 2,   // ld32.h, ld32.hs, st32.h, st32.hs, +8kb
  AddrMode32WD = 3,  // ld32.w, st32.w, ld32.d, st32.d, +16kb
  AddrMode16B = 4,   // ld16.b, +32b
  AddrMode16H = 5,   // ld16.h, +64b
  AddrMode16W = 6,   // ld16.w, +128b or +1kb
  AddrMode32SDF = 7, // flds, fldd, +1kb
};

// CSKY Specific MachineOperand Flags.
enum TOF {
  MO_None = 0,
  MO_ADDR32,
  MO_GOT32,
  MO_GOTOFF,
  MO_PLT32,
  MO_ADDR_HI16,
  MO_ADDR_LO16,

  // Used to differentiate between target-specific "direct" flags and "bitmask"
  // flags. A machine operand can only have one "direct" flag, but can have
  // multiple "bitmask" flags.
  MO_DIRECT_FLAG_MASK = 15
};

enum {
  AddrModeMask = 0x1f,
};

} // namespace CSKYII

namespace CSKYOp {
enum OperandType : unsigned {
  OPERAND_BARESYMBOL = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_CONSTPOOL
};
} // namespace CSKYOp

} // namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYBASEINFO_H
