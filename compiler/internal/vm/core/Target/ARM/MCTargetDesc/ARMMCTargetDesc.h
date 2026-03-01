//===-- ARMMCTargetDesc.h - ARM Target Descriptions -------------*- C++ -*-===//
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
// This file provides ARM specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_MCTARGETDESC_ARMMCTARGETDESC_H
#define LLVM_LIB_TARGET_ARM_MCTARGETDESC_ARMMCTARGETDESC_H

#include "vm/core/Support/DataTypes.h"
#include "vm/core/MC/MCInstrDesc.h"
#include <memory>
#include <string>

namespace vm::core {
class formatted_raw_ostream;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCInstPrinter;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCStreamer;
class MCTargetOptions;
class MCRelocationInfo;
class MCTargetStreamer;
class StringRef;
class Target;
class Triple;

namespace ARM_MC {
std::string ParseARMTriple(const Triple &TT, StringRef CPU);
void initLLVMToCVRegMapping(MCRegisterInfo *MRI);

bool isPredicated(const MCInst &MI, const MCInstrInfo *MCII);
bool isCPSRDefined(const MCInst &MI, const MCInstrInfo *MCII);

template<class Inst>
bool isLDMBaseRegInList(const Inst &MI) {
  auto BaseReg = MI.getOperand(0).getReg();
  for (unsigned I = 1, E = MI.getNumOperands(); I < E; ++I) {
    const auto &Op = MI.getOperand(I);
    if (Op.isReg() && Op.getReg() == BaseReg)
      return true;
  }
  return false;
}

uint64_t evaluateBranchTarget(const MCInstrDesc &InstDesc, uint64_t Addr,
                              int64_t Imm);

/// Create a ARM MCSubtargetInfo instance. This is exposed so Asm parser, etc.
/// do not need to go through TargetRegistry.
MCSubtargetInfo *createARMMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                          StringRef FS);
}

MCTargetStreamer *createARMNullTargetStreamer(MCStreamer &S);
MCTargetStreamer *createARMTargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS,
                                             MCInstPrinter *InstPrint);
MCTargetStreamer *createARMObjectTargetStreamer(MCStreamer &S,
                                                const MCSubtargetInfo &STI);
MCTargetStreamer *createARMObjectTargetELFStreamer(MCStreamer &S);
MCTargetStreamer *createARMObjectTargetMachOStreamer(MCStreamer &S);
MCTargetStreamer *createARMObjectTargetWinCOFFStreamer(MCStreamer &S);

MCCodeEmitter *createARMLEMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);

MCCodeEmitter *createARMBEMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);

MCAsmBackend *createARMLEAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);

MCAsmBackend *createARMBEAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);

// Construct a PE/COFF machine code streamer which will generate a PE/COFF
// object file.
MCStreamer *createARMWinCOFFStreamer(MCContext &Context,
                                     std::unique_ptr<MCAsmBackend> &&MAB,
                                     std::unique_ptr<MCObjectWriter> &&OW,
                                     std::unique_ptr<MCCodeEmitter> &&Emitter);

/// Construct an ELF Mach-O object writer.
std::unique_ptr<MCObjectTargetWriter> createARMELFObjectWriter(uint8_t OSABI);

/// Construct an ARM Mach-O object writer.
std::unique_ptr<MCObjectTargetWriter>
createARMMachObjectWriter(bool Is64Bit, uint32_t CPUType,
                          uint32_t CPUSubtype);

/// Construct an ARM PE/COFF object writer.
std::unique_ptr<MCObjectTargetWriter>
createARMWinCOFFObjectWriter();

/// Construct ARM Mach-O relocation info.
MCRelocationInfo *createARMMachORelocationInfo(MCContext &Ctx);

namespace ARM {
enum OperandType {
  OPERAND_VPRED_R = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_VPRED_N,
};
inline bool isVpred(OperandType op) {
  return op == OPERAND_VPRED_R || op == OPERAND_VPRED_N;
}
inline bool isVpred(uint8_t op) {
  return isVpred(static_cast<OperandType>(op));
}

bool isCDECoproc(size_t Coproc, const MCSubtargetInfo &STI);

} // end namespace ARM

} // End toolchain namespace

// Defines symbolic names for ARM registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "ARMGenRegisterInfo.inc"

// Defines symbolic names for the ARM instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "ARMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "ARMGenSubtargetInfo.inc"

#endif
