//===- LoongArchFixupKinds.h - LoongArch Specific Fixup Entries -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHFIXUPKINDS_H
#define LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace LoongArch {
//
// This table *must* be in the same order of
// MCFixupKindInfo Infos[LoongArch::NumTargetFixupKinds] in
// LoongArchAsmBackend.cpp.
//
enum Fixups {
  // Define fixups can be handled by LoongArchAsmBackend::applyFixup.
  // 16-bit fixup corresponding to %b16(foo) for instructions like bne.
  fixup_loongarch_b16 = FirstTargetFixupKind,
  // 21-bit fixup corresponding to %b21(foo) for instructions like bnez.
  fixup_loongarch_b21,
  // 26-bit fixup corresponding to %b26(foo)/%plt(foo) for instructions b/bl.
  fixup_loongarch_b26,
  // 20-bit fixup corresponding to %abs_hi20(foo) for instruction lu12i.w.
  fixup_loongarch_abs_hi20,
  // 12-bit fixup corresponding to %abs_lo12(foo) for instruction ori.
  fixup_loongarch_abs_lo12,
  // 20-bit fixup corresponding to %abs64_lo20(foo) for instruction lu32i.d.
  fixup_loongarch_abs64_lo20,
  // 12-bit fixup corresponding to %abs_hi12(foo) for instruction lu52i.d.
  fixup_loongarch_abs64_hi12,

  // Used as a sentinel, must be the last of the fixup which can be handled by
  // LoongArchAsmBackend::applyFixup.
  fixup_loongarch_invalid,
  NumTargetFixupKinds = fixup_loongarch_invalid - FirstTargetFixupKind,
};
} // end namespace LoongArch
} // end namespace vm::core

#endif
