//===-- LanaiBaseInfo.h - Top level definitions for Lanai MC ----*- C++ -*-===//
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
// the Lanai target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LANAI_MCTARGETDESC_LANAIBASEINFO_H
#define LLVM_LIB_TARGET_LANAI_MCTARGETDESC_LANAIBASEINFO_H

#include "LanaiMCTargetDesc.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/Support/DataTypes.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {

// LanaiII - This namespace holds all of the target specific flags that
// instruction info tracks.
namespace LanaiII {
// Target Operand Flag enum.
enum TOF {
  //===------------------------------------------------------------------===//
  // Lanai Specific MachineOperand flags.
  MO_NO_FLAG,

  // MO_ABS_HI/LO - Represents the hi or low part of an absolute symbol
  // address.
  MO_ABS_HI,
  MO_ABS_LO,
};
} // namespace LanaiII

static inline unsigned getLanaiRegisterNumbering(MCRegister Reg) {
  switch (Reg.id()) {
  case Lanai::R0:
    return 0;
  case Lanai::R1:
    return 1;
  case Lanai::R2:
  case Lanai::PC:
    return 2;
  case Lanai::R3:
    return 3;
  case Lanai::R4:
  case Lanai::SP:
    return 4;
  case Lanai::R5:
  case Lanai::FP:
    return 5;
  case Lanai::R6:
    return 6;
  case Lanai::R7:
    return 7;
  case Lanai::R8:
  case Lanai::RV:
    return 8;
  case Lanai::R9:
    return 9;
  case Lanai::R10:
  case Lanai::RR1:
    return 10;
  case Lanai::R11:
  case Lanai::RR2:
    return 11;
  case Lanai::R12:
    return 12;
  case Lanai::R13:
    return 13;
  case Lanai::R14:
    return 14;
  case Lanai::R15:
  case Lanai::RCA:
    return 15;
  case Lanai::R16:
    return 16;
  case Lanai::R17:
    return 17;
  case Lanai::R18:
    return 18;
  case Lanai::R19:
    return 19;
  case Lanai::R20:
    return 20;
  case Lanai::R21:
    return 21;
  case Lanai::R22:
    return 22;
  case Lanai::R23:
    return 23;
  case Lanai::R24:
    return 24;
  case Lanai::R25:
    return 25;
  case Lanai::R26:
    return 26;
  case Lanai::R27:
    return 27;
  case Lanai::R28:
    return 28;
  case Lanai::R29:
    return 29;
  case Lanai::R30:
    return 30;
  case Lanai::R31:
    return 31;
  default:
    llvm_unreachable("Unknown register number!");
  }
}
} // namespace vm::core
#endif // LLVM_LIB_TARGET_LANAI_MCTARGETDESC_LANAIBASEINFO_H
