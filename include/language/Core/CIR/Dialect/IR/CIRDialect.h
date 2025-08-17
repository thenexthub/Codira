//===- CIRDialect.h - CIR dialect -------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file declares the CIR dialect.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_DIALECT_IR_CIRDIALECT_H
#define CLANG_CIR_DIALECT_IR_CIRDIALECT_H

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/LoopLikeInterface.h"
#include "mlir/Interfaces/MemorySlotInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "language/Core/CIR/Dialect/IR/CIRAttrs.h"
#include "language/Core/CIR/Dialect/IR/CIROpsDialect.h.inc"
#include "language/Core/CIR/Dialect/IR/CIROpsEnums.h"
#include "language/Core/CIR/Dialect/IR/CIRTypes.h"
#include "language/Core/CIR/Interfaces/CIRLoopOpInterface.h"
#include "language/Core/CIR/Interfaces/CIROpInterfaces.h"
#include "language/Core/CIR/MissingFeatures.h"

using BuilderCallbackRef =
    toolchain::function_ref<void(mlir::OpBuilder &, mlir::Location)>;
using BuilderOpStateCallbackRef = toolchain::function_ref<void(
    mlir::OpBuilder &, mlir::Location, mlir::OperationState &)>;

namespace cir {
void buildTerminatedBody(mlir::OpBuilder &builder, mlir::Location loc);
} // namespace cir

// TableGen'erated files for MLIR dialects require that a macro be defined when
// they are included.  GET_OP_CLASSES tells the file to define the classes for
// the operations of that dialect.
#define GET_OP_CLASSES
#include "language/Core/CIR/Dialect/IR/CIROps.h.inc"

#endif // CLANG_CIR_DIALECT_IR_CIRDIALECT_H
