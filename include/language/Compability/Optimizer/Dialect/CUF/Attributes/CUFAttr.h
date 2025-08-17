//===-- Optimizer/Dialect/CUF/Attributes/CUFAttr.h -- CUF attributes ------===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_CUF_CUFATTR_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_CUF_CUFATTR_H

#include "language/Compability/Support/Fortran.h"
#include "mlir/IR/BuiltinAttributes.h"

namespace toolchain {
class StringRef;
}

#include "language/Compability/Optimizer/Dialect/CUF/Attributes/CUFEnumAttr.h.inc"

#define GET_ATTRDEF_CLASSES
#include "language/Compability/Optimizer/Dialect/CUF/Attributes/CUFAttr.h.inc"

namespace cuf {

/// Attribute to mark Fortran entities with the CUDA attribute.
static constexpr toolchain::StringRef getDataAttrName() { return "cuf.data_attr"; }
static constexpr toolchain::StringRef getProcAttrName() { return "cuf.proc_attr"; }

/// Attribute to carry CUDA launch_bounds values.
static constexpr toolchain::StringRef getLaunchBoundsAttrName() {
  return "cuf.launch_bounds";
}

/// Attribute to carry CUDA cluster_dims values.
static constexpr toolchain::StringRef getClusterDimsAttrName() {
  return "cuf.cluster_dims";
}

inline cuf::DataAttributeAttr
getDataAttribute(mlir::MLIRContext *mlirContext,
                 std::optional<language::Compability::common::CUDADataAttr> cudaAttr) {
  if (cudaAttr) {
    cuf::DataAttribute attr;
    switch (*cudaAttr) {
    case language::Compability::common::CUDADataAttr::Constant:
      attr = cuf::DataAttribute::Constant;
      break;
    case language::Compability::common::CUDADataAttr::Device:
      attr = cuf::DataAttribute::Device;
      break;
    case language::Compability::common::CUDADataAttr::Managed:
      attr = cuf::DataAttribute::Managed;
      break;
    case language::Compability::common::CUDADataAttr::Pinned:
      attr = cuf::DataAttribute::Pinned;
      break;
    case language::Compability::common::CUDADataAttr::Shared:
      attr = cuf::DataAttribute::Shared;
      break;
    case language::Compability::common::CUDADataAttr::Texture:
      // Obsolete attribute
      return {};
    case language::Compability::common::CUDADataAttr::Unified:
      attr = cuf::DataAttribute::Unified;
      break;
    }
    return cuf::DataAttributeAttr::get(mlirContext, attr);
  }
  return {};
}

inline cuf::ProcAttributeAttr
getProcAttribute(mlir::MLIRContext *mlirContext,
                 std::optional<language::Compability::common::CUDASubprogramAttrs> cudaAttr) {
  if (cudaAttr) {
    cuf::ProcAttribute attr;
    switch (*cudaAttr) {
    case language::Compability::common::CUDASubprogramAttrs::Host:
      attr = cuf::ProcAttribute::Host;
      break;
    case language::Compability::common::CUDASubprogramAttrs::Device:
      attr = cuf::ProcAttribute::Device;
      break;
    case language::Compability::common::CUDASubprogramAttrs::HostDevice:
      attr = cuf::ProcAttribute::HostDevice;
      break;
    case language::Compability::common::CUDASubprogramAttrs::Global:
      attr = cuf::ProcAttribute::Global;
      break;
    case language::Compability::common::CUDASubprogramAttrs::Grid_Global:
      attr = cuf::ProcAttribute::GridGlobal;
      break;
    }
    return cuf::ProcAttributeAttr::get(mlirContext, attr);
  }
  return {};
}

} // namespace cuf

#endif // FORTRAN_OPTIMIZER_DIALECT_CUF_CUFATTR_H
