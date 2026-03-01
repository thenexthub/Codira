//===- SPIRVOpUtils.h - MLIR SPIR-V Dialect Op Definition Utilities -------===//
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

#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"

namespace mlir::spirv {

/// Returns the bit width of the `type`.
inline unsigned getBitWidth(Type type) {
  if (isa<spirv::PointerType>(type)) {
    // Just return 64 bits for pointer types for now.
    // TODO: Make sure not caller relies on the actual pointer width value.
    return 64;
  }

  if (type.isIntOrFloat())
    return type.getIntOrFloatBitWidth();

  if (auto vectorType = dyn_cast<VectorType>(type)) {
    assert(vectorType.getElementType().isIntOrFloat());
    return vectorType.getNumElements() *
           vectorType.getElementType().getIntOrFloatBitWidth();
  }
  llvm_unreachable("unhandled bit width computation for type");
}

void printVariableDecorations(Operation *op, OpAsmPrinter &printer,
                              SmallVectorImpl<StringRef> &elidedAttrs);

LogicalResult extractValueFromConstOp(Operation *op, int32_t &value);

LogicalResult verifyMemorySemantics(Operation *op,
                                    spirv::MemorySemantics memorySemantics);

} // namespace mlir::spirv
