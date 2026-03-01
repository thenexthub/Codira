//===-- SPIRVMCTargetDesc.cpp - SPIR-V Target Descriptions ----*- C++ -*---===//
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
// This file provides SPIR-V specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "SPIRVMCTargetDesc.h"
#include "SPIRVInstPrinter.h"
#include "SPIRVMCAsmInfo.h"
#include "SPIRVTargetStreamer.h"
#include "TargetInfo/SPIRVTargetInfo.h"
#include "vm/core/MC/MCInstrAnalysis.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "SPIRVGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SPIRVGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SPIRVGenRegisterInfo.inc"

using namespace vm::core;

static MCInstrInfo *createSPIRVMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSPIRVMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSPIRVMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  return X;
}

static MCSubtargetInfo *
createSPIRVMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createSPIRVMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &,
                                                 MCInstPrinter *) {
  return new SPIRVTargetStreamer(S);
}

static MCInstPrinter *createSPIRVMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  assert(SyntaxVariant == 0);
  return new SPIRVInstPrinter(MAI, MII, MRI);
}

namespace {

class SPIRVMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit SPIRVMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}
};

} // end anonymous namespace

static MCInstrAnalysis *createSPIRVInstrAnalysis(const MCInstrInfo *Info) {
  return new SPIRVMCInstrAnalysis(Info);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSPIRVTargetMC() {
  for (Target *T : {&getTheSPIRV32Target(), &getTheSPIRV64Target(),
                    &getTheSPIRVLogicalTarget()}) {
    RegisterMCAsmInfo<SPIRVMCAsmInfo> X(*T);
    TargetRegistry::RegisterMCInstrInfo(*T, createSPIRVMCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createSPIRVMCRegisterInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createSPIRVMCSubtargetInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createSPIRVMCInstPrinter);
    TargetRegistry::RegisterMCInstrAnalysis(*T, createSPIRVInstrAnalysis);
    TargetRegistry::RegisterMCCodeEmitter(*T, createSPIRVMCCodeEmitter);
    TargetRegistry::RegisterMCAsmBackend(*T, createSPIRVAsmBackend);
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);
  }
}
