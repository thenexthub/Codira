//===-- ARMWinEH.cpp - Windows on ARM EH Support Functions ------*- C++ -*-===//
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

#include "vm/core/Support/ARMWinEH.h"

namespace vm::core {
namespace ARM {
namespace WinEH {
std::pair<uint16_t, uint32_t> SavedRegisterMask(const RuntimeFunction &RF,
                                                bool Prologue) {
  uint8_t NumRegisters = RF.Reg();
  uint8_t RegistersVFP = RF.R();
  uint8_t LinkRegister = RF.L();
  uint8_t ChainedFrame = RF.C();

  uint16_t GPRMask = (ChainedFrame << 11);
  uint32_t VFPMask = 0;

  if (Prologue) {
    GPRMask |= (LinkRegister << 14);
  } else {
    // If Ret != 0, we pop into Lr and return later
    if (RF.Ret() != ReturnType::RT_POP)
      GPRMask |= (LinkRegister << 14);
    else if (!RF.H()) // If H == 0, we pop directly into Pc
      GPRMask |= (LinkRegister << 15);
    // else, Ret == 0 && H == 1, we pop into Pc separately afterwards
  }

  if (RegistersVFP)
    VFPMask |= (((1 << ((NumRegisters + 1) % 8)) - 1) << 8);
  else
    GPRMask |= (((1 << (NumRegisters + 1)) - 1) << 4);

  if ((PrologueFolding(RF) && Prologue) || (EpilogueFolding(RF) && !Prologue))
    GPRMask |= (((1 << ((RF.StackAdjust() & 0x3) + 1)) - 1)
                << (~RF.StackAdjust() & 0x3));

  return {GPRMask, VFPMask};
}
} // namespace WinEH
} // namespace ARM
} // namespace vm::core

