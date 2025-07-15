//===--- GenDecl.h - Codira IR generation for some decl ----------*- C++ -*-===//
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
//  This file provides the private interface to some decl emission code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENDECL_H
#define LANGUAGE_IRGEN_GENDECL_H

#include "DebugTypeInfo.h"
#include "IRGen.h"
#include "language/Basic/OptimizationMode.h"
#include "language/SIL/SILLocation.h"
#include "clang/AST/DeclCXX.h"
#include "toolchain/IR/CallingConv.h"
#include "toolchain/Support/CommandLine.h"

namespace toolchain {
  class AttributeList;
  class Function;
  class FunctionType;
  class CallBase;
}
namespace language {
namespace irgen {
  class IRGenModule;
  class LinkEntity;
  class LinkInfo;
  class Signature;

  void updateLinkageForDefinition(IRGenModule &IGM,
                                  toolchain::GlobalValue *global,
                                  const LinkEntity &entity);

  toolchain::Function *createFunction(
      IRGenModule &IGM, LinkInfo &linkInfo, const Signature &signature,
      toolchain::Function *insertBefore = nullptr,
      OptimizationMode FuncOptMode = OptimizationMode::NotSet,
      StackProtectorMode stackProtect = StackProtectorMode::NoStackProtector);

  toolchain::GlobalVariable *
  createVariable(IRGenModule &IGM, LinkInfo &linkInfo, toolchain::Type *objectType,
                 Alignment alignment, DebugTypeInfo DebugType = DebugTypeInfo(),
                 std::optional<SILLocation> DebugLoc = std::nullopt,
                 StringRef DebugName = StringRef());

  toolchain::GlobalVariable *
  createLinkerDirectiveVariable(IRGenModule &IGM, StringRef Name);

  void disableAddressSanitizer(IRGenModule &IGM, toolchain::GlobalVariable *var);

  /// If the calling convention for `ctor` doesn't match the calling convention
  /// that we assumed for it when we imported it as `initializer`, emit and
  /// return a thunk that conforms to the assumed calling convention. The thunk
  /// is marked `alwaysinline`, so it doesn't generate any runtime overhead.
  /// If the assumed calling convention was correct, just return `ctor`.
  ///
  /// See also comments in CXXMethodConventions in SIL/IR/SILFunctionType.cpp.
  toolchain::Constant *
  emitCXXConstructorThunkIfNeeded(IRGenModule &IGM, Signature signature,
                                  const clang::CXXConstructorDecl *ctor,
                                  StringRef name, toolchain::Constant *ctorAddress);

  toolchain::CallBase *emitCXXConstructorCall(IRGenFunction &IGF,
                                         const clang::CXXConstructorDecl *ctor,
                                         toolchain::FunctionType *ctorFnType,
                                         toolchain::Constant *ctorAddress,
                                         toolchain::ArrayRef<toolchain::Value *> args);

  bool hasValidSignatureForEmbedded(SILFunction *f);
}
}

extern toolchain::cl::opt<bool> UseBasicDynamicReplacement;

#endif
