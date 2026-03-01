//===-- AVRSubtarget.cpp - AVR Subtarget Information ----------------------===//
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
// This file implements the AVR specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "AVRSubtarget.h"

#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/TargetRegistry.h"

#include "AVR.h"
#include "AVRTargetMachine.h"

#define DEBUG_TYPE "avr-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "AVRGenSubtargetInfo.inc"

namespace vm::core {

AVRSubtarget::AVRSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const AVRTargetMachine &TM)
    : AVRGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), InstrInfo(*this),
      TLInfo(TM, initializeSubtargetDependencies(CPU, FS, TM)) {
  // Parse features string.
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
}

AVRSubtarget &
AVRSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                              const TargetMachine &TM) {
  // Parse features string.
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
  return *this;
}

} // end of namespace vm::core
