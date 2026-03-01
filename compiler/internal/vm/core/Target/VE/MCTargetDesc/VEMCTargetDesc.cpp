//===-- VEMCTargetDesc.cpp - VE Target Descriptions -----------------------===//
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
// This file provides VE specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "VEMCTargetDesc.h"
#include "TargetInfo/VETargetInfo.h"
#include "VEInstPrinter.h"
#include "VEMCAsmInfo.h"
#include "VETargetStreamer.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "VEGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "VEGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "VEGenRegisterInfo.inc"

static MCAsmInfo *createVEMCAsmInfo(const MCRegisterInfo &MRI, const Triple &TT,
                                    const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new VEELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(VE::SX11, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createVEMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitVEMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createVEMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitVEMCRegisterInfo(X, VE::SX10);
  return X;
}

static MCSubtargetInfo *createVEMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                                StringRef FS) {
  if (CPU.empty())
    CPU = "generic";
  return createVEMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCTargetStreamer *
createObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new VETargetELFStreamer(S);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint) {
  return new VETargetAsmStreamer(S, OS);
}

static MCTargetStreamer *createNullTargetStreamer(MCStreamer &S) {
  return new VETargetStreamer(S);
}

static MCInstPrinter *createVEMCInstPrinter(const Triple &T,
                                            unsigned SyntaxVariant,
                                            const MCAsmInfo &MAI,
                                            const MCInstrInfo &MII,
                                            const MCRegisterInfo &MRI) {
  return new VEInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeVETargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheVETarget(), createVEMCAsmInfo);

  for (Target *T : {&getTheVETarget()}) {
    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createVEMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createVEMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createVEMCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(*T, createVEMCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createVEAsmBackend);

    // Register the object target streamer.
    TargetRegistry::RegisterObjectTargetStreamer(*T,
                                                 createObjectTargetStreamer);

    // Register the asm streamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);

    // Register the null streamer.
    TargetRegistry::RegisterNullTargetStreamer(*T, createNullTargetStreamer);

    // Register the MCInstPrinter
    TargetRegistry::RegisterMCInstPrinter(*T, createVEMCInstPrinter);
  }
}
