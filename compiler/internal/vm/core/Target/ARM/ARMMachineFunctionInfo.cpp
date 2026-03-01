//===-- ARMMachineFunctionInfo.cpp - ARM machine function info ------------===//
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

#include "ARMMachineFunctionInfo.h"
#include "ARMSubtarget.h"

using namespace vm::core;

void ARMFunctionInfo::anchor() {}

yaml::ARMFunctionInfo::ARMFunctionInfo(const toolchain::ARMFunctionInfo &MFI)
    : LRSpilled(MFI.isLRSpilled()) {}

void yaml::ARMFunctionInfo::mappingImpl(yaml::IO &YamlIO) {
  MappingTraits<ARMFunctionInfo>::mapping(YamlIO, *this);
}

void ARMFunctionInfo::initializeBaseYamlFields(
    const yaml::ARMFunctionInfo &YamlMFI) {
  LRSpilled = YamlMFI.LRSpilled;
}

static bool GetBranchTargetEnforcement(const Function &F,
                                       const ARMSubtarget *Subtarget) {
  if (!Subtarget->isMClass() || !Subtarget->hasV7Ops())
    return false;

  return F.hasFnAttribute("branch-target-enforcement");
}

// The pair returns values for the ARMFunctionInfo members
// SignReturnAddress and SignReturnAddressAll respectively.
static std::pair<bool, bool> GetSignReturnAddress(const Function &F) {
  if (!F.hasFnAttribute("sign-return-address")) {
    return {false, false};
  }

  StringRef Scope = F.getFnAttribute("sign-return-address").getValueAsString();
  if (Scope == "none")
    return {false, false};

  if (Scope == "all")
    return {true, true};

  assert(Scope == "non-leaf");
  return {true, false};
}

ARMFunctionInfo::ARMFunctionInfo(const Function &F,
                                 const ARMSubtarget *Subtarget)
    : isThumb(Subtarget->isThumb()), hasThumb2(Subtarget->hasThumb2()),
      IsCmseNSEntry(F.hasFnAttribute("cmse_nonsecure_entry")),
      IsCmseNSCall(F.hasFnAttribute("cmse_nonsecure_call")),
      BranchTargetEnforcement(GetBranchTargetEnforcement(F, Subtarget)) {
  if (Subtarget->isMClass() && Subtarget->hasV7Ops())
    std::tie(SignReturnAddress, SignReturnAddressAll) = GetSignReturnAddress(F);
}

MachineFunctionInfo *
ARMFunctionInfo::clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
                       const DenseMap<MachineBasicBlock *, MachineBasicBlock *>
                           &Src2DstMBB) const {
  return DestMF.cloneInfo<ARMFunctionInfo>(*this);
}
