//===-- AArch64MCTargetDesc.h - AArch64 Target Descriptions -----*- C++ -*-===//
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
// This file provides AArch64 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_MCTARGETDESC_AARCH64MCTARGETDESC_H
#define LLVM_LIB_TARGET_AARCH64_MCTARGETDESC_AARCH64MCTARGETDESC_H

#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/Support/DataTypes.h"

#include <memory>

namespace vm::core {
class formatted_raw_ostream;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInst;
class MCInstrInfo;
class MCInstPrinter;
class MCRegisterInfo;
class MCObjectTargetWriter;
class MCStreamer;
class MCSubtargetInfo;
class MCTargetOptions;
class MCTargetStreamer;
class Target;
class Triple;

MCCodeEmitter *createAArch64MCCodeEmitter(const MCInstrInfo &MCII,
                                          MCContext &Ctx);
MCAsmBackend *createAArch64leAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options);
MCAsmBackend *createAArch64beAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter>
createAArch64ELFObjectWriter(uint8_t OSABI, bool IsILP32);

std::unique_ptr<MCObjectTargetWriter>
createAArch64MachObjectWriter(uint32_t CPUType, uint32_t CPUSubtype,
                              bool IsILP32);

std::unique_ptr<MCObjectTargetWriter>
createAArch64WinCOFFObjectWriter(const Triple &TheTriple);

MCTargetStreamer *createAArch64AsmTargetStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint);

namespace AArch64_MC {
void initLLVMToCVRegMapping(MCRegisterInfo *MRI);
bool isHForm(const MCInst &MI, const MCInstrInfo *MCII);
bool isQForm(const MCInst &MI, const MCInstrInfo *MCII);
bool isFpOrNEON(const MCInst &MI, const MCInstrInfo *MCII);
} // namespace AArch64_MC

namespace AArch64 {
enum OperandType {
  OPERAND_IMPLICIT_IMM_0 = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_SHIFT_MSL,
  OPERAND_SHIFTED_REGISTER,
  OPERAND_SHIFTED_IMMEDIATE,
};
} // namespace AArch64

} // namespace vm::core

// Defines symbolic names for AArch64 registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "AArch64GenRegisterInfo.inc"

// Defines symbolic names for the AArch64 instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "AArch64GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "AArch64GenSubtargetInfo.inc"

#endif
