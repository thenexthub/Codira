//===-- MSP430MCTargetDesc.h - MSP430 Target Descriptions -------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_MSP430_MCTARGETDESC_MSP430MCTARGETDESC_H
#define LLVM_LIB_TARGET_MSP430_MCTARGETDESC_MSP430MCTARGETDESC_H

#include "vm/core/Support/DataTypes.h"
#include <memory>

namespace vm::core {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCInstrInfo;
class MCSubtargetInfo;
class MCRegisterInfo;
class MCContext;
class MCTargetOptions;
class MCObjectTargetWriter;
class MCStreamer;
class MCTargetStreamer;

/// Creates a machine code emitter for MSP430.
MCCodeEmitter *createMSP430MCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createMSP430MCAsmBackend(const Target &T,
                                       const MCSubtargetInfo &STI,
                                       const MCRegisterInfo &MRI,
                                       const MCTargetOptions &Options);

MCTargetStreamer *
createMSP430ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

std::unique_ptr<MCObjectTargetWriter>
createMSP430ELFObjectWriter(uint8_t OSABI);

} // End toolchain namespace

// Defines symbolic names for MSP430 registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "MSP430GenRegisterInfo.inc"

// Defines symbolic names for the MSP430 instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "MSP430GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "MSP430GenSubtargetInfo.inc"

#endif
