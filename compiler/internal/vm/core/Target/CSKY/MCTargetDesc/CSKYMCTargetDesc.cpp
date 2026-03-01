//===-- CSKYMCTargetDesc.cpp - CSKY Target Descriptions -------------------===//
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
/// This file provides CSKY specific target descriptions.
///
//===----------------------------------------------------------------------===//

#include "CSKYMCTargetDesc.h"
#include "CSKYAsmBackend.h"
#include "CSKYELFStreamer.h"
#include "CSKYInstPrinter.h"
#include "CSKYMCAsmInfo.h"
#include "CSKYTargetStreamer.h"
#include "TargetInfo/CSKYTargetInfo.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCInstrAnalysis.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "CSKYGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "CSKYGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "CSKYGenSubtargetInfo.inc"

using namespace vm::core;

static MCAsmInfo *createCSKYMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new CSKYMCAsmInfo(TT);

  // Initial state of the frame pointer is SP.
  unsigned Reg = MRI.getDwarfRegNum(CSKY::R14, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createCSKYMCInstrInfo() {
  MCInstrInfo *Info = new MCInstrInfo();
  InitCSKYMCInstrInfo(Info);
  return Info;
}

static MCInstPrinter *createCSKYMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new CSKYInstPrinter(MAI, MII, MRI);
}

static MCRegisterInfo *createCSKYMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *Info = new MCRegisterInfo();
  InitCSKYMCRegisterInfo(Info, CSKY::R15);
  return Info;
}

static MCSubtargetInfo *createCSKYMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";
  return createCSKYMCSubtargetInfoImpl(TT, CPUName, /*TuneCPU=*/CPUName, FS);
}

static MCTargetStreamer *
createCSKYObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new CSKYTargetELFStreamer(S, STI);
  return nullptr;
}

static MCStreamer *createELFStreamer(const Triple &T, MCContext &Ctx,
                                     std::unique_ptr<MCAsmBackend> &&MAB,
                                     std::unique_ptr<MCObjectWriter> &&OW,
                                     std::unique_ptr<MCCodeEmitter> &&Emitter) {
  CSKYELFStreamer *S = new CSKYELFStreamer(Ctx, std::move(MAB), std::move(OW),
                                           std::move(Emitter));

  return S;
}

static MCTargetStreamer *
createCSKYAsmTargetStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                            MCInstPrinter *InstPrinter) {
  return new CSKYTargetAsmStreamer(S, OS);
}

static MCTargetStreamer *createCSKYNullTargetStreamer(MCStreamer &S) {
  return new CSKYTargetStreamer(S);
}

namespace {

class CSKYMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit CSKYMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override {
    if (isConditionalBranch(Inst) || isUnconditionalBranch(Inst)) {
      int64_t Imm;
      Imm = Inst.getOperand(Inst.getNumOperands() - 1).getImm();
      Target = Addr + Imm;
      return true;
    }

    if (Inst.getOpcode() == CSKY::BSR32) {
      Target = Addr + Inst.getOperand(0).getImm();
      return true;
    }

    switch (Inst.getOpcode()) {
    default:
      return false;
    case CSKY::LRW16:
    case CSKY::LRW32:
    case CSKY::JSRI32:
    case CSKY::JMPI32:
      int64_t Imm = Inst.getOperand(Inst.getNumOperands() - 1).getImm();
      Target = ((Addr + Imm) & 0xFFFFFFFC);
      return true;
    }

    return false;
  }
};

} // end anonymous namespace

static MCInstrAnalysis *createCSKYInstrAnalysis(const MCInstrInfo *Info) {
  return new CSKYMCInstrAnalysis(Info);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCSKYTargetMC() {
  auto &CSKYTarget = getTheCSKYTarget();
  TargetRegistry::RegisterMCAsmBackend(CSKYTarget, createCSKYAsmBackend);
  TargetRegistry::RegisterMCAsmInfo(CSKYTarget, createCSKYMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(CSKYTarget, createCSKYMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(CSKYTarget, createCSKYMCRegisterInfo);
  TargetRegistry::RegisterMCCodeEmitter(CSKYTarget, createCSKYMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(CSKYTarget, createCSKYMCInstPrinter);
  TargetRegistry::RegisterMCSubtargetInfo(CSKYTarget,
                                          createCSKYMCSubtargetInfo);
  TargetRegistry::RegisterELFStreamer(CSKYTarget, createELFStreamer);
  TargetRegistry::RegisterObjectTargetStreamer(CSKYTarget,
                                               createCSKYObjectTargetStreamer);
  TargetRegistry::RegisterAsmTargetStreamer(CSKYTarget,
                                            createCSKYAsmTargetStreamer);
  // Register the null target streamer.
  TargetRegistry::RegisterNullTargetStreamer(CSKYTarget,
                                             createCSKYNullTargetStreamer);
  TargetRegistry::RegisterMCInstrAnalysis(CSKYTarget, createCSKYInstrAnalysis);
}
