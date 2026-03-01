//===- lib/MC/MCInstrInfo.cpp - Target Instruction Info -------------------===//
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

#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCSubtargetInfo.h"

using namespace vm::core;

bool MCInstrInfo::getDeprecatedInfo(MCInst &MI, const MCSubtargetInfo &STI,
                                    std::string &Info) const {
  unsigned Opcode = MI.getOpcode();
  if (ComplexDeprecationInfos && ComplexDeprecationInfos[Opcode])
    return ComplexDeprecationInfos[Opcode](MI, STI, Info);
  if (DeprecatedFeatures && DeprecatedFeatures[Opcode] != uint8_t(-1U) &&
      STI.getFeatureBits()[DeprecatedFeatures[Opcode]]) {
    // FIXME: it would be nice to include the subtarget feature here.
    Info = "deprecated";
    return true;
  }
  return false;
}
