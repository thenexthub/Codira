//===- MLProgramDialect.cpp - MLProgram dialect implementation ------------===//
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

#include "mlir/Dialect/MLProgram/IR/MLProgram.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/Transforms/InliningUtils.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::ml_program;

//===----------------------------------------------------------------------===//
/// Tablegen Definitions
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/MLProgram/IR/MLProgramOpsDialect.cpp.inc"
#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/MLProgram/IR/MLProgramAttributes.cpp.inc"
#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/MLProgram/IR/MLProgramTypes.cpp.inc"

namespace {

struct MLProgramInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  bool isLegalToInline(Operation *, Region *, bool,
                       IRMapping &) const override {
    // We have no specific opinion on whether ops defined in this dialect should
    // be inlined.
    return true;
  }
};

struct MLProgramOpAsmDialectInterface : public OpAsmDialectInterface {
  using OpAsmDialectInterface::OpAsmDialectInterface;
};
} // namespace

void ml_program::MLProgramDialect::initialize() {
#define GET_ATTRDEF_LIST
  addAttributes<
#include "mlir/Dialect/MLProgram/IR/MLProgramAttributes.cpp.inc"
      >();

#define GET_TYPEDEF_LIST
  addTypes<
#include "mlir/Dialect/MLProgram/IR/MLProgramTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/MLProgram/IR/MLProgramOps.cpp.inc"
      >();

  addInterfaces<MLProgramInlinerInterface, MLProgramOpAsmDialectInterface>();
}
