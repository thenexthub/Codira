//===----------------------------------------------------------------------===//
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
// This contains code dealing with C++ code generation of virtual tables.
//
//===----------------------------------------------------------------------===//

#include "CIRGenVTables.h"
#include "CIRGenModule.h"
#include "mlir/IR/Types.h"
#include "language/Core/AST/VTableBuilder.h"
#include "toolchain/ADT/SmallVector.h"

using namespace toolchain;
using namespace language::Core;
using namespace language::Core::CIRGen;

CIRGenVTables::CIRGenVTables(CIRGenModule &cgm)
    : cgm(cgm), vtContext(cgm.getASTContext().getVTableContext()) {}

mlir::Type CIRGenModule::getVTableComponentType() {
  mlir::Type ptrTy = builder.getUInt8PtrTy();
  assert(!cir::MissingFeatures::vtableRelativeLayout());
  return ptrTy;
}

mlir::Type CIRGenVTables::getVTableComponentType() {
  return cgm.getVTableComponentType();
}

mlir::Type CIRGenVTables::getVTableType(const VTableLayout &layout) {
  SmallVector<mlir::Type, 4> tys;
  auto componentType = getVTableComponentType();
  for (unsigned i = 0, e = layout.getNumVTables(); i != e; ++i)
    tys.push_back(cir::ArrayType::get(componentType, layout.getVTableSize(i)));

  // FIXME(cir): should VTableLayout be encoded like we do for some
  // AST nodes?
  return cgm.getBuilder().getAnonRecordTy(tys, /*incomplete=*/false);
}
