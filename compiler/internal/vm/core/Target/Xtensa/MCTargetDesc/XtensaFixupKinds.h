//===-- XtensaMCFixups.h - Xtensa-specific fixup entries --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
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

#ifndef LLVM_LIB_TARGET_XTENSA_MCTARGETDESC_XTENSAMCFIXUPS_H
#define LLVM_LIB_TARGET_XTENSA_MCTARGETDESC_XTENSAMCFIXUPS_H

#include "vm/core/MC/MCFixup.h"

namespace vm::core {
namespace Xtensa {
enum FixupKind {
  fixup_xtensa_branch_6 = FirstTargetFixupKind,
  fixup_xtensa_branch_8,
  fixup_xtensa_branch_12,
  fixup_xtensa_jump_18,
  fixup_xtensa_call_18,
  fixup_xtensa_l32r_16,
  fixup_xtensa_loop_8,
  fixup_xtensa_invalid,
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
} // end namespace Xtensa
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_XTENSA_MCTARGETDESC_XTENSAMCFIXUPS_H
