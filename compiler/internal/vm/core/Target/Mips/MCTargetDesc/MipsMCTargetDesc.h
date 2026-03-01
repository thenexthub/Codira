//===-- MipsMCTargetDesc.h - Mips Target Descriptions -----------*- C++ -*-===//
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
// This file provides Mips specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCTARGETDESC_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCTARGETDESC_H

#include "vm/core/Support/DataTypes.h"

#include <memory>

namespace vm::core {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegisterInfo;
class MCStreamer;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;

MCCodeEmitter *createMipsMCCodeEmitterEB(const MCInstrInfo &MCII,
                                         MCContext &Ctx);
MCCodeEmitter *createMipsMCCodeEmitterEL(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createMipsAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

/// Construct an MIPS Windows COFF machine code streamer which will generate
/// PE/COFF format object files.
///
/// Takes ownership of \p AB and \p CE.
MCStreamer *createMipsWinCOFFStreamer(MCContext &C,
                                      std::unique_ptr<MCAsmBackend> &&AB,
                                      std::unique_ptr<MCObjectWriter> &&OW,
                                      std::unique_ptr<MCCodeEmitter> &&CE);

/// Construct a Mips ELF object writer.
std::unique_ptr<MCObjectTargetWriter>
createMipsELFObjectWriter(const Triple &TT, bool IsN32);
/// Construct a Mips Win COFF object writer.
std::unique_ptr<MCObjectTargetWriter> createMipsWinCOFFObjectWriter();

namespace MIPS_MC {
void initLLVMToCVRegMapping(MCRegisterInfo *MRI);

StringRef selectMipsCPU(const Triple &TT, StringRef CPU);
}

} // End toolchain namespace

// Defines symbolic names for Mips registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "MipsGenRegisterInfo.inc"

// Defines symbolic names for the Mips instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "MipsGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "MipsGenSubtargetInfo.inc"

#endif
