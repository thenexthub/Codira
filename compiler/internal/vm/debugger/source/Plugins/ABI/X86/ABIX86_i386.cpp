//===-- ABIX86_i386.cpp ---------------------------------------------------===//
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

#include "ABIX86_i386.h"

uint32_t ABIX86_i386::GetGenericNum(llvm::StringRef name) {
  return llvm::StringSwitch<uint32_t>(name)
      .Case("eip", LLDB_REGNUM_GENERIC_PC)
      .Case("esp", LLDB_REGNUM_GENERIC_SP)
      .Case("ebp", LLDB_REGNUM_GENERIC_FP)
      .Case("eflags", LLDB_REGNUM_GENERIC_FLAGS)
      .Case("edi", LLDB_REGNUM_GENERIC_ARG1)
      .Case("esi", LLDB_REGNUM_GENERIC_ARG2)
      .Case("edx", LLDB_REGNUM_GENERIC_ARG3)
      .Case("ecx", LLDB_REGNUM_GENERIC_ARG4)
      .Default(LLDB_INVALID_REGNUM);
}
