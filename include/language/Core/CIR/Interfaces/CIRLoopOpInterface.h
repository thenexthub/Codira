//===----------------------------------------------------------------------===//
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
//===---------------------------------------------------------------------===//
//
// Defines the interface to generically handle CIR loop operations.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_INTERFACES_CIRLOOPOPINTERFACE_H
#define CLANG_CIR_INTERFACES_CIRLOOPOPINTERFACE_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/Operation.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/LoopLikeInterface.h"

namespace cir {
namespace detail {

/// Verify invariants of the LoopOpInterface.
mlir::LogicalResult verifyLoopOpInterface(::mlir::Operation *op);

} // namespace detail
} // namespace cir

/// Include the tablegen'd interface declarations.
#include "language/Core/CIR/Interfaces/CIRLoopOpInterface.h.inc"

#endif // CLANG_CIR_INTERFACES_CIRLOOPOPINTERFACE_H
