//===-- WebAssemblyTypeUtilities - WebAssembly Type Utilities---*- C++ -*-====//
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

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WEBASSEMBLYTYPEUTILITIES_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WEBASSEMBLYTYPEUTILITIES_H

#include "MCTargetDesc/WebAssemblyMCTypeUtilities.h"
#include "WasmAddressSpaces.h"
#include "vm/core/BinaryFormat/Wasm.h"
#include "vm/core/CodeGenTypes/MachineValueType.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/MC/MCSymbolWasm.h"

namespace vm::core {

namespace WebAssembly {

/// Return true if this is a WebAssembly Externref Type.
inline bool isWebAssemblyExternrefType(const Type *Ty) {
  return Ty->isPointerTy() &&
         Ty->getPointerAddressSpace() ==
             WebAssembly::WasmAddressSpace::WASM_ADDRESS_SPACE_EXTERNREF;
}

/// Return true if this is a WebAssembly Funcref Type.
inline bool isWebAssemblyFuncrefType(const Type *Ty) {
  return Ty->isPointerTy() &&
         Ty->getPointerAddressSpace() ==
             WebAssembly::WasmAddressSpace::WASM_ADDRESS_SPACE_FUNCREF;
}

/// Return true if this is a WebAssembly Reference Type.
inline bool isWebAssemblyReferenceType(const Type *Ty) {
  return isWebAssemblyExternrefType(Ty) || isWebAssemblyFuncrefType(Ty);
}

/// Return true if the table represents a WebAssembly table type.
inline bool isWebAssemblyTableType(const Type *Ty) {
  return Ty->isArrayTy() &&
         isWebAssemblyReferenceType(Ty->getArrayElementType());
}

// Convert StringRef to ValType / HealType / BlockType

MVT parseMVT(StringRef Type);

// Convert a MVT into its corresponding wasm ValType.
wasm::ValType toValType(MVT Type);

/// Sets a Wasm Symbol Type.
void wasmSymbolSetType(MCSymbolWasm *Sym, const Type *GlobalVT,
                       ArrayRef<MVT> VTs);

} // end namespace WebAssembly
} // end namespace vm::core

#endif
