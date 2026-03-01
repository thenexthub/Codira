//===- NVGPU.cpp - C Interface for NVGPU dialect ------------------===//
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

#include "mlir-c/Dialect/NVGPU.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/NVGPU/IR/NVGPUDialect.h"
#include "mlir/IR/BuiltinTypes.h"

using namespace mlir;
using namespace mlir::nvgpu;

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(NVGPU, nvgpu, mlir::nvgpu::NVGPUDialect)

bool mlirTypeIsANVGPUTensorMapDescriptorType(MlirType type) {
  return isa<nvgpu::TensorMapDescriptorType>(unwrap(type));
}

MlirType mlirNVGPUTensorMapDescriptorTypeGet(MlirContext ctx,
                                             MlirType tensorMemrefType,
                                             int swizzle, int l2promo,
                                             int oobFill, int interleave) {
  return wrap(nvgpu::TensorMapDescriptorType::get(
      unwrap(ctx), cast<MemRefType>(unwrap(tensorMemrefType)),
      TensorMapSwizzleKind(swizzle), TensorMapL2PromoKind(l2promo),
      TensorMapOOBKind(oobFill), TensorMapInterleaveKind(interleave)));
}

MlirStringRef mlirNVGPUTensorMapDescriptorTypeGetName(void) {
  return wrap(nvgpu::TensorMapDescriptorType::name);
}
