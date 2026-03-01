//===-- M68kFixupKinds.h - M68k Specific Fixup Entries ----------*- C++ -*-===//
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
///
/// \file
/// This file contains M68k specific fixup entries.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68k_MCTARGETDESC_M68kFIXUPKINDS_H
#define LLVM_LIB_TARGET_M68k_MCTARGETDESC_M68kFIXUPKINDS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
static inline unsigned getFixupKindLog2Size(unsigned Kind) {
  switch (Kind) {
  case FK_Data_1:
    return 0;
  case FK_Data_2:
    return 1;
  case FK_Data_4:
    return 2;
  }
  llvm_unreachable("invalid fixup kind!");
}
} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68k_MCTARGETDESC_M68kFIXUPKINDS_H
