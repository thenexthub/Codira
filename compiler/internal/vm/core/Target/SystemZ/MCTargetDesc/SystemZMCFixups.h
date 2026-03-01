//===-- SystemZMCFixups.h - SystemZ-specific fixup entries ------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZMCFIXUPS_H
#define LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZMCFIXUPS_H

#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace SystemZ {
enum FixupKind {
  // These correspond directly to R_390_* relocations.
  FK_390_PC12DBL = FirstTargetFixupKind,
  FK_390_PC16DBL,
  FK_390_PC24DBL,
  FK_390_PC32DBL,
  FK_390_TLS_CALL,

  FK_390_S8Imm,
  FK_390_S16Imm,
  FK_390_S20Imm,
  FK_390_S32Imm,
  FK_390_U1Imm,
  FK_390_U2Imm,
  FK_390_U3Imm,
  FK_390_U4Imm,
  FK_390_U8Imm,
  FK_390_U12Imm,
  FK_390_U16Imm,
  FK_390_U32Imm,
  FK_390_U48Imm,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

// clang-format off
const static MCFixupKindInfo MCFixupKindInfos[SystemZ::NumTargetFixupKinds] = {
    {"FK_390_PC12DBL", 4, 12, 0},
    {"FK_390_PC16DBL", 0, 16, 0},
    {"FK_390_PC24DBL", 0, 24, 0},
    {"FK_390_PC32DBL", 0, 32, 0},
    {"FK_390_TLS_CALL",0,  0, 0},
    {"FK_390_S8Imm",   0,  8, 0},
    {"FK_390_S16Imm",  0, 16, 0},
    {"FK_390_S20Imm",  4, 20, 0},
    {"FK_390_S32Imm",  0, 32, 0},
    {"FK_390_U1Imm",   0,  1, 0},
    {"FK_390_U2Imm",   0,  2, 0},
    {"FK_390_U3Imm",   0,  3, 0},
    {"FK_390_U4Imm",   0,  4, 0},
    {"FK_390_U8Imm",   0,  8, 0},
    {"FK_390_U12Imm",  4, 12, 0},
    {"FK_390_U16Imm",  0, 16, 0},
    {"FK_390_U32Imm",  0, 32, 0},
    {"FK_390_U48Imm",  0, 48, 0},
};
// clang-format on
} // end namespace SystemZ
} // end namespace vm::core

#endif
