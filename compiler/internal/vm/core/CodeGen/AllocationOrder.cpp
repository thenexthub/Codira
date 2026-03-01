//===-- toolchain/CodeGen/AllocationOrder.cpp - Allocation Order ---------------===//
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
// This file implements an allocation order for virtual registers.
//
// The preferred allocation order for a virtual register depends on allocation
// hints and target hooks. The AllocationOrder class encapsulates all of that.
//
//===----------------------------------------------------------------------===//

#include "AllocationOrder.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/RegisterClassInfo.h"
#include "vm/core/CodeGen/VirtRegMap.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "regalloc"

// Compare VirtRegMap::getRegAllocPref().
AllocationOrder AllocationOrder::create(Register VirtReg, const VirtRegMap &VRM,
                                        const RegisterClassInfo &RegClassInfo,
                                        const LiveRegMatrix *Matrix) {
  const MachineFunction &MF = VRM.getMachineFunction();
  const TargetRegisterInfo *TRI = &VRM.getTargetRegInfo();
  auto Order = RegClassInfo.getOrder(MF.getRegInfo().getRegClass(VirtReg));
  SmallVector<MCPhysReg, 16> Hints;
  bool HardHints =
      TRI->getRegAllocationHints(VirtReg, Order, Hints, MF, &VRM, Matrix);

  LLVM_DEBUG({
    if (!Hints.empty()) {
      dbgs() << "hints:";
      for (MCPhysReg Hint : Hints)
        dbgs() << ' ' << printReg(Hint, TRI);
      dbgs() << '\n';
    }
  });
  assert(all_of(Hints,
                [&](MCPhysReg Hint) { return is_contained(Order, Hint); }) &&
         "Target hint is outside allocation order.");
  return AllocationOrder(std::move(Hints), Order, HardHints);
}
