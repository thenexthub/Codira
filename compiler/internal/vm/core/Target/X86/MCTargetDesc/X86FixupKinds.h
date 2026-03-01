//===-- X86FixupKinds.h - X86 Specific Fixup Entries ------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86FIXUPKINDS_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86FIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace X86 {
enum Fixups {
  reloc_riprel_4byte = FirstTargetFixupKind, // 32-bit rip-relative
  reloc_riprel_4byte_movq_load,              // 32-bit rip-relative in movq
  reloc_riprel_4byte_movq_load_rex2,         // 32-bit rip-relative in movq
                                             // with rex2 prefix
  reloc_riprel_4byte_relax,                  // 32-bit rip-relative in relaxable
                                             // instruction
  reloc_riprel_4byte_relax_rex,              // 32-bit rip-relative in relaxable
                                             // instruction with rex prefix
  reloc_riprel_4byte_relax_rex2,             // 32-bit rip-relative in relaxable
                                             // instruction with rex2 prefix
  reloc_riprel_4byte_relax_evex,             // 32-bit rip-relative in relaxable
                                             // instruction of APX NDD/NF with
                                             // EVEX prefix
  reloc_signed_4byte,                        // 32-bit signed. Unlike FK_Data_4
                                             // this will be sign extended at
                                             // runtime.
  reloc_signed_4byte_relax,                  // like reloc_signed_4byte, but
                                             // in a relaxable instruction.
  reloc_global_offset_table,                 // 32-bit, relative to the start
                                             // of the instruction. Used only
                                             // for _GLOBAL_OFFSET_TABLE_.
  reloc_branch_4byte_pcrel,                  // 32-bit PC relative branch.
  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}
}

#endif
