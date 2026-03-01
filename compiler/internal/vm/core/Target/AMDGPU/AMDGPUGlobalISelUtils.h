//===- AMDGPUGlobalISelUtils -------------------------------------*- C++ -*-==//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUGLOBALISELUTILS_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUGLOBALISELUTILS_H

#include "vm/core/ADT/DenseSet.h"
#include "vm/core/CodeGen/Register.h"
#include <utility>

namespace vm::core {

class MachineRegisterInfo;
class GCNSubtarget;
class GISelValueTracking;
class LLT;
class MachineFunction;
class MachineIRBuilder;
class RegisterBankInfo;

namespace AMDGPU {

/// Returns base register and constant offset.
std::pair<Register, unsigned>
getBaseWithConstantOffset(MachineRegisterInfo &MRI, Register Reg,
                          GISelValueTracking *ValueTracking = nullptr,
                          bool CheckNUW = false);

// Currently finds S32/S64 lane masks that can be declared as divergent by
// uniformity analysis (all are phis at the moment).
// These are defined as i32/i64 in some IR intrinsics (not as i1).
// Tablegen forces(via telling that lane mask IR intrinsics are uniform) most of
// S32/S64 lane masks to be uniform, as this results in them ending up with sgpr
// reg class after instruction-select, don't search for all of them.
class IntrinsicLaneMaskAnalyzer {
  SmallDenseSet<Register, 8> S32S64LaneMask;
  MachineRegisterInfo &MRI;

public:
  IntrinsicLaneMaskAnalyzer(MachineFunction &MF);
  bool isS32S64LaneMask(Register Reg) const;

private:
  void initLaneMaskIntrinsics(MachineFunction &MF);
};

void buildReadAnyLane(MachineIRBuilder &B, Register SgprDst, Register VgprSrc,
                      const RegisterBankInfo &RBI);
void buildReadFirstLane(MachineIRBuilder &B, Register SgprDst, Register VgprSrc,
                        const RegisterBankInfo &RBI);
}
}

#endif
