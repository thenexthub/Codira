//===-- ARMFeatures.h - Checks for ARM instruction features -----*- C++ -*-===//
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
// This file contains the code shared between ARM CodeGen and ARM MC
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARMFEATURES_H
#define LLVM_LIB_TARGET_ARM_ARMFEATURES_H

#include "MCTargetDesc/ARMMCTargetDesc.h"

namespace vm::core {

template<typename InstrType> // could be MachineInstr or MCInst
bool IsCPSRDead(const InstrType *Instr);

template<typename InstrType> // could be MachineInstr or MCInst
inline bool isV8EligibleForIT(const InstrType *Instr) {
  switch (Instr->getOpcode()) {
  default:
    return false;
  case ARM::tADC:
  case ARM::tADDi3:
  case ARM::tADDi8:
  case ARM::tADDrr:
  case ARM::tAND:
  case ARM::tASRri:
  case ARM::tASRrr:
  case ARM::tBIC:
  case ARM::tEOR:
  case ARM::tLSLri:
  case ARM::tLSLrr:
  case ARM::tLSRri:
  case ARM::tLSRrr:
  case ARM::tMOVi8:
  case ARM::tMUL:
  case ARM::tMVN:
  case ARM::tORR:
  case ARM::tROR:
  case ARM::tRSB:
  case ARM::tSBC:
  case ARM::tSUBi3:
  case ARM::tSUBi8:
  case ARM::tSUBrr:
    // Outside of an IT block, these set CPSR.
    return IsCPSRDead(Instr);
  case ARM::tADDrSPi:
  case ARM::tCMNz:
  case ARM::tCMPi8:
  case ARM::tCMPr:
  case ARM::tLDRBi:
  case ARM::tLDRBr:
  case ARM::tLDRHi:
  case ARM::tLDRHr:
  case ARM::tLDRSB:
  case ARM::tLDRSH:
  case ARM::tLDRi:
  case ARM::tLDRr:
  case ARM::tLDRspi:
  case ARM::tSTRBi:
  case ARM::tSTRBr:
  case ARM::tSTRHi:
  case ARM::tSTRHr:
  case ARM::tSTRi:
  case ARM::tSTRr:
  case ARM::tSTRspi:
  case ARM::tTST:
    return true;
// there are some "conditionally deprecated" opcodes
  case ARM::tADDspr:
  case ARM::tBLXr:
  case ARM::tBLXr_noip:
    return Instr->getOperand(2).getReg() != ARM::PC;
  // ADD PC, SP and BLX PC were always unpredictable,
  // now on top of it they're deprecated
  case ARM::tADDrSP:
  case ARM::tBX:
    return Instr->getOperand(0).getReg() != ARM::PC;
  case ARM::tADDhirr:
    return Instr->getOperand(0).getReg() != ARM::PC &&
           Instr->getOperand(2).getReg() != ARM::PC;
  case ARM::tCMPhir:
  case ARM::tMOVr:
    return Instr->getOperand(0).getReg() != ARM::PC &&
           Instr->getOperand(1).getReg() != ARM::PC;
  }
}

}

#endif
