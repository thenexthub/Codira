//===-- PPCFixupKinds.h - PPC Specific Fixup Entries ------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_POWERPC_MCTARGETDESC_PPCFIXUPKINDS_H
#define LLVM_LIB_TARGET_POWERPC_MCTARGETDESC_PPCFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

#undef PPC

namespace vm::core {
namespace PPC {
enum Fixups {
  // 24-bit PC relative relocation for direct branches like 'b' and 'bl'.
  fixup_ppc_br24 = FirstTargetFixupKind,

  // 24-bit PC relative relocation for direct branches like 'b' and 'bl' where
  // the caller does not use the TOC.
  fixup_ppc_br24_notoc,

  /// 14-bit PC relative relocation for conditional branches.
  fixup_ppc_brcond14,

  /// 24-bit absolute relocation for direct branches like 'ba' and 'bla'.
  fixup_ppc_br24abs,

  /// 14-bit absolute relocation for conditional branches.
  fixup_ppc_brcond14abs,

  /// A 16-bit fixup corresponding to lo16(_foo) or ha16(_foo) for instrs like
  /// 'li' or 'addis'.
  fixup_ppc_half16,

  /// A 14-bit fixup corresponding to lo16(_foo) with implied 2 zero bits for
  /// instrs like 'std'.
  fixup_ppc_half16ds,

  // A 32-bit fixup corresponding to PC-relative paddis.
  fixup_ppc_pcrel32,

  // A 32-bit fixup corresponding to Non-PC-relative paddis.
  fixup_ppc_imm32,

  // A 34-bit fixup corresponding to PC-relative paddi.
  fixup_ppc_pcrel34,

  // A 34-bit fixup corresponding to Non-PC-relative paddi.
  fixup_ppc_imm34,

  /// Not a true fixup, but ties a symbol to a call to __tls_get_addr for the
  /// TLS general and local dynamic models, or inserts the thread-pointer
  /// register number.
  fixup_ppc_nofixup,

  /// A 16-bit fixup corresponding to lo16(_foo) with implied 3 zero bits for
  /// instrs like 'lxv'. Produces the same relocation as fixup_ppc_half16ds.
  fixup_ppc_half16dq,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}
}

#endif
