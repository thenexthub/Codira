//===-- CSKYFixupKinds.h - CSKY Specific Fixup Entries ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYFIXUPKINDS_H
#define LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace CSKY {
enum Fixups {
  fixup_csky_addr32 = FirstTargetFixupKind,

  fixup_csky_addr_hi16,

  fixup_csky_addr_lo16,

  fixup_csky_pcrel_imm16_scale2,

  fixup_csky_pcrel_uimm16_scale4,

  fixup_csky_pcrel_imm26_scale2,

  fixup_csky_pcrel_imm18_scale2,

  fixup_csky_gotpc,

  fixup_csky_gotoff,

  fixup_csky_got32,

  fixup_csky_got_imm18_scale4,

  fixup_csky_plt32,

  fixup_csky_plt_imm18_scale4,

  fixup_csky_pcrel_imm10_scale2,

  fixup_csky_pcrel_uimm7_scale4,

  fixup_csky_pcrel_uimm8_scale4,

  fixup_csky_doffset_imm18,

  fixup_csky_doffset_imm18_scale2,

  fixup_csky_doffset_imm18_scale4,
  // Marker
  fixup_csky_invalid,
  NumTargetFixupKinds = fixup_csky_invalid - FirstTargetFixupKind
};
} // end namespace CSKY
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYFIXUPKINDS_H
