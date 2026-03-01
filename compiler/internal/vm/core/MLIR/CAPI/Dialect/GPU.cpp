//===- GPU.cpp - C Interface for GPU dialect ------------------------------===//
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

#include "mlir-c/Dialect/GPU.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "vm/core/Support/Casting.h"

using namespace mlir;

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(GPU, gpu, gpu::GPUDialect)

//===-------------------------------------------------------------------===//
// AsyncTokenType
//===-------------------------------------------------------------------===//

bool mlirTypeIsAGPUAsyncTokenType(MlirType type) {
  return isa<gpu::AsyncTokenType>(unwrap(type));
}

MlirType mlirGPUAsyncTokenTypeGet(MlirContext ctx) {
  return wrap(gpu::AsyncTokenType::get(unwrap(ctx)));
}

MlirStringRef mlirGPUAsyncTokenTypeGetName(void) {
  return wrap(gpu::AsyncTokenType::name);
}

//===---------------------------------------------------------------------===//
// ObjectAttr
//===---------------------------------------------------------------------===//

bool mlirAttributeIsAGPUObjectAttr(MlirAttribute attr) {
  return toolchain::isa<gpu::ObjectAttr>(unwrap(attr));
}

MlirAttribute mlirGPUObjectAttrGet(MlirContext mlirCtx, MlirAttribute target,
                                   uint32_t format, MlirStringRef objectStrRef,
                                   MlirAttribute mlirObjectProps) {
  MLIRContext *ctx = unwrap(mlirCtx);
  toolchain::StringRef object = unwrap(objectStrRef);
  DictionaryAttr objectProps;
  if (mlirObjectProps.ptr != nullptr)
    objectProps = toolchain::cast<DictionaryAttr>(unwrap(mlirObjectProps));
  return wrap(gpu::ObjectAttr::get(
      ctx, unwrap(target), static_cast<gpu::CompilationTarget>(format),
      StringAttr::get(ctx, object), objectProps, nullptr));
}

MlirStringRef mlirGPUObjectAttrGetName(void) {
  return wrap(gpu::ObjectAttr::name);
}

MlirAttribute mlirGPUObjectAttrGetWithKernels(MlirContext mlirCtx,
                                              MlirAttribute target,
                                              uint32_t format,
                                              MlirStringRef objectStrRef,
                                              MlirAttribute mlirObjectProps,
                                              MlirAttribute mlirKernelsAttr) {
  MLIRContext *ctx = unwrap(mlirCtx);
  toolchain::StringRef object = unwrap(objectStrRef);
  DictionaryAttr objectProps;
  if (mlirObjectProps.ptr != nullptr)
    objectProps = toolchain::cast<DictionaryAttr>(unwrap(mlirObjectProps));
  gpu::KernelTableAttr kernels;
  if (mlirKernelsAttr.ptr != nullptr)
    kernels = toolchain::cast<gpu::KernelTableAttr>(unwrap(mlirKernelsAttr));
  return wrap(gpu::ObjectAttr::get(
      ctx, unwrap(target), static_cast<gpu::CompilationTarget>(format),
      StringAttr::get(ctx, object), objectProps, kernels));
}

MlirAttribute mlirGPUObjectAttrGetTarget(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  return wrap(objectAttr.getTarget());
}

uint32_t mlirGPUObjectAttrGetFormat(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  return static_cast<uint32_t>(objectAttr.getFormat());
}

MlirStringRef mlirGPUObjectAttrGetObject(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  toolchain::StringRef object = objectAttr.getObject();
  return mlirStringRefCreate(object.data(), object.size());
}

bool mlirGPUObjectAttrHasProperties(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  return objectAttr.getProperties() != nullptr;
}

MlirAttribute mlirGPUObjectAttrGetProperties(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  return wrap(objectAttr.getProperties());
}

bool mlirGPUObjectAttrHasKernels(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  return objectAttr.getKernels() != nullptr;
}

MlirAttribute mlirGPUObjectAttrGetKernels(MlirAttribute mlirObjectAttr) {
  gpu::ObjectAttr objectAttr =
      toolchain::cast<gpu::ObjectAttr>(unwrap(mlirObjectAttr));
  return wrap(objectAttr.getKernels());
}
