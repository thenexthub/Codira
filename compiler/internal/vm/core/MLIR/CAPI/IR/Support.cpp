//===- Support.cpp - Helpers for C interface to MLIR API ------------------===//
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

#include "mlir/CAPI/Support.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/ThreadPool.h"

#include <cstring>

MlirStringRef mlirStringRefCreateFromCString(const char *str) {
  return mlirStringRefCreate(str, strlen(str));
}

bool mlirStringRefEqual(MlirStringRef string, MlirStringRef other) {
  return toolchain::StringRef(string.data, string.length) ==
         toolchain::StringRef(other.data, other.length);
}

//===----------------------------------------------------------------------===//
// LLVM ThreadPool API.
//===----------------------------------------------------------------------===//
MlirLlvmThreadPool mlirLlvmThreadPoolCreate() {
  return wrap(new toolchain::DefaultThreadPool());
}

void mlirLlvmThreadPoolDestroy(MlirLlvmThreadPool threadPool) {
  delete unwrap(threadPool);
}

//===----------------------------------------------------------------------===//
// TypeID API.
//===----------------------------------------------------------------------===//
MlirTypeID mlirTypeIDCreate(const void *ptr) {
  assert(reinterpret_cast<uintptr_t>(ptr) % 8 == 0 &&
         "ptr must be 8 byte aligned");
  // This is essentially a no-op that returns back `ptr`, but by going through
  // the `TypeID` functions we can get compiler errors in case the `TypeID`
  // api/representation changes
  return wrap(mlir::TypeID::getFromOpaquePointer(ptr));
}

bool mlirTypeIDEqual(MlirTypeID typeID1, MlirTypeID typeID2) {
  return unwrap(typeID1) == unwrap(typeID2);
}

size_t mlirTypeIDHashValue(MlirTypeID typeID) {
  return hash_value(unwrap(typeID));
}

//===----------------------------------------------------------------------===//
// TypeIDAllocator API.
//===----------------------------------------------------------------------===//

MlirTypeIDAllocator mlirTypeIDAllocatorCreate() {
  return wrap(new mlir::TypeIDAllocator());
}

void mlirTypeIDAllocatorDestroy(MlirTypeIDAllocator allocator) {
  delete unwrap(allocator);
}

MlirTypeID mlirTypeIDAllocatorAllocateTypeID(MlirTypeIDAllocator allocator) {
  return wrap(unwrap(allocator)->allocate());
}
