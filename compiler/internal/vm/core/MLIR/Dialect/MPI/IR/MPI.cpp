//===- MPI.cpp - MPI dialect implementation -------------------------------===//
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

#include "mlir/Dialect/MPI/IR/MPI.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::mpi;

//===----------------------------------------------------------------------===//
/// Tablegen Definitions
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/MPI/IR/MPI.cpp.inc"

#include "mlir/Dialect/MPI/IR/MPIDialect.cpp.inc"

void MPIDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/MPI/IR/MPIOps.cpp.inc"
      >();

  addTypes<
#define GET_TYPEDEF_LIST
#include "mlir/Dialect/MPI/IR/MPITypesGen.cpp.inc"
      >();

  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/Dialect/MPI/IR/MPIAttrDefs.cpp.inc"
      >();
}

//===----------------------------------------------------------------------===//
// TableGen'd dialect, type, and op definitions
//===----------------------------------------------------------------------===//

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/MPI/IR/MPITypesGen.cpp.inc"

#include "mlir/Dialect/MPI/IR/MPIEnums.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/MPI/IR/MPIAttrDefs.cpp.inc"
