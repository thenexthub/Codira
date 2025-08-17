//===-- Lower/CUDA.h -- CUDA Fortran utilities ------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CUDA_H
#define LANGUAGE_COMPABILITY_LOWER_CUDA_H

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/MutableBox.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Runtime/allocator-registry-consts.h"
#include "language/Compability/Semantics/tools.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"

namespace mlir {
class Value;
class Location;
class MLIRContext;
} // namespace mlir

namespace language::Compability::lower {

class AbstractConverter;

static inline unsigned getAllocatorIdx(const language::Compability::semantics::Symbol &sym) {
  std::optional<language::Compability::common::CUDADataAttr> cudaAttr =
      language::Compability::semantics::GetCUDADataAttr(&sym.GetUltimate());
  if (cudaAttr) {
    if (*cudaAttr == language::Compability::common::CUDADataAttr::Pinned)
      return kPinnedAllocatorPos;
    if (*cudaAttr == language::Compability::common::CUDADataAttr::Device)
      return kDeviceAllocatorPos;
    if (*cudaAttr == language::Compability::common::CUDADataAttr::Managed)
      return kManagedAllocatorPos;
    if (*cudaAttr == language::Compability::common::CUDADataAttr::Unified)
      return kUnifiedAllocatorPos;
  }
  return kDefaultAllocator;
}

void initializeDeviceComponentAllocator(
    language::Compability::lower::AbstractConverter &converter,
    const language::Compability::semantics::Symbol &sym, const fir::MutableBoxValue &box);

mlir::Type gatherDeviceComponentCoordinatesAndType(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const language::Compability::semantics::Symbol &sym, fir::RecordType recTy,
    toolchain::SmallVector<mlir::Value> &coordinates);

/// Translate the CUDA Fortran attributes of \p sym into the FIR CUDA attribute
/// representation.
cuf::DataAttributeAttr
translateSymbolCUFDataAttribute(mlir::MLIRContext *mlirContext,
                                const language::Compability::semantics::Symbol &sym);

bool isTransferWithConversion(mlir::Value rhs);

} // end namespace language::Compability::lower

#endif // FORTRAN_LOWER_CUDA_H
