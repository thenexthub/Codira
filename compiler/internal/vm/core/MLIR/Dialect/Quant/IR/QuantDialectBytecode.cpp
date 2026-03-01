//===- QuantDialectBytecode.cpp - Quant Bytecode Implementation
//------------===//
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

#include "QuantDialectBytecode.h"
#include "mlir/Bytecode/BytecodeImplementation.h"
#include "mlir/Dialect/Quant/IR/Quant.h"
#include "mlir/Dialect/Quant/IR/QuantTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "vm/core/ADT/APFloat.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::quant;

namespace {

static LogicalResult readDoubleAPFloat(DialectBytecodeReader &reader,
                                       double &val) {
  auto valOr =
      reader.readAPFloatWithKnownSemantics(toolchain::APFloat::IEEEdouble());
  if (failed(valOr))
    return failure();
  val = valOr->convertToDouble();
  return success();
}

#include "mlir/Dialect/Quant/IR/QuantDialectBytecode.cpp.inc"

/// This class implements the bytecode interface for the Quant dialect.
struct QuantDialectBytecodeInterface : public BytecodeDialectInterface {
  QuantDialectBytecodeInterface(Dialect *dialect)
      : BytecodeDialectInterface(dialect) {}

  //===--------------------------------------------------------------------===//
  // Attributes

  Attribute readAttribute(DialectBytecodeReader &reader) const override {
    return ::readAttribute(getContext(), reader);
  }

  LogicalResult writeAttribute(Attribute attr,
                               DialectBytecodeWriter &writer) const override {
    return ::writeAttribute(attr, writer);
  }

  //===--------------------------------------------------------------------===//
  // Types

  Type readType(DialectBytecodeReader &reader) const override {
    return ::readType(getContext(), reader);
  }

  LogicalResult writeType(Type type,
                          DialectBytecodeWriter &writer) const override {
    return ::writeType(type, writer);
  }
};
} // namespace

void quant::detail::addBytecodeInterface(QuantDialect *dialect) {
  dialect->addInterfaces<QuantDialectBytecodeInterface>();
}
