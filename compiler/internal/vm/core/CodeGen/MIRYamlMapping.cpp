//===- MIRYamlMapping.cpp - Describe mapping between MIR and YAML ---------===//
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
// This file implements the mapping between various MIR data structures and
// their corresponding YAML representation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MIRYamlMapping.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/FormatVariadic.h"

using namespace vm::core;
using namespace vm::core::yaml;

FrameIndex::FrameIndex(int FI, const toolchain::MachineFrameInfo &MFI) {
  IsFixed = MFI.isFixedObjectIndex(FI);
  if (IsFixed)
    FI -= MFI.getObjectIndexBegin();
  this->FI = FI;
}

// Returns the value and if the frame index is fixed or not.
Expected<int> FrameIndex::getFI(const toolchain::MachineFrameInfo &MFI) const {
  int FI = this->FI;
  if (IsFixed) {
    if (unsigned(FI) >= MFI.getNumFixedObjects())
      return make_error<StringError>(
          formatv("invalid fixed frame index {0}", FI).str(),
          inconvertibleErrorCode());
    FI += MFI.getObjectIndexBegin();
  }
  if (unsigned(FI + MFI.getNumFixedObjects()) >= MFI.getNumObjects())
    return make_error<StringError>(formatv("invalid frame index {0}", FI).str(),
                                   inconvertibleErrorCode());
  return FI;
}
