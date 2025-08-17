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

#ifndef CLANG_LIB_CIR_CODEGEN_CIRGENVTABLES_H
#define CLANG_LIB_CIR_CODEGEN_CIRGENVTABLES_H

#include "mlir/IR/Types.h"
#include "language/Core/AST/GlobalDecl.h"
#include "language/Core/AST/VTableBuilder.h"

namespace language::Core {
class CXXRecordDecl;
}

namespace language::Core::CIRGen {
class CIRGenModule;

class CIRGenVTables {
  CIRGenModule &cgm;

  language::Core::VTableContextBase *vtContext;

  mlir::Type getVTableComponentType();

public:
  CIRGenVTables(CIRGenModule &cgm);

  language::Core::ItaniumVTableContext &getItaniumVTableContext() {
    return *toolchain::cast<language::Core::ItaniumVTableContext>(vtContext);
  }

  const language::Core::ItaniumVTableContext &getItaniumVTableContext() const {
    return *toolchain::cast<language::Core::ItaniumVTableContext>(vtContext);
  }

  /// Returns the type of a vtable with the given layout. Normally a struct of
  /// arrays of pointers, with one struct element for each vtable in the vtable
  /// group.
  mlir::Type getVTableType(const language::Core::VTableLayout &layout);
};

} // namespace language::Core::CIRGen

#endif // CLANG_LIB_CIR_CODEGEN_CIRGENVTABLES_H
