//===-- AArch64FixupKinds.h - AArch64 Specific Fixup Entries ----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AARCH64_MCTARGETDESC_AARCH64FIXUPKINDS_H
#define LLVM_LIB_TARGET_AARCH64_MCTARGETDESC_AARCH64FIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace AArch64 {

enum Fixups {
  // A 21-bit pc-relative immediate inserted into an ADR instruction.
  fixup_aarch64_pcrel_adr_imm21 = FirstTargetFixupKind,

  // A 21-bit pc-relative immediate inserted into an ADRP instruction.
  fixup_aarch64_pcrel_adrp_imm21,

  // 12-bit fixup for add/sub instructions. No alignment adjustment. All value
  // bits are encoded.
  fixup_aarch64_add_imm12,

  // unsigned 12-bit fixups for load and store instructions.
  fixup_aarch64_ldst_imm12_scale1,
  fixup_aarch64_ldst_imm12_scale2,
  fixup_aarch64_ldst_imm12_scale4,
  fixup_aarch64_ldst_imm12_scale8,
  fixup_aarch64_ldst_imm12_scale16,

  // The high 19 bits of a 21-bit pc-relative immediate. Same encoding as
  // fixup_aarch64_pcrel_adrhi, except this is used by pc-relative loads and
  // generates relocations directly when necessary.
  fixup_aarch64_ldr_pcrel_imm19,

  // FIXME: comment
  fixup_aarch64_movw,

  // The high 9 bits of a 11-bit pc-relative immediate.
  fixup_aarch64_pcrel_branch9,

  // The high 14 bits of a 21-bit pc-relative immediate.
  fixup_aarch64_pcrel_branch14,

  // The high 16 bits of a 18-bit unsigned PC-relative immediate. Used by
  // pointer authentication, only within a function, so no relocation can be
  // generated.
  fixup_aarch64_pcrel_branch16,

  // The high 19 bits of a 21-bit pc-relative immediate. Same encoding as
  // fixup_aarch64_pcrel_adrhi, except this is use by b.cc and generates
  // relocations directly when necessary.
  fixup_aarch64_pcrel_branch19,

  // The high 26 bits of a 28-bit pc-relative immediate.
  fixup_aarch64_pcrel_branch26,

  // The high 26 bits of a 28-bit pc-relative immediate. Distinguished from
  // branch26 only on ELF.
  fixup_aarch64_pcrel_call26,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // end namespace AArch64
} // end namespace vm::core

#endif
