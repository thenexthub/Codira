//===-- Utils.cpp ---------------------------------------------------------===//
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

#include "language/Compability/Optimizer/Support/Utils.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"

fir::TypeInfoOp fir::lookupTypeInfoOp(fir::RecordType recordType,
                                      mlir::ModuleOp module,
                                      const mlir::SymbolTable *symbolTable) {
  // fir.type_info was created with the mangled name of the derived type.
  // It is the same as the name in the related fir.type, except when a pass
  // lowered the fir.type (e.g., when lowering fir.boxproc type if the type has
  // pointer procedure components), in which case suffix may have been added to
  // the fir.type name. Get rid of them when looking up for the fir.type_info.
  toolchain::StringRef originalMangledTypeName =
      fir::NameUniquer::dropTypeConversionMarkers(recordType.getName());
  return fir::lookupTypeInfoOp(originalMangledTypeName, module, symbolTable);
}

fir::TypeInfoOp fir::lookupTypeInfoOp(toolchain::StringRef name,
                                      mlir::ModuleOp module,
                                      const mlir::SymbolTable *symbolTable) {
  if (symbolTable)
    if (auto typeInfo = symbolTable->lookup<fir::TypeInfoOp>(name))
      return typeInfo;
  return module.lookupSymbol<fir::TypeInfoOp>(name);
}

std::optional<toolchain::ArrayRef<int64_t>> fir::getComponentLowerBoundsIfNonDefault(
    fir::RecordType recordType, toolchain::StringRef component,
    mlir::ModuleOp module, const mlir::SymbolTable *symbolTable) {
  fir::TypeInfoOp typeInfo =
      fir::lookupTypeInfoOp(recordType, module, symbolTable);
  if (!typeInfo || typeInfo.getComponentInfo().empty())
    return std::nullopt;
  for (auto componentInfo :
       typeInfo.getComponentInfo().getOps<fir::DTComponentOp>())
    if (componentInfo.getName() == component)
      return componentInfo.getLowerBounds();
  return std::nullopt;
}
