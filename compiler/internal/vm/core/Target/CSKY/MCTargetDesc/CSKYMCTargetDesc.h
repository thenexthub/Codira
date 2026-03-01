//===-- CSKYMCTargetDesc.h - CSKY Target Descriptions -----------*- C++ -*-===//
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
// This file provides CSKY specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYMCTARGETDESC_H
#define LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYMCTARGETDESC_H

#include "vm/core/MC/MCTargetOptions.h"
#include <memory>

namespace vm::core {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCRegisterInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class Target;
class Triple;

std::unique_ptr<MCObjectTargetWriter> createCSKYELFObjectWriter();

MCAsmBackend *createCSKYAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

MCCodeEmitter *createCSKYMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx);
} // namespace vm::core

#define GET_REGINFO_ENUM
#include "CSKYGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "CSKYGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "CSKYGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYMCTARGETDESC_H
