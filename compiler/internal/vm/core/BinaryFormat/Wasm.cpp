//===-- toolchain/BinaryFormat/Wasm.cpp -------------------------------*- C++-*-===//
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

#include "vm/core/BinaryFormat/Wasm.h"

toolchain::StringRef toolchain::wasm::toString(wasm::WasmSymbolType Type) {
  switch (Type) {
  case wasm::WASM_SYMBOL_TYPE_FUNCTION:
    return "WASM_SYMBOL_TYPE_FUNCTION";
  case wasm::WASM_SYMBOL_TYPE_GLOBAL:
    return "WASM_SYMBOL_TYPE_GLOBAL";
  case wasm::WASM_SYMBOL_TYPE_TABLE:
    return "WASM_SYMBOL_TYPE_TABLE";
  case wasm::WASM_SYMBOL_TYPE_DATA:
    return "WASM_SYMBOL_TYPE_DATA";
  case wasm::WASM_SYMBOL_TYPE_SECTION:
    return "WASM_SYMBOL_TYPE_SECTION";
  case wasm::WASM_SYMBOL_TYPE_TAG:
    return "WASM_SYMBOL_TYPE_TAG";
  }
  llvm_unreachable("unknown symbol type");
}

toolchain::StringRef toolchain::wasm::relocTypetoString(uint32_t Type) {
  switch (Type) {
#define WASM_RELOC(NAME, VALUE)                                                \
  case VALUE:                                                                  \
    return #NAME;
#include "vm/core/BinaryFormat/WasmRelocs.def"
#undef WASM_RELOC
  default:
    llvm_unreachable("unknown reloc type");
  }
}

toolchain::StringRef toolchain::wasm::sectionTypeToString(uint32_t Type) {
#define ECase(X)                                                               \
  case wasm::WASM_SEC_##X:                                                     \
    return #X;
  switch (Type) {
    ECase(CUSTOM);
    ECase(TYPE);
    ECase(IMPORT);
    ECase(FUNCTION);
    ECase(TABLE);
    ECase(MEMORY);
    ECase(GLOBAL);
    ECase(EXPORT);
    ECase(START);
    ECase(ELEM);
    ECase(CODE);
    ECase(DATA);
    ECase(DATACOUNT);
    ECase(TAG);
  default:
    llvm_unreachable("unknown section type");
  }
#undef ECase
}

bool toolchain::wasm::relocTypeHasAddend(uint32_t Type) {
  switch (Type) {
  case R_WASM_MEMORY_ADDR_LEB:
  case R_WASM_MEMORY_ADDR_LEB64:
  case R_WASM_MEMORY_ADDR_SLEB:
  case R_WASM_MEMORY_ADDR_SLEB64:
  case R_WASM_MEMORY_ADDR_REL_SLEB:
  case R_WASM_MEMORY_ADDR_REL_SLEB64:
  case R_WASM_MEMORY_ADDR_I32:
  case R_WASM_MEMORY_ADDR_I64:
  case R_WASM_MEMORY_ADDR_TLS_SLEB:
  case R_WASM_MEMORY_ADDR_TLS_SLEB64:
  case R_WASM_FUNCTION_OFFSET_I32:
  case R_WASM_FUNCTION_OFFSET_I64:
  case R_WASM_SECTION_OFFSET_I32:
  case R_WASM_MEMORY_ADDR_LOCREL_I32:
    return true;
  default:
    return false;
  }
}
