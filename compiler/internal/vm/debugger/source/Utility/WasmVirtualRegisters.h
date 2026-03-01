//===----------------------------------------------------------------------===//
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

#ifndef LLDB_SOURCE_UTILITY_WASM_VIRTUAL_REGISTERS_H
#define LLDB_SOURCE_UTILITY_WASM_VIRTUAL_REGISTERS_H

#include "lldb/lldb-private.h"

namespace lldb_private {

// LLDB doesn't have an address space to represents WebAssembly locals,
// globals and operand stacks. We encode these elements into virtual
// registers:
//
//   | tag: 2 bits | index: 30 bits |
//
// Where tag is:
//    0: Not a Wasm location
//    1: Local
//    2: Global
//    3: Operand stack value
enum WasmVirtualRegisterKinds {
  eWasmTagNotAWasmLocation = 0,
  eWasmTagLocal = 1,
  eWasmTagGlobal = 2,
  eWasmTagOperandStack = 3,
};

static const uint32_t kWasmVirtualRegisterTagMask = 0x03;
static const uint32_t kWasmVirtualRegisterIndexMask = 0x3fffffff;
static const uint32_t kWasmVirtualRegisterTagShift = 30;

inline uint32_t GetWasmVirtualRegisterTag(size_t reg) {
  return (reg >> kWasmVirtualRegisterTagShift) & kWasmVirtualRegisterTagMask;
}

inline uint32_t GetWasmVirtualRegisterIndex(size_t reg) {
  return reg & kWasmVirtualRegisterIndexMask;
}

inline uint32_t GetWasmRegister(uint8_t tag, uint32_t index) {
  return ((tag & kWasmVirtualRegisterTagMask) << kWasmVirtualRegisterTagShift) |
         (index & kWasmVirtualRegisterIndexMask);
}

} // namespace lldb_private

#endif
