//===-- TargetOptionsImpl.cpp - Options that apply to all targets ----------==//
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
// This file implements the methods in the TargetOptions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Target/TargetOptions.h"
using namespace vm::core;

/// DisableFramePointerElim - This returns true if frame pointer elimination
/// optimization should be disabled for the given machine function.
bool TargetOptions::DisableFramePointerElim(const MachineFunction &MF) const {
  const Function &F = MF.getFunction();

  Attribute FPAttr = F.getFnAttribute("frame-pointer");
  if (!FPAttr.isValid())
    return false;
  StringRef FP = FPAttr.getValueAsString();
  if (FP == "all")
    return true;
  if (FP == "non-leaf" || FP == "non-leaf-no-reserve")
    return MF.getFrameInfo().hasCalls();
  if (FP == "none" || FP == "reserved")
    return false;
  llvm_unreachable("unknown frame pointer flag");
}

bool TargetOptions::FramePointerIsReserved(const MachineFunction &MF) const {
  const Function &F = MF.getFunction();
  Attribute FPAttr = F.getFnAttribute("frame-pointer");
  if (!FPAttr.isValid())
    return false;

  return StringSwitch<bool>(FPAttr.getValueAsString())
      .Cases({"all", "non-leaf", "reserved"}, true)
      .Case(("non-leaf-no-reserve"), MF.getFrameInfo().hasCalls())
      .Case("none", false);
}

/// HonorSignDependentRoundingFPMath - Return true if the codegen must assume
/// that the rounding mode of the FPU can change from its default.
bool TargetOptions::HonorSignDependentRoundingFPMath() const {
  return HonorSignDependentRoundingFPMathOption;
}

/// NOTE: There are targets that still do not support the debug entry values
/// production and that is being controlled with the SupportsDebugEntryValues.
/// In addition, SCE debugger does not have the feature implemented, so prefer
/// not to emit the debug entry values in that case.
/// The EnableDebugEntryValues can be used for the testing purposes.
bool TargetOptions::ShouldEmitDebugEntryValues() const {
  return (SupportsDebugEntryValues && DebuggerTuning != DebuggerKind::SCE) ||
         EnableDebugEntryValues;
}
