//===--- GenClangType.cpp - Codira IR Generation For Types -----------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Wrapper functions for creating Clang types from Codira types.
//
//===----------------------------------------------------------------------===//

#include "IRGenModule.h"

#include "language/AST/ASTContext.h"
#include "language/AST/Types.h"

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/CanonicalType.h"
#include "language/Core/AST/Type.h"

using namespace language;
using namespace irgen;

language::Core::CanQualType IRGenModule::getClangType(CanType type) {
  auto *ty = type->getASTContext().getClangTypeForIRGen(type);
  return ty ? ty->getCanonicalTypeUnqualified() : language::Core::CanQualType();
}

language::Core::CanQualType IRGenModule::getClangType(SILType type) {
  if (type.isForeignReferenceType())
    return getClangType(type.getASTType()
                            ->wrapInPointer(PTK_UnsafePointer)
                            ->getCanonicalType());
  return getClangType(type.getASTType());
}

language::Core::CanQualType IRGenModule::getClangType(SILParameterInfo params,
                                             CanSILFunctionType funcTy) {
  auto paramTy = params.getSILStorageType(getSILModule(), funcTy,
                                          getMaximalTypeExpansionContext());
  auto clangType = getClangType(paramTy);
  // @block_storage types must be @inout_aliasable and have
  // special lowering
  if (!paramTy.is<SILBlockStorageType>()) {
    if (params.isIndirectMutating()) {
      return getClangASTContext().getPointerType(clangType);
    }
    if (params.isFormalIndirect() &&
        // Sensitive return types are represented as indirect return value in SIL,
        // but are returned as values (if small) in LLVM IR.
        !paramTy.isSensitive()) {
      auto constTy =
        getClangASTContext().getCanonicalType(clangType.withConst());
      return getClangASTContext().getPointerType(constTy);
    }
  }
  return clangType;
}
