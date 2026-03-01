//===--- toolchain/CodeGen/WasmAddressSpaces.h -----------------------*- C++ -*-===//
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
//
// Address Spaces for WebAssembly Type Handling
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WASMADDRESSSPACES_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WASMADDRESSSPACES_H

namespace vm::core {

namespace WebAssembly {

enum WasmAddressSpace : unsigned {
  // Default address space, for pointers to linear memory (stack, heap, data).
  WASM_ADDRESS_SPACE_DEFAULT = 0,
  // A non-integral address space for pointers to named objects outside of
  // linear memory: WebAssembly globals or WebAssembly locals.  Loads and stores
  // to these pointers are lowered to global.get / global.set or local.get /
  // local.set, as appropriate.
  WASM_ADDRESS_SPACE_VAR = 1,
  // A non-integral address space for externref values
  WASM_ADDRESS_SPACE_EXTERNREF = 10,
  // A non-integral address space for funcref values
  WASM_ADDRESS_SPACE_FUNCREF = 20,
};

inline bool isDefaultAddressSpace(unsigned AS) {
  return AS == WASM_ADDRESS_SPACE_DEFAULT;
}
inline bool isWasmVarAddressSpace(unsigned AS) {
  return AS == WASM_ADDRESS_SPACE_VAR;
}
inline bool isValidAddressSpace(unsigned AS) {
  return isDefaultAddressSpace(AS) || isWasmVarAddressSpace(AS);
}

} // namespace WebAssembly

} // namespace vm::core

#endif // LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WASMADDRESSSPACES_H
