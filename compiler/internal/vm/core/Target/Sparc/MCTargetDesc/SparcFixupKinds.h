//===-- SparcFixupKinds.h - Sparc Specific Fixup Entries --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCFIXUPKINDS_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

// clang-format off
namespace vm::core {
  namespace Sparc {
  // clang-format off
    enum Fixups {
      // fixup_sparc_call30 - 30-bit PC relative relocation for call
      fixup_sparc_call30 = FirstTargetFixupKind,

      /// fixup_sparc_13 - 13-bit fixup
      fixup_sparc_13,

      // Marker
      LastTargetFixupKind,
      NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
    };
  // clang-format on
  }
}
// clang-format on
#endif
