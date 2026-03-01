//===-- NVPTXMCTargetDesc.cpp - NVPTX Target Descriptions -------*- C++ -*-===//
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
// This file provides NVPTX specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "NVPTXMCTargetDesc.h"
#include "NVPTXInstPrinter.h"
#include "NVPTXMCAsmInfo.h"
#include "NVPTXTargetStreamer.h"
#include "TargetInfo/NVPTXTargetInfo.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "NVPTXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "NVPTXGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "NVPTXGenRegisterInfo.inc"

static MCInstrInfo *createNVPTXMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitNVPTXMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createNVPTXMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  // PTX does not have a return address register.
  InitNVPTXMCRegisterInfo(X, 0);
  return X;
}

static MCSubtargetInfo *
createNVPTXMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createNVPTXMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createNVPTXMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new NVPTXInstPrinter(MAI, MII, MRI);
  return nullptr;
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &,
                                                 MCInstPrinter *) {
  return new NVPTXAsmTargetStreamer(S);
}

static MCTargetStreamer *createNullTargetStreamer(MCStreamer &S) {
  return new NVPTXTargetStreamer(S);
}

// Force static initialization.
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeNVPTXTargetMC() {
  for (Target *T : {&getTheNVPTXTarget32(), &getTheNVPTXTarget64()}) {
    // Register the MC asm info.
    RegisterMCAsmInfo<NVPTXMCAsmInfo> X(*T);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createNVPTXMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createNVPTXMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createNVPTXMCSubtargetInfo);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createNVPTXMCInstPrinter);

    // Register the MCTargetStreamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);

    // Register the MCTargetStreamer.
    TargetRegistry::RegisterNullTargetStreamer(*T, createNullTargetStreamer);
  }
}
