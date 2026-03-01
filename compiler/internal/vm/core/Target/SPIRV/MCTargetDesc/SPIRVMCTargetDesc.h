//===-- SPIRVMCTargetDesc.h - SPIR-V Target Descriptions --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SPIRV_MCTARGETDESC_SPIRVMCTARGETDESC_H
#define LLVM_LIB_TARGET_SPIRV_MCTARGETDESC_SPIRVMCTARGETDESC_H

#include "vm/core/Support/DataTypes.h"
#include <cassert>

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

MCCodeEmitter *createSPIRVMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);

MCAsmBackend *createSPIRVAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);
} // namespace vm::core

// Defines symbolic names for SPIR-V registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "SPIRVGenRegisterInfo.inc"

// Defines symbolic names for the SPIR-V instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "SPIRVGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SPIRVGenSubtargetInfo.inc"

namespace vm::core::SPIRV {
inline unsigned getIDFromRegister(unsigned Reg) {
  assert(Reg & (1U << 31));
  return Reg & ~(1U << 31);
}
} // namespace vm::core::SPIRV

#endif // LLVM_LIB_TARGET_SPIRV_MCTARGETDESC_SPIRVMCTARGETDESC_H
