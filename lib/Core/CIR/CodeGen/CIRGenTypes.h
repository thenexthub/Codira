//===--- CIRGenTypes.h - Type translation for CIR CodeGen -------*- C++ -*-===//
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
// This is the code that handles AST -> CIR type lowering.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CODEGENTYPES_H
#define LANGUAGE_CORE_LIB_CODEGEN_CODEGENTYPES_H

#include "ABIInfo.h"
#include "CIRGenFunctionInfo.h"
#include "CIRGenRecordLayout.h"

#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/ABI.h"
#include "language/Core/CIR/Dialect/IR/CIRTypes.h"

#include "toolchain/ADT/SmallPtrSet.h"

namespace language::Core {
class ASTContext;
class FunctionType;
class GlobalDecl;
class QualType;
class Type;
} // namespace language::Core

namespace mlir {
class Type;
}

namespace language::Core::CIRGen {

class CallArgList;
class CIRGenBuilderTy;
class CIRGenCXXABI;
class CIRGenModule;

/// This class organizes the cross-module state that is used while lowering
/// AST types to CIR types.
class CIRGenTypes {
  CIRGenModule &cgm;
  language::Core::ASTContext &astContext;
  CIRGenBuilderTy &builder;
  CIRGenCXXABI &theCXXABI;

  const ABIInfo &theABIInfo;

  /// Contains the CIR type for any converted RecordDecl.
  toolchain::DenseMap<const language::Core::Type *, std::unique_ptr<CIRGenRecordLayout>>
      cirGenRecordLayouts;

  /// Contains the CIR type for any converted RecordDecl
  toolchain::DenseMap<const language::Core::Type *, cir::RecordType> recordDeclTypes;

  /// Hold memoized CIRGenFunctionInfo results
  toolchain::FoldingSet<CIRGenFunctionInfo> functionInfos;

  /// This set keeps track of records that we're currently converting to a CIR
  /// type. For example, when converting:
  /// struct A { struct B { int x; } } when processing 'x', the 'A' and 'B'
  /// types will be in this set.
  toolchain::SmallPtrSet<const language::Core::Type *, 4> recordsBeingLaidOut;

  toolchain::SmallVector<const language::Core::RecordDecl *, 8> deferredRecords;

  /// Heper for convertType.
  mlir::Type convertFunctionTypeInternal(language::Core::QualType ft);

public:
  CIRGenTypes(CIRGenModule &cgm);
  ~CIRGenTypes();

  CIRGenBuilderTy &getBuilder() const { return builder; }
  CIRGenModule &getCGModule() const { return cgm; }

  /// Utility to check whether a function type can be converted to a CIR type
  /// (i.e. doesn't depend on an incomplete tag type).
  bool isFuncTypeConvertible(const language::Core::FunctionType *ft);
  bool isFuncParamTypeConvertible(language::Core::QualType type);

  /// Derives the 'this' type for CIRGen purposes, i.e. ignoring method CVR
  /// qualification.
  language::Core::CanQualType deriveThisType(const language::Core::CXXRecordDecl *rd,
                                    const language::Core::CXXMethodDecl *md);

  /// This map of language::Core::Type to mlir::Type (which includes CIR type) is a
  /// cache of types that have already been processed.
  using TypeCacheTy = toolchain::DenseMap<const language::Core::Type *, mlir::Type>;
  TypeCacheTy typeCache;

  mlir::MLIRContext &getMLIRContext() const;
  language::Core::ASTContext &getASTContext() const { return astContext; }

  bool isRecordLayoutComplete(const language::Core::Type *ty) const;
  bool noRecordsBeingLaidOut() const { return recordsBeingLaidOut.empty(); }
  bool isRecordBeingLaidOut(const language::Core::Type *ty) const {
    return recordsBeingLaidOut.count(ty);
  }

  const ABIInfo &getABIInfo() const { return theABIInfo; }

  /// Convert a Clang type into a mlir::Type.
  mlir::Type convertType(language::Core::QualType type);

  mlir::Type convertRecordDeclType(const language::Core::RecordDecl *recordDecl);

  std::unique_ptr<CIRGenRecordLayout>
  computeRecordLayout(const language::Core::RecordDecl *rd, cir::RecordType *ty);

  std::string getRecordTypeName(const language::Core::RecordDecl *,
                                toolchain::StringRef suffix);

  const CIRGenRecordLayout &getCIRGenRecordLayout(const language::Core::RecordDecl *rd);

  /// Convert type T into an mlir::Type. This differs from convertType in that
  /// it is used to convert to the memory representation for a type. For
  /// example, the scalar representation for bool is i1, but the memory
  /// representation is usually i8 or i32, depending on the target.
  // TODO: convert this comment to account for MLIR's equivalence
  mlir::Type convertTypeForMem(language::Core::QualType, bool forBitField = false);

  /// Get the CIR function type for \arg Info.
  cir::FuncType getFunctionType(const CIRGenFunctionInfo &info);

  // The arrangement methods are split into three families:
  //   - those meant to drive the signature and prologue/epilogue
  //     of a function declaration or definition,
  //   - those meant for the computation of the CIR type for an abstract
  //     appearance of a function, and
  //   - those meant for performing the CIR-generation of a call.
  // They differ mainly in how they deal with optional (i.e. variadic)
  // arguments, as well as unprototyped functions.
  //
  // Key points:
  // - The CIRGenFunctionInfo for emitting a specific call site must include
  //   entries for the optional arguments.
  // - The function type used at the call site must reflect the formal
  // signature
  //   of the declaration being called, or else the call will go away.
  // - For the most part, unprototyped functions are called by casting to a
  //   formal signature inferred from the specific argument types used at the
  //   call-site. However, some targets (e.g. x86-64) screw with this for
  //   compatability reasons.

  const CIRGenFunctionInfo &arrangeGlobalDeclaration(GlobalDecl gd);

  /// UpdateCompletedType - when we find the full definition for a TagDecl,
  /// replace the 'opaque' type we previously made for it if applicable.
  void updateCompletedType(const language::Core::TagDecl *td);

  /// Free functions are functions that are compatible with an ordinary C
  /// function pointer type.
  const CIRGenFunctionInfo &
  arrangeFunctionDeclaration(const language::Core::FunctionDecl *fd);

  /// Return whether a type can be zero-initialized (in the C++ sense) with an
  /// LLVM zeroinitializer.
  bool isZeroInitializable(language::Core::QualType ty);
  bool isZeroInitializable(const RecordDecl *rd);

  const CIRGenFunctionInfo &arrangeCXXConstructorCall(
      const CallArgList &args, const language::Core::CXXConstructorDecl *d,
      language::Core::CXXCtorType ctorKind, bool passProtoArgs = true);

  const CIRGenFunctionInfo &
  arrangeCXXMethodCall(const CallArgList &args,
                       const language::Core::FunctionProtoType *type,
                       RequiredArgs required, unsigned numPrefixArgs);

  /// C++ methods have some special rules and also have implicit parameters.
  const CIRGenFunctionInfo &
  arrangeCXXMethodDeclaration(const language::Core::CXXMethodDecl *md);
  const CIRGenFunctionInfo &arrangeCXXStructorDeclaration(language::Core::GlobalDecl gd);

  const CIRGenFunctionInfo &
  arrangeCXXMethodType(const language::Core::CXXRecordDecl *rd,
                       const language::Core::FunctionProtoType *ftp,
                       const language::Core::CXXMethodDecl *md);

  const CIRGenFunctionInfo &arrangeFreeFunctionCall(const CallArgList &args,
                                                    const FunctionType *fnType);

  const CIRGenFunctionInfo &
  arrangeCIRFunctionInfo(CanQualType returnType,
                         toolchain::ArrayRef<CanQualType> argTypes,
                         RequiredArgs required);

  const CIRGenFunctionInfo &
  arrangeFreeFunctionType(CanQual<FunctionProtoType> fpt);
  const CIRGenFunctionInfo &
  arrangeFreeFunctionType(CanQual<FunctionNoProtoType> fnpt);
};

} // namespace language::Core::CIRGen

#endif
