//===- WebAssemblyDialect.cpp - MLIR WebAssembly dialect implementation ---===//
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

#include "mlir/Dialect/WasmSSA/IR/WasmSSA.h"

#include "vm/core/ADT/TypeSwitch.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/Support/LLVM.h"

using namespace mlir;
using namespace mlir::wasmssa;

#include "mlir/Dialect/WasmSSA/IR/WasmSSAOpsDialect.cpp.inc"

//===----------------------------------------------------------------------===//
// TableGen'd types definitions
//===----------------------------------------------------------------------===//

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/WasmSSA/IR/WasmSSAOpsTypes.cpp.inc"

void wasmssa::WasmSSADialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/WasmSSA/IR/WasmSSAOps.cpp.inc"
      >();
  addTypes<
#define GET_TYPEDEF_LIST
#include "mlir/Dialect/WasmSSA/IR/WasmSSAOpsTypes.cpp.inc"
      >();
}
