//===-- SparcMCTargetDesc.h - Sparc Target Descriptions ---------*- C++ -*-===//
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
// This file provides Sparc specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCTARGETDESC_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCTARGETDESC_H

#include "vm/core/ADT/StringRef.h"
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
class Triple;

MCCodeEmitter *createSparcMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);
MCAsmBackend *createSparcAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter>
createSparcELFObjectWriter(bool Is64Bit, bool IsV8Plus, uint8_t OSABI);

// Defines symbolic names for Sparc v9 ASI tag names.
namespace SparcASITag {
struct ASITag {
  const char *Name;
  const char *AltName;
  unsigned Encoding;
};

#define GET_ASITagsList_DECL
#include "SparcGenSearchableTables.inc"
} // end namespace SparcASITag

// Defines symbolic names for Sparc v9 prefetch tag names.
namespace SparcPrefetchTag {
struct PrefetchTag {
  const char *Name;
  unsigned Encoding;
};

#define GET_PrefetchTagsList_DECL
#include "SparcGenSearchableTables.inc"
} // end namespace SparcPrefetchTag
} // End toolchain namespace

// Defines symbolic names for Sparc registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "SparcGenRegisterInfo.inc"

// Defines symbolic names for the Sparc instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "SparcGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SparcGenSubtargetInfo.inc"

#endif
