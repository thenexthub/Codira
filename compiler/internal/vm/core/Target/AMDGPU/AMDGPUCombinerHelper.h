//=== lib/CodeGen/GlobalISel/AMDGPUCombinerHelper.h -------------*- C++ -*-===//
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
///
/// \file
/// This contains common combine transformations that may be used in a combine
/// pass.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUCOMBINERHELPER_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUCOMBINERHELPER_H

#include "GCNSubtarget.h"
#include "vm/core/CodeGen/GlobalISel/Combiner.h"
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"

namespace vm::core {
class AMDGPUCombinerHelper : public CombinerHelper {
protected:
  const GCNSubtarget &STI;
  const SIInstrInfo &TII;

public:
  using CombinerHelper::CombinerHelper;
  AMDGPUCombinerHelper(GISelChangeObserver &Observer, MachineIRBuilder &B,
                       bool IsPreLegalize, GISelValueTracking *VT,
                       MachineDominatorTree *MDT, const LegalizerInfo *LI,
                       const GCNSubtarget &STI);

  bool matchFoldableFneg(MachineInstr &MI, MachineInstr *&MatchInfo) const;
  void applyFoldableFneg(MachineInstr &MI, MachineInstr *&MatchInfo) const;

  bool matchExpandPromotedF16FMed3(MachineInstr &MI, Register Src0,
                                   Register Src1, Register Src2) const;
  void applyExpandPromotedF16FMed3(MachineInstr &MI, Register Src0,
                                   Register Src1, Register Src2) const;

  bool matchCombineFmulWithSelectToFldexp(
      MachineInstr &MI, MachineInstr &Sel,
      std::function<void(MachineIRBuilder &)> &MatchInfo) const;

  bool matchConstantIs32BitMask(Register Reg) const;
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUCOMBINERHELPER_H
