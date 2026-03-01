//===- XtensaSubtarget.cpp - Xtensa Subtarget Information -----------------===//
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
// This file implements the Xtensa specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "XtensaSubtarget.h"
#include "XtensaSelectionDAGInfo.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "xtensa-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "XtensaGenSubtargetInfo.inc"

using namespace vm::core;

XtensaSubtarget &
XtensaSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  StringRef CPUName = CPU;
  if (CPUName.empty()) {
    // set default cpu name
    CPUName = "generic";
  }

  // Parse features string.
  ParseSubtargetFeatures(CPUName, CPUName, FS);
  return *this;
}

XtensaSubtarget::XtensaSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                                 const TargetMachine &TM)
    : XtensaGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS), TargetTriple(TT),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM, *this),
      FrameLowering(*this) {
  TSInfo = std::make_unique<SelectionDAGTargetInfo>();
}

XtensaSubtarget::~XtensaSubtarget() = default;

const SelectionDAGTargetInfo *XtensaSubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}
