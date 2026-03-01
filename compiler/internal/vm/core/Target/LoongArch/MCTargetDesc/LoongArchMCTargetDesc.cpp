//===-- LoongArchMCTargetDesc.cpp - LoongArch Target Descriptions ---------===//
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
// This file provides LoongArch specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "LoongArchMCTargetDesc.h"
#include "LoongArchELFStreamer.h"
#include "LoongArchInstPrinter.h"
#include "LoongArchMCAsmInfo.h"
#include "TargetInfo/LoongArchTargetInfo.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCDwarf.h"
#include "vm/core/MC/MCInstrAnalysis.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include <bitset>

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "LoongArchGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LoongArchGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LoongArchGenSubtargetInfo.inc"

using namespace vm::core;

static MCRegisterInfo *createLoongArchMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitLoongArchMCRegisterInfo(X, LoongArch::R1);
  return X;
}

static MCInstrInfo *createLoongArchMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitLoongArchMCInstrInfo(X);
  return X;
}

static MCSubtargetInfo *
createLoongArchMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (CPU.empty() || CPU == "generic")
    CPU = TT.isArch64Bit() ? "generic-la64" : "generic-la32";
  return createLoongArchMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCAsmInfo *createLoongArchMCAsmInfo(const MCRegisterInfo &MRI,
                                           const Triple &TT,
                                           const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new LoongArchMCAsmInfo(TT);

  // Initial state of the frame pointer is sp(r3).
  unsigned SP = MRI.getDwarfRegNum(LoongArch::R3, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createLoongArchMCInstPrinter(const Triple &T,
                                                   unsigned SyntaxVariant,
                                                   const MCAsmInfo &MAI,
                                                   const MCInstrInfo &MII,
                                                   const MCRegisterInfo &MRI) {
  return new LoongArchInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *
createLoongArchObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return STI.getTargetTriple().isOSBinFormatELF()
             ? new LoongArchTargetELFStreamer(S, STI)
             : nullptr;
}

static MCTargetStreamer *
createLoongArchAsmTargetStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                                 MCInstPrinter *InstPrint) {
  return new LoongArchTargetAsmStreamer(S, OS);
}

namespace {

class LoongArchMCInstrAnalysis : public MCInstrAnalysis {
  int64_t GPRState[31] = {};
  std::bitset<31> GPRValidMask;

  static bool isGPR(MCRegister Reg) {
    return Reg >= LoongArch::R0 && Reg <= LoongArch::R31;
  }

  static unsigned getRegIndex(MCRegister Reg) {
    assert(isGPR(Reg) && Reg != LoongArch::R0 && "Invalid GPR reg");
    return Reg - LoongArch::R1;
  }

  void setGPRState(MCRegister Reg, std::optional<int64_t> Value) {
    if (Reg == LoongArch::R0)
      return;

    auto Index = getRegIndex(Reg);

    if (Value) {
      GPRState[Index] = *Value;
      GPRValidMask.set(Index);
    } else {
      GPRValidMask.reset(Index);
    }
  }

  std::optional<int64_t> getGPRState(MCRegister Reg) const {
    if (Reg == LoongArch::R0)
      return 0;

    auto Index = getRegIndex(Reg);

    if (GPRValidMask.test(Index))
      return GPRState[Index];
    return std::nullopt;
  }

public:
  explicit LoongArchMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}

  void resetState() override { GPRValidMask.reset(); }

  void updateState(const MCInst &Inst, uint64_t Addr) override {
    // Terminators mark the end of a basic block which means the sequentially
    // next instruction will be the first of another basic block and the current
    // state will typically not be valid anymore. For calls, we assume all
    // registers may be clobbered by the callee (TODO: should we take the
    // calling convention into account?).
    if (isTerminator(Inst) || isCall(Inst)) {
      resetState();
      return;
    }

    switch (Inst.getOpcode()) {
    default: {
      // Clear the state of all defined registers for instructions that we don't
      // explicitly support.
      auto NumDefs = Info->get(Inst.getOpcode()).getNumDefs();
      for (unsigned I = 0; I < NumDefs; ++I) {
        auto DefReg = Inst.getOperand(I).getReg();
        if (isGPR(DefReg))
          setGPRState(DefReg, std::nullopt);
      }
      break;
    }
    case LoongArch::PCADDU18I:
      setGPRState(
          Inst.getOperand(0).getReg(),
          Addr + SignExtend64<38>(
                     static_cast<uint64_t>(Inst.getOperand(1).getImm()) << 18));
      break;
    }
  }

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override {
    unsigned NumOps = Inst.getNumOperands();
    if ((isBranch(Inst) && !isIndirectBranch(Inst)) ||
        Inst.getOpcode() == LoongArch::BL) {
      Target = Addr + Inst.getOperand(NumOps - 1).getImm();
      return true;
    }

    if (Inst.getOpcode() == LoongArch::JIRL) {
      if (auto TargetRegState = getGPRState(Inst.getOperand(1).getReg())) {
        Target = *TargetRegState + Inst.getOperand(2).getImm();
        return true;
      }
      return false;
    }

    return false;
  }

  bool isTerminator(const MCInst &Inst) const override {
    if (MCInstrAnalysis::isTerminator(Inst))
      return true;

    switch (Inst.getOpcode()) {
    default:
      return false;
    case LoongArch::JIRL:
      return Inst.getOperand(0).getReg() == LoongArch::R0;
    }
  }

  bool isCall(const MCInst &Inst) const override {
    if (MCInstrAnalysis::isCall(Inst))
      return true;

    switch (Inst.getOpcode()) {
    default:
      return false;
    case LoongArch::JIRL:
      return Inst.getOperand(0).getReg() != LoongArch::R0;
    }
  }

  bool isReturn(const MCInst &Inst) const override {
    if (MCInstrAnalysis::isReturn(Inst))
      return true;

    switch (Inst.getOpcode()) {
    default:
      return false;
    case LoongArch::JIRL:
      return Inst.getOperand(0).getReg() == LoongArch::R0 &&
             Inst.getOperand(1).getReg() == LoongArch::R1;
    }
  }

  bool isBranch(const MCInst &Inst) const override {
    if (MCInstrAnalysis::isBranch(Inst))
      return true;

    switch (Inst.getOpcode()) {
    default:
      return false;
    case LoongArch::JIRL:
      return Inst.getOperand(0).getReg() == LoongArch::R0 &&
             Inst.getOperand(1).getReg() != LoongArch::R1;
    }
  }

  bool isUnconditionalBranch(const MCInst &Inst) const override {
    if (MCInstrAnalysis::isUnconditionalBranch(Inst))
      return true;

    switch (Inst.getOpcode()) {
    default:
      return false;
    case LoongArch::JIRL:
      return Inst.getOperand(0).getReg() == LoongArch::R0 &&
             Inst.getOperand(1).getReg() != LoongArch::R1;
    }
  }

  bool isIndirectBranch(const MCInst &Inst) const override {
    if (MCInstrAnalysis::isIndirectBranch(Inst))
      return true;

    switch (Inst.getOpcode()) {
    default:
      return false;
    case LoongArch::JIRL:
      return Inst.getOperand(0).getReg() == LoongArch::R0 &&
             Inst.getOperand(1).getReg() != LoongArch::R1;
    }
  }
};

} // end namespace

static MCInstrAnalysis *createLoongArchInstrAnalysis(const MCInstrInfo *Info) {
  return new LoongArchMCInstrAnalysis(Info);
}

namespace {
MCStreamer *createLoongArchELFStreamer(const Triple &T, MCContext &Context,
                                       std::unique_ptr<MCAsmBackend> &&MAB,
                                       std::unique_ptr<MCObjectWriter> &&MOW,
                                       std::unique_ptr<MCCodeEmitter> &&MCE) {
  return createLoongArchELFStreamer(Context, std::move(MAB), std::move(MOW),
                                    std::move(MCE));
}
} // end namespace

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeLoongArchTargetMC() {
  for (Target *T : {&getTheLoongArch32Target(), &getTheLoongArch64Target()}) {
    TargetRegistry::RegisterMCRegInfo(*T, createLoongArchMCRegisterInfo);
    TargetRegistry::RegisterMCInstrInfo(*T, createLoongArchMCInstrInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createLoongArchMCSubtargetInfo);
    TargetRegistry::RegisterMCAsmInfo(*T, createLoongArchMCAsmInfo);
    TargetRegistry::RegisterMCCodeEmitter(*T, createLoongArchMCCodeEmitter);
    TargetRegistry::RegisterMCAsmBackend(*T, createLoongArchAsmBackend);
    TargetRegistry::RegisterMCInstPrinter(*T, createLoongArchMCInstPrinter);
    TargetRegistry::RegisterMCInstrAnalysis(*T, createLoongArchInstrAnalysis);
    TargetRegistry::RegisterELFStreamer(*T, createLoongArchELFStreamer);
    TargetRegistry::RegisterObjectTargetStreamer(
        *T, createLoongArchObjectTargetStreamer);
    TargetRegistry::RegisterAsmTargetStreamer(*T,
                                              createLoongArchAsmTargetStreamer);
  }
}
