//===- Serialization.cpp - MLIR SPIR-V Serialization ----------------------===//
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
// This file defines the MLIR SPIR-V module to SPIR-V binary serialization entry
// point.
//
//===----------------------------------------------------------------------===//

#include "Serializer.h"

#include "mlir/Target/SPIRV/Serialization.h"

#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "spirv-serialization"

namespace mlir {
LogicalResult spirv::serialize(spirv::ModuleOp module,
                               SmallVectorImpl<uint32_t> &binary,
                               const SerializationOptions &options) {
  if (!module.getVceTriple())
    return module.emitError(
        "module must have 'vce_triple' attribute to be serializeable");

  Serializer serializer(module, options);

  if (failed(serializer.serialize()))
    return failure();

  LLVM_DEBUG(serializer.printValueIDMap(toolchain::dbgs()));

  serializer.collect(binary);
  return success();
}
} // namespace mlir
