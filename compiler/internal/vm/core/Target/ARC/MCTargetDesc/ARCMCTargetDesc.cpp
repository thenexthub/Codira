//===- ARCMCTargetDesc.cpp - ARC Target Descriptions ------------*- C++ -*-===//
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
// This file provides ARC specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "ARCMCTargetDesc.h"
#include "ARCInstPrinter.h"
#include "ARCMCAsmInfo.h"
#include "ARCTargetStreamer.h"
#include "TargetInfo/ARCTargetInfo.h"
#include "vm/core/MC/MCDwarf.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/FormattedStream.h"

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "ARCGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "ARCGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "ARCGenRegisterInfo.inc"

static MCInstrInfo *createARCMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitARCMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createARCMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitARCMCRegisterInfo(X, ARC::BLINK);
  return X;
}

static MCSubtargetInfo *createARCMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createARCMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCAsmInfo *createARCMCAsmInfo(const MCRegisterInfo &MRI,
                                     const Triple &TT,
                                     const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new ARCMCAsmInfo(TT);

  // Initial state of the frame pointer is SP.
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, ARC::SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createARCMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  return new ARCInstPrinter(MAI, MII, MRI);
}

ARCTargetStreamer::ARCTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}
ARCTargetStreamer::~ARCTargetStreamer() = default;

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint) {
  return new ARCTargetStreamer(S);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeARCTargetMC() {
  // Register the MC asm info.
  Target &TheARCTarget = getTheARCTarget();
  RegisterMCAsmInfoFn X(TheARCTarget, createARCMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheARCTarget, createARCMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheARCTarget, createARCMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheARCTarget,
                                          createARCMCSubtargetInfo);

  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheARCTarget, createARCMCInstPrinter);

  TargetRegistry::RegisterAsmTargetStreamer(TheARCTarget,
                                            createTargetAsmStreamer);
}
