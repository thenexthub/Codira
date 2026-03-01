//===- MachineSizeOpts.cpp - code size optimization related code ----------===//
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
// This file contains some shared machine IR code size optimization related
// code.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineSizeOpts.h"
#include "vm/core/CodeGen/MBFIWrapper.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"

using namespace vm::core;

extern cl::opt<bool> EnablePGSO;
extern cl::opt<bool> PGSOLargeWorkingSetSizeOnly;
extern cl::opt<bool> ForcePGSO;
extern cl::opt<int> PgsoCutoffInstrProf;
extern cl::opt<int> PgsoCutoffSampleProf;

bool toolchain::shouldOptimizeForSize(const MachineFunction *MF,
                                 ProfileSummaryInfo *PSI,
                                 const MachineBlockFrequencyInfo *MBFI,
                                 PGSOQueryType QueryType) {
  if (MF->getFunction().hasOptSize())
    return true;
  return shouldFuncOptimizeForSizeImpl(MF, PSI, MBFI, QueryType);
}

bool toolchain::shouldOptimizeForSize(const MachineBasicBlock *MBB,
                                 ProfileSummaryInfo *PSI,
                                 const MachineBlockFrequencyInfo *MBFI,
                                 PGSOQueryType QueryType) {
  assert(MBB);
  if (MBB->getParent()->getFunction().hasOptSize())
    return true;
  return shouldOptimizeForSizeImpl(MBB, PSI, MBFI, QueryType);
}

bool toolchain::shouldOptimizeForSize(const MachineBasicBlock *MBB,
                                 ProfileSummaryInfo *PSI,
                                 MBFIWrapper *MBFIW,
                                 PGSOQueryType QueryType) {
  assert(MBB);
  if (MBB->getParent()->getFunction().hasOptSize())
    return true;
  if (!MBFIW)
    return false;
  BlockFrequency BlockFreq = MBFIW->getBlockFreq(MBB);
  return shouldOptimizeForSizeImpl(BlockFreq, PSI, &MBFIW->getMBFI(),
                                   QueryType);
}
