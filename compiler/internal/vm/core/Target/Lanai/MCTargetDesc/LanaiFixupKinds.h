//===-- LanaiFixupKinds.h - Lanai Specific Fixup Entries --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_LANAI_MCTARGETDESC_LANAIFIXUPKINDS_H
#define LLVM_LIB_TARGET_LANAI_MCTARGETDESC_LANAIFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace Lanai {
// Although most of the current fixup types reflect a unique relocation
// one can have multiple fixup types for a given relocation and thus need
// to be uniquely named.
//
// This table *must* be in the save order of
// MCFixupKindInfo Infos[Lanai::NumTargetFixupKinds]
// in LanaiAsmBackend.cpp.
//
enum Fixups {
  // Results in R_Lanai_NONE
  FIXUP_LANAI_NONE = FirstTargetFixupKind,

  FIXUP_LANAI_21,   // 21-bit symbol relocation
  FIXUP_LANAI_21_F, // 21-bit symbol relocation, last two bits masked to 0
  FIXUP_LANAI_25,   // 25-bit branch targets
  FIXUP_LANAI_32,   // general 32-bit relocation
  FIXUP_LANAI_HI16, // upper 16-bits of a symbolic relocation
  FIXUP_LANAI_LO16, // lower 16-bits of a symbolic relocation

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
} // namespace Lanai
} // namespace vm::core

#endif // LLVM_LIB_TARGET_LANAI_MCTARGETDESC_LANAIFIXUPKINDS_H
