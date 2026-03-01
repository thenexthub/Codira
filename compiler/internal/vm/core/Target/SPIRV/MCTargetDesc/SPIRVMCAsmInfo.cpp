//===-- SPIRVMCAsmInfo.h - SPIR-V asm properties --------------*- C++ -*--====//
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
// This file contains the declarations of the SPIRVMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "SPIRVMCAsmInfo.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

SPIRVMCAsmInfo::SPIRVMCAsmInfo(const Triple &TT,
                               const MCTargetOptions &Options) {
  IsLittleEndian = true;

  HasSingleParameterDotFile = false;
  HasDotTypeDotSizeDirective = false;

  MinInstAlignment = 4;

  CodePointerSize = 4;
  CommentString = ";";
  HasFunctionAlignment = false;
}

bool SPIRVMCAsmInfo::shouldOmitSectionDirective(StringRef SectionName) const {
  return true;
}
