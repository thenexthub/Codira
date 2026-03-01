//===-- AVRMCTargetDesc.h - AVR Target Descriptions -------------*- C++ -*-===//
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

#ifndef LLVM_AVR_MCTARGET_DESC_H
#define LLVM_AVR_MCTARGET_DESC_H

#include "vm/core/Support/DataTypes.h"

#include <memory>

namespace vm::core {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCInstrInfo *createAVRMCInstrInfo();

/// Creates a machine code emitter for AVR.
MCCodeEmitter *createAVRMCCodeEmitter(const MCInstrInfo &MCII,
                                      MCContext &Ctx);

/// Creates an assembly backend for AVR.
MCAsmBackend *createAVRAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const toolchain::MCTargetOptions &TO);

/// Creates an ELF object writer for AVR.
std::unique_ptr<MCObjectTargetWriter> createAVRELFObjectWriter(uint8_t OSABI);

} // end namespace vm::core

#define GET_REGINFO_ENUM
#include "AVRGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "AVRGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "AVRGenSubtargetInfo.inc"

#endif // LLVM_AVR_MCTARGET_DESC_H
