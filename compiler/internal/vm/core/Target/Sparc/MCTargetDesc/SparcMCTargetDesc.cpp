//===-- SparcMCTargetDesc.cpp - Sparc Target Descriptions -----------------===//
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
// This file provides Sparc specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "SparcMCTargetDesc.h"
#include "SparcInstPrinter.h"
#include "SparcMCAsmInfo.h"
#include "SparcTargetStreamer.h"
#include "TargetInfo/SparcTargetInfo.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {
namespace SparcASITag {
#define GET_ASITagsList_IMPL
#include "SparcGenSearchableTables.inc"
} // end namespace SparcASITag

namespace SparcPrefetchTag {
#define GET_PrefetchTagsList_IMPL
#include "SparcGenSearchableTables.inc"
} // end namespace SparcPrefetchTag
} // end namespace vm::core

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "SparcGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SparcGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SparcGenRegisterInfo.inc"

static MCAsmInfo *createSparcMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new SparcELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCAsmInfo *createSparcV9MCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT,
                                         const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new SparcELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, Reg, 2047);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createSparcMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSparcMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSparcMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitSparcMCRegisterInfo(X, SP::O7);
  return X;
}

static MCSubtargetInfo *
createSparcMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = TT.getArch() == Triple::sparcv9 ? "v9" : "v8";

  MCSubtargetInfo *STI =
      createSparcMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
  if (TT.isSPARC64() && !STI->hasFeature(Sparc::Feature64Bit)) {
    FeatureBitset Features = STI->getFeatureBits();
    STI->setFeatureBits(Features.set(Sparc::Feature64Bit));
  }

  return STI;
}

static MCTargetStreamer *
createObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new SparcTargetELFStreamer(S, STI);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint) {
  return new SparcTargetAsmStreamer(S, OS);
}

static MCTargetStreamer *createNullTargetStreamer(MCStreamer &S) {
  return new SparcTargetStreamer(S);
}

static MCInstPrinter *createSparcMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new SparcInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSparcTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheSparcTarget(), createSparcMCAsmInfo);
  RegisterMCAsmInfoFn Y(getTheSparcV9Target(), createSparcV9MCAsmInfo);
  RegisterMCAsmInfoFn Z(getTheSparcelTarget(), createSparcMCAsmInfo);

  for (Target *T :
       {&getTheSparcTarget(), &getTheSparcV9Target(), &getTheSparcelTarget()}) {
    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createSparcMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createSparcMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createSparcMCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(*T, createSparcMCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createSparcAsmBackend);

    // Register the object target streamer.
    TargetRegistry::RegisterObjectTargetStreamer(*T,
                                                 createObjectTargetStreamer);

    // Register the asm streamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);

    // Register the null streamer.
    TargetRegistry::RegisterNullTargetStreamer(*T, createNullTargetStreamer);

    // Register the MCInstPrinter
    TargetRegistry::RegisterMCInstPrinter(*T, createSparcMCInstPrinter);
  }
}
