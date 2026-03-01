//===-- HexagonMask.cpp - replace const ext tfri with mask ------===//
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
//===----------------------------------------------------------------------===//

#include "Hexagon.h"
#include "HexagonSubtarget.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Support/MathExtras.h"
#include "vm/core/Target/TargetMachine.h"

#define DEBUG_TYPE "mask"

using namespace vm::core;

namespace {
class HexagonMask : public MachineFunctionPass {
public:
  static char ID;
  HexagonMask() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "Hexagon replace const ext tfri with mask";
  }
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const HexagonInstrInfo *HII;
  void replaceConstExtTransferImmWithMask(MachineFunction &MF);
};
} // end anonymous namespace

char HexagonMask::ID = 0;

void HexagonMask::replaceConstExtTransferImmWithMask(MachineFunction &MF) {
  for (auto &MBB : MF) {
    for (auto &MI : toolchain::make_early_inc_range(MBB)) {
      if (MI.getOpcode() != Hexagon::A2_tfrsi)
        continue;

      const MachineOperand &Op0 = MI.getOperand(0);
      const MachineOperand &Op1 = MI.getOperand(1);
      if (!Op1.isImm())
        continue;
      int32_t V = Op1.getImm();
      if (isInt<16>(V))
        continue;

      unsigned Idx, Len;
      if (!isShiftedMask_32(V, Idx, Len))
        continue;
      if (!isUInt<5>(Idx) || !isUInt<5>(Len))
        continue;

      BuildMI(MBB, MI, MI.getDebugLoc(), HII->get(Hexagon::S2_mask),
              Op0.getReg())
          .addImm(Len)
          .addImm(Idx);
      MBB.erase(MI);
    }
  }
}

bool HexagonMask::runOnMachineFunction(MachineFunction &MF) {
  auto &HST = MF.getSubtarget<HexagonSubtarget>();
  HII = HST.getInstrInfo();
  const Function &F = MF.getFunction();

  if (!F.hasOptSize())
    return false;
  // Mask instruction is available only from v66
  if (!HST.hasV66Ops())
    return false;
  // The mask instruction available in v66 can be used to generate values in
  // registers using 2 immediates Eg. to form 0x07fffffc in R0, you would write
  // "R0 = mask(#25,#2)" Since it is a single-word instruction, it takes less
  // code size than a constant-extended transfer at Os
  replaceConstExtTransferImmWithMask(MF);

  return true;
}

//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

INITIALIZE_PASS(HexagonMask, "hexagon-mask", "Hexagon mask", false, false)

FunctionPass *toolchain::createHexagonMask() { return new HexagonMask(); }
