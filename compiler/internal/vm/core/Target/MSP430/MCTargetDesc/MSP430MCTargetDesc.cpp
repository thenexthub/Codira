//===-- MSP430MCTargetDesc.cpp - MSP430 Target Descriptions ---------------===//
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
// This file provides MSP430 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "MSP430MCTargetDesc.h"
#include "MSP430InstPrinter.h"
#include "MSP430MCAsmInfo.h"
#include "TargetInfo/MSP430TargetInfo.h"
#include "vm/core/MC/MCDwarf.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "MSP430GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MSP430GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MSP430GenRegisterInfo.inc"

static MCInstrInfo *createMSP430MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMSP430MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMSP430MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMSP430MCRegisterInfo(X, MSP430::PC);
  return X;
}

static MCAsmInfo *createMSP430MCAsmInfo(const MCRegisterInfo &MRI,
                                        const Triple &TT,
                                        const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new MSP430MCAsmInfo(TT);

  // Initialize initial frame state.
  int stackGrowth = -2;

  // Initial state of the frame pointer is sp+ptr_size.
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(
      nullptr, MRI.getDwarfRegNum(MSP430::SP, true), -stackGrowth);
  MAI->addInitialFrameState(Inst);

  // Add return address to move list
  MCCFIInstruction Inst2 = MCCFIInstruction::createOffset(
      nullptr, MRI.getDwarfRegNum(MSP430::PC, true), stackGrowth);
  MAI->addInitialFrameState(Inst2);

  return MAI;
}

static MCSubtargetInfo *
createMSP430MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createMSP430MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createMSP430MCInstPrinter(const Triple &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new MSP430InstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeMSP430TargetMC() {
  Target &T = getTheMSP430Target();

  TargetRegistry::RegisterMCAsmInfo(T, createMSP430MCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(T, createMSP430MCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createMSP430MCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createMSP430MCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createMSP430MCInstPrinter);
  TargetRegistry::RegisterMCCodeEmitter(T, createMSP430MCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(T, createMSP430MCAsmBackend);
  TargetRegistry::RegisterObjectTargetStreamer(
      T, createMSP430ObjectTargetStreamer);
}
