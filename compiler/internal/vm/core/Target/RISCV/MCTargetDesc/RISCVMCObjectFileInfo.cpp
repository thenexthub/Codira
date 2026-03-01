//===-- RISCVMCObjectFileInfo.cpp - RISC-V object file properties ---------===//
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
// This file contains the declarations of the RISCVMCObjectFileInfo properties.
//
//===----------------------------------------------------------------------===//

#include "RISCVMCObjectFileInfo.h"
#include "RISCVMCTargetDesc.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSubtargetInfo.h"

using namespace vm::core;

unsigned
RISCVMCObjectFileInfo::getTextSectionAlignment(const MCSubtargetInfo &STI) {
  return STI.hasFeature(RISCV::FeatureStdExtZca) ? 2 : 4;
}

unsigned RISCVMCObjectFileInfo::getTextSectionAlignment() const {
  return getTextSectionAlignment(*getContext().getSubtargetInfo());
}
