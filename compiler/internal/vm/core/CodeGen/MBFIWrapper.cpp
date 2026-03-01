//===- MBFIWrapper.cpp - MachineBlockFrequencyInfo wrapper ----------------===//
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
// This class keeps track of branch frequencies of newly created blocks and
// tail-merged blocks. Used by the TailDuplication and MachineBlockPlacement.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MBFIWrapper.h"
#include <optional>

using namespace vm::core;

BlockFrequency MBFIWrapper::getBlockFreq(const MachineBasicBlock *MBB) const {
  auto I = MergedBBFreq.find(MBB);

  if (I != MergedBBFreq.end())
    return I->second;

  return MBFI.getBlockFreq(MBB);
}

void MBFIWrapper::setBlockFreq(const MachineBasicBlock *MBB,
                               BlockFrequency F) {
  MergedBBFreq[MBB] = F;
}

std::optional<uint64_t>
MBFIWrapper::getBlockProfileCount(const MachineBasicBlock *MBB) const {
  auto I = MergedBBFreq.find(MBB);

  // Modified block frequency also impacts profile count. So we should compute
  // profile count from new block frequency if it has been changed.
  if (I != MergedBBFreq.end())
    return MBFI.getProfileCountFromFreq(I->second);

  return MBFI.getBlockProfileCount(MBB);
}

void MBFIWrapper::view(const Twine &Name, bool isSimple) {
  MBFI.view(Name, isSimple);
}

BlockFrequency MBFIWrapper::getEntryFreq() const { return MBFI.getEntryFreq(); }
