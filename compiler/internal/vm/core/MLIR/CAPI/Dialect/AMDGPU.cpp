//===- AMDGPU.cpp - C Interface for AMDGPU dialect ------------------===//
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

#include "mlir-c/Dialect/AMDGPU.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/AMDGPU/IR/AMDGPUDialect.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(AMDGPU, amdgpu,
                                      mlir::amdgpu::AMDGPUDialect)

using namespace mlir;
using namespace mlir::amdgpu;

//===---------------------------------------------------------------------===//
// TDMBaseType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAAMDGPUTDMBaseType(MlirType type) {
  return isa<amdgpu::TDMBaseType>(unwrap(type));
}

MlirTypeID mlirAMDGPUTDMBaseTypeGetTypeID() {
  return wrap(amdgpu::TDMBaseType::getTypeID());
}

MlirType mlirAMDGPUTDMBaseTypeGet(MlirContext ctx, MlirType elementType) {
  return wrap(amdgpu::TDMBaseType::get(unwrap(ctx), unwrap(elementType)));
}

MlirStringRef mlirAMDGPUTDMBaseTypeGetName(void) {
  return wrap(amdgpu::TDMBaseType::name);
}

//===---------------------------------------------------------------------===//
// TDMDescriptorType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAAMDGPUTDMDescriptorType(MlirType type) {
  return isa<amdgpu::TDMDescriptorType>(unwrap(type));
}

MlirTypeID mlirAMDGPUTDMDescriptorTypeGetTypeID() {
  return wrap(amdgpu::TDMDescriptorType::getTypeID());
}

MlirType mlirAMDGPUTDMDescriptorTypeGet(MlirContext ctx) {
  return wrap(amdgpu::TDMDescriptorType::get(unwrap(ctx)));
}

MlirStringRef mlirAMDGPUTDMDescriptorTypeGetName(void) {
  return wrap(amdgpu::TDMDescriptorType::name);
}

//===---------------------------------------------------------------------===//
// TDMGatherBaseType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAAMDGPUTDMGatherBaseType(MlirType type) {
  return isa<amdgpu::TDMGatherBaseType>(unwrap(type));
}

MlirTypeID mlirAMDGPUTDMGatherBaseTypeGetTypeID() {
  return wrap(amdgpu::TDMGatherBaseType::getTypeID());
}

MlirType mlirAMDGPUTDMGatherBaseTypeGet(MlirContext ctx, MlirType elementType,
                                        MlirType indexType) {
  return wrap(amdgpu::TDMGatherBaseType::get(unwrap(ctx), unwrap(elementType),
                                             unwrap(indexType)));
}

MlirStringRef mlirAMDGPUTDMGatherBaseTypeGetName(void) {
  return wrap(amdgpu::TDMGatherBaseType::name);
}
