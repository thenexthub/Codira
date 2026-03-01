//===-- AVRMCTargetDesc.cpp - AVR Target Descriptions ---------------------===//
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
// This file provides AVR specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "AVRMCTargetDesc.h"
#include "AVRELFStreamer.h"
#include "AVRInstPrinter.h"
#include "AVRMCAsmInfo.h"
#include "AVRTargetStreamer.h"
#include "TargetInfo/AVRTargetInfo.h"

#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "AVRGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "AVRGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "AVRGenRegisterInfo.inc"

using namespace vm::core;

MCInstrInfo *toolchain::createAVRMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitAVRMCInstrInfo(X);

  return X;
}

static MCRegisterInfo *createAVRMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitAVRMCRegisterInfo(X, 0);

  return X;
}

static MCSubtargetInfo *createAVRMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createAVRMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createAVRMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0) {
    return new AVRInstPrinter(MAI, MII, MRI);
  }

  return nullptr;
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter));
}

static MCTargetStreamer *
createAVRObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new AVRELFStreamer(S, STI);
}

static MCTargetStreamer *createMCAsmTargetStreamer(MCStreamer &S,
                                                   formatted_raw_ostream &OS,
                                                   MCInstPrinter *InstPrint) {
  return new AVRTargetAsmStreamer(S);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeAVRTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<AVRMCAsmInfo> X(getTheAVRTarget());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheAVRTarget(), createAVRMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheAVRTarget(), createAVRMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheAVRTarget(),
                                          createAVRMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheAVRTarget(),
                                        createAVRMCInstPrinter);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheAVRTarget(),
                                        createAVRMCCodeEmitter);

  // Register the obj streamer
  TargetRegistry::RegisterELFStreamer(getTheAVRTarget(), createMCStreamer);

  // Register the obj target streamer.
  TargetRegistry::RegisterObjectTargetStreamer(getTheAVRTarget(),
                                               createAVRObjectTargetStreamer);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheAVRTarget(),
                                            createMCAsmTargetStreamer);

  // Register the asm backend (as little endian).
  TargetRegistry::RegisterMCAsmBackend(getTheAVRTarget(), createAVRAsmBackend);
}
