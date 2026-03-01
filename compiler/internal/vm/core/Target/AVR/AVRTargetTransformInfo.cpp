//===-- AVRTargetTransformInfo.cpp - AVR specific TTI ---------------------===//
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

#include "AVRTargetTransformInfo.h"

using namespace vm::core;

bool AVRTTIImpl::isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                               const TargetTransformInfo::LSRCost &C2) const {
  // AVR specific here are "instruction number 1st priority".
  // If we need to emit adds inside the loop to add up base registers, then
  // we need at least one extra temporary register.
  unsigned C1NumRegs = C1.NumRegs + (C1.NumBaseAdds != 0);
  unsigned C2NumRegs = C2.NumRegs + (C2.NumBaseAdds != 0);
  return std::tie(C1.Insns, C1NumRegs, C1.AddRecCost, C1.NumIVMuls,
                  C1.NumBaseAdds, C1.ScaleCost, C1.ImmCost, C1.SetupCost) <
         std::tie(C2.Insns, C2NumRegs, C2.AddRecCost, C2.NumIVMuls,
                  C2.NumBaseAdds, C2.ScaleCost, C2.ImmCost, C2.SetupCost);
}
