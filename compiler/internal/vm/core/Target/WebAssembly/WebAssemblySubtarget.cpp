//===-- WebAssemblySubtarget.cpp - WebAssembly Subtarget Information ------===//
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
///
/// \file
/// This file implements the WebAssembly-specific subclass of
/// TargetSubtarget.
///
//===----------------------------------------------------------------------===//

#include "WebAssemblySubtarget.h"
#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "WebAssemblyInstrInfo.h"
#include "vm/core/MC/TargetRegistry.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-subtarget"

#define GET_SUBTARGETINFO_CTOR
#define GET_SUBTARGETINFO_TARGET_DESC
#include "WebAssemblyGenSubtargetInfo.inc"

WebAssemblySubtarget &
WebAssemblySubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                      StringRef FS) {
  // Determine default and user-specified characteristics
  LLVM_DEBUG(toolchain::dbgs() << "initializeSubtargetDependencies\n");

  if (CPU.empty())
    CPU = "generic";

  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);

  FeatureBitset Bits = getFeatureBits();

  // bulk-memory implies bulk-memory-opt
  if (HasBulkMemory) {
    HasBulkMemoryOpt = true;
    Bits.set(WebAssembly::FeatureBulkMemoryOpt);
  }

  // gc implies reference-types
  if (HasGC) {
    HasReferenceTypes = true;
  }

  // reference-types implies call-indirect-overlong
  if (HasReferenceTypes) {
    HasCallIndirectOverlong = true;
    Bits.set(WebAssembly::FeatureCallIndirectOverlong);
  }

  // In case we changed any bits, update `MCSubtargetInfo`'s `FeatureBitset`.
  setFeatureBits(Bits);

  return *this;
}

WebAssemblySubtarget::WebAssemblySubtarget(const Triple &TT,
                                           const std::string &CPU,
                                           const std::string &FS,
                                           const TargetMachine &TM)
    : WebAssemblyGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      TargetTriple(TT), InstrInfo(initializeSubtargetDependencies(CPU, FS)),
      TLInfo(TM, *this) {}

bool WebAssemblySubtarget::enableAtomicExpand() const {
  // If atomics are disabled, atomic ops are lowered instead of expanded
  return hasAtomics();
}

bool WebAssemblySubtarget::enableMachineScheduler() const {
  // Disable the MachineScheduler for now. Even with ShouldTrackPressure set and
  // enableMachineSchedDefaultSched overridden, it appears to have an overall
  // negative effect for the kinds of register optimizations we're doing.
  return false;
}

bool WebAssemblySubtarget::useAA() const { return true; }
