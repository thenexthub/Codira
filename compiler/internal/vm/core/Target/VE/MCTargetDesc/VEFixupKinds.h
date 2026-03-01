//===-- VEFixupKinds.h - VE Specific Fixup Entries --------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_VE_MCTARGETDESC_VEFIXUPKINDS_H
#define LLVM_LIB_TARGET_VE_MCTARGETDESC_VEFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace VE {
enum Fixups {
  /// fixup_ve_reflong - 32-bit fixup corresponding to foo
  fixup_ve_reflong = FirstTargetFixupKind,

  /// fixup_ve_srel32 - 32-bit fixup corresponding to foo for relative branch
  fixup_ve_srel32,

  /// fixup_ve_hi32 - 32-bit fixup corresponding to foo\@hi
  fixup_ve_hi32,

  /// fixup_ve_lo32 - 32-bit fixup corresponding to foo\@lo
  fixup_ve_lo32,

  /// fixup_ve_pc_hi32 - 32-bit fixup corresponding to foo\@pc_hi
  fixup_ve_pc_hi32,

  /// fixup_ve_pc_lo32 - 32-bit fixup corresponding to foo\@pc_lo
  fixup_ve_pc_lo32,

  /// fixup_ve_got_hi32 - 32-bit fixup corresponding to foo\@got_hi
  fixup_ve_got_hi32,

  /// fixup_ve_got_lo32 - 32-bit fixup corresponding to foo\@got_lo
  fixup_ve_got_lo32,

  /// fixup_ve_gotoff_hi32 - 32-bit fixup corresponding to foo\@gotoff_hi
  fixup_ve_gotoff_hi32,

  /// fixup_ve_gotoff_lo32 - 32-bit fixup corresponding to foo\@gotoff_lo
  fixup_ve_gotoff_lo32,

  /// fixup_ve_plt_hi32/lo32
  fixup_ve_plt_hi32,
  fixup_ve_plt_lo32,

  /// fixups for Thread Local Storage
  fixup_ve_tls_gd_hi32,
  fixup_ve_tls_gd_lo32,
  fixup_ve_tpoff_hi32,
  fixup_ve_tpoff_lo32,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
} // namespace VE
} // namespace vm::core

#endif
