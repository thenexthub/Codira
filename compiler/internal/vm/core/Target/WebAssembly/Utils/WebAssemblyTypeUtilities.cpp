//===-- WebAssemblyTypeUtilities.cpp - WebAssembly Type Utility Functions -===//
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
/// This file implements several utility functions for WebAssembly type parsing.
///
//===----------------------------------------------------------------------===//

#include "WebAssemblyTypeUtilities.h"
#include "vm/core/ADT/StringSwitch.h"

// Get register classes enum.
#define GET_REGINFO_ENUM
#include "WebAssemblyGenRegisterInfo.inc"

using namespace vm::core;

MVT WebAssembly::parseMVT(StringRef Type) {
  return StringSwitch<MVT>(Type)
      .Case("i32", MVT::i32)
      .Case("i64", MVT::i64)
      .Case("f32", MVT::f32)
      .Case("f64", MVT::f64)
      .Case("i64", MVT::i64)
      .Case("v16i8", MVT::v16i8)
      .Case("v8i16", MVT::v8i16)
      .Case("v4i32", MVT::v4i32)
      .Case("v2i64", MVT::v2i64)
      .Case("funcref", MVT::funcref)
      .Case("externref", MVT::externref)
      .Case("exnref", MVT::exnref)
      .Default(MVT::INVALID_SIMPLE_VALUE_TYPE);
}

wasm::ValType WebAssembly::toValType(MVT Type) {
  switch (Type.SimpleTy) {
  case MVT::i32:
    return wasm::ValType::I32;
  case MVT::i64:
    return wasm::ValType::I64;
  case MVT::f32:
    return wasm::ValType::F32;
  case MVT::f64:
    return wasm::ValType::F64;
  case MVT::v16i8:
  case MVT::v8i16:
  case MVT::v4i32:
  case MVT::v2i64:
  case MVT::v8f16:
  case MVT::v4f32:
  case MVT::v2f64:
    return wasm::ValType::V128;
  case MVT::funcref:
    return wasm::ValType::FUNCREF;
  case MVT::externref:
    return wasm::ValType::EXTERNREF;
  case MVT::exnref:
    return wasm::ValType::EXNREF;
  default:
    llvm_unreachable("unexpected type");
  }
}

void WebAssembly::wasmSymbolSetType(MCSymbolWasm *Sym, const Type *GlobalVT,
                                    ArrayRef<MVT> VTs) {
  assert(!Sym->getType());

  // Tables are represented as Arrays in LLVM IR therefore
  // they reach this point as aggregate Array types with an element type
  // that is a reference type.
  wasm::ValType ValTy;
  bool IsTable = false;
  if (WebAssembly::isWebAssemblyTableType(GlobalVT)) {
    IsTable = true;
    const Type *ElTy = GlobalVT->getArrayElementType();
    if (WebAssembly::isWebAssemblyExternrefType(ElTy))
      ValTy = wasm::ValType::EXTERNREF;
    else if (WebAssembly::isWebAssemblyFuncrefType(ElTy))
      ValTy = wasm::ValType::FUNCREF;
    else
      report_fatal_error("unhandled reference type");
  } else if (VTs.size() == 1) {
    ValTy = WebAssembly::toValType(VTs[0]);
  } else
    report_fatal_error("Aggregate globals not yet implemented");

  if (IsTable) {
    Sym->setType(wasm::WASM_SYMBOL_TYPE_TABLE);
    Sym->setTableType(ValTy);
  } else {
    Sym->setType(wasm::WASM_SYMBOL_TYPE_GLOBAL);
    Sym->setGlobalType(wasm::WasmGlobalType{uint8_t(ValTy), /*Mutable=*/true});
  }
}
