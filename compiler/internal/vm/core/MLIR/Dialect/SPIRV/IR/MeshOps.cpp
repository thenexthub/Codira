//===- MeshOps.cpp - MLIR SPIR-V Mesh Ops  --------------------------------===//
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
//
// Defines the mesh operations in the SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/IR/SPIRVEnums.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVTypes.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// spirv.EXT.EmitMeshTasks
//===----------------------------------------------------------------------===//

LogicalResult spirv::EXTEmitMeshTasksOp::verify() {
  if (Value payload = getPayload()) {
    // The operand definition restricts type to be SPIRV_AnyPointer, so we can
    // cast here safely.
    auto payloadType = cast<spirv::PointerType>(payload.getType());
    if (payloadType.getStorageClass() !=
        spirv::StorageClass::TaskPayloadWorkgroupEXT)
      return emitOpError("payload must be a variable with a storage class of "
                         "TaskPayloadWorkgroupEXT");
  }
  return success();
}
