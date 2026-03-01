//===-- X86AsmParserCommon.h - Common functions for X86AsmParser ---------===//
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

#ifndef LLVM_LIB_TARGET_X86_ASMPARSER_X86ASMPARSERCOMMON_H
#define LLVM_LIB_TARGET_X86_ASMPARSER_X86ASMPARSERCOMMON_H

#include "vm/core/Support/MathExtras.h"

namespace vm::core {

inline bool isImmSExti16i8Value(uint64_t Value) {
  return isInt<8>(Value) ||
         (isUInt<16>(Value) && isInt<8>(static_cast<int16_t>(Value)));
}

inline bool isImmSExti32i8Value(uint64_t Value) {
  return isInt<8>(Value) ||
         (isUInt<32>(Value) && isInt<8>(static_cast<int32_t>(Value)));
}

inline bool isImmSExti64i8Value(uint64_t Value) {
  return isInt<8>(Value);
}

inline bool isImmSExti64i32Value(uint64_t Value) {
  return isInt<32>(Value);
}

inline bool isImmUnsignedi8Value(uint64_t Value) {
  return isUInt<8>(Value) || isInt<8>(Value);
}

inline bool isImmUnsignedi4Value(uint64_t Value) {
  return isUInt<4>(Value);
}

inline bool isImmUnsignedi6Value(uint64_t Value) {
  return isUInt<6>(Value);
}

} // End of namespace vm::core

#endif
