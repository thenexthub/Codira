//===-- WebAssemblyMCTypeUtilities - WebAssembly Type Utilities-*- C++ -*-====//
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
/// This file contains the declaration of the WebAssembly-specific type parsing
/// utility functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_MCTARGETDESC_WEBASSEMBLYMCTYPEUTILITIES_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_MCTARGETDESC_WEBASSEMBLYMCTYPEUTILITIES_H

#include "vm/core/BinaryFormat/Wasm.h"

namespace vm::core {

namespace WebAssembly {

/// Used as immediate MachineOperands for block signatures
enum class BlockType : unsigned {
  Invalid = 0x00,
  Void = 0x40,
  I32 = unsigned(wasm::ValType::I32),
  I64 = unsigned(wasm::ValType::I64),
  F32 = unsigned(wasm::ValType::F32),
  F64 = unsigned(wasm::ValType::F64),
  V128 = unsigned(wasm::ValType::V128),
  Externref = unsigned(wasm::ValType::EXTERNREF),
  Funcref = unsigned(wasm::ValType::FUNCREF),
  Exnref = unsigned(wasm::ValType::EXNREF),
  // Multivalue blocks are emitted in two cases:
  // 1. When the blocks will never be exited and are at the ends of functions
  //    (see WebAssemblyCFGStackify::fixEndsAtEndOfFunction). In this case the
  //    exact multivalue signature can always be inferred from the return type
  //    of the parent function.
  // 2. (catch_ref ...) clause in try_table instruction. Currently all tags we
  //    support (cpp_exception and c_longjmp) throws a single i32, so the
  //    multivalue signature for this case will be (i32, exnref).
  // The real multivalue siganture will be added in MCInstLower.
  Multivalue = 0xffff,
};

inline bool isRefType(wasm::ValType Type) {
  return Type == wasm::ValType::EXTERNREF || Type == wasm::ValType::FUNCREF ||
         Type == wasm::ValType::EXNREF;
}

// Convert ValType or a list/signature of ValTypes to a string.

// Convert an unsigned integer, which can be among wasm::ValType enum, to its
// type name string. If the input is not within wasm::ValType, returns
// "invalid_type".
const char *anyTypeToString(unsigned Type);
const char *typeToString(wasm::ValType Type);
// Convert a list of ValTypes into a string in the format of
// "type0, type1, ... typeN"
std::string typeListToString(ArrayRef<wasm::ValType> List);
// Convert a wasm signature into a string in the format of
// "(params) -> (results)", where params and results are a string of ValType
// lists.
std::string signatureToString(const wasm::WasmSignature *Sig);

// Convert a register class ID to a wasm ValType.
wasm::ValType regClassToValType(unsigned RC);

// Convert StringRef to ValType / HealType / BlockType

std::optional<wasm::ValType> parseType(StringRef Type);
BlockType parseBlockType(StringRef Type);

} // end namespace WebAssembly
} // end namespace vm::core

#endif
