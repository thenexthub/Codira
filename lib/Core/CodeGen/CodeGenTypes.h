//===--- CodeGenTypes.h - Type translation for LLVM CodeGen -----*- C++ -*-===//
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
// This is the code that handles AST -> LLVM type lowering.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CODEGENTYPES_H
#define LANGUAGE_CORE_LIB_CODEGEN_CODEGENTYPES_H

#include "CGCall.h"
#include "language/Core/Basic/ABI.h"
#include "language/Core/CodeGen/CGFunctionInfo.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/IR/Module.h"

namespace toolchain {
class FunctionType;
class DataLayout;
class Type;
class LLVMContext;
class StructType;
}

namespace language::Core {
class ASTContext;
template <typename> class CanQual;
class CXXConstructorDecl;
class CXXMethodDecl;
class CodeGenOptions;
class FunctionProtoType;
class QualType;
class RecordDecl;
class TagDecl;
class TargetInfo;
class Type;
typedef CanQual<Type> CanQualType;
class GlobalDecl;

namespace CodeGen {
class ABIInfo;
class CGCXXABI;
class CGRecordLayout;
class CodeGenModule;
class RequiredArgs;

/// This class organizes the cross-module state that is used while lowering
/// AST types to LLVM types.
class CodeGenTypes {
  CodeGenModule &CGM;
  // Some of this stuff should probably be left on the CGM.
  ASTContext &Context;
  toolchain::Module &TheModule;
  const TargetInfo &Target;

  /// The opaque type map for Objective-C interfaces. All direct
  /// manipulation is done by the runtime interfaces, which are
  /// responsible for coercing to the appropriate type; these opaque
  /// types are never refined.
  toolchain::DenseMap<const ObjCInterfaceType*, toolchain::Type *> InterfaceTypes;

  /// Maps clang struct type with corresponding record layout info.
  toolchain::DenseMap<const Type*, std::unique_ptr<CGRecordLayout>> CGRecordLayouts;

  /// Contains the LLVM IR type for any converted RecordDecl.
  toolchain::DenseMap<const Type*, toolchain::StructType *> RecordDeclTypes;

  /// Hold memoized CGFunctionInfo results.
  toolchain::FoldingSet<CGFunctionInfo> FunctionInfos{FunctionInfosLog2InitSize};

  toolchain::SmallPtrSet<const CGFunctionInfo*, 4> FunctionsBeingProcessed;

  /// True if we didn't layout a function due to a being inside
  /// a recursive struct conversion, set this to true.
  bool SkippedLayout;

  /// True if any instance of long double types are used.
  bool LongDoubleReferenced;

  /// This map keeps cache of toolchain::Types and maps language::Core::Type to
  /// corresponding toolchain::Type.
  toolchain::DenseMap<const Type *, toolchain::Type *> TypeCache;

  toolchain::DenseMap<const Type *, toolchain::Type *> RecordsWithOpaqueMemberPointers;

  static constexpr unsigned FunctionInfosLog2InitSize = 9;
  /// Helper for ConvertType.
  toolchain::Type *ConvertFunctionTypeInternal(QualType FT);

public:
  CodeGenTypes(CodeGenModule &cgm);
  ~CodeGenTypes();

  const toolchain::DataLayout &getDataLayout() const {
    return TheModule.getDataLayout();
  }
  CodeGenModule &getCGM() const { return CGM; }
  ASTContext &getContext() const { return Context; }
  const TargetInfo &getTarget() const { return Target; }
  CGCXXABI &getCXXABI() const;
  toolchain::LLVMContext &getLLVMContext() { return TheModule.getContext(); }
  const CodeGenOptions &getCodeGenOpts() const;

  /// Convert clang calling convention to LLVM callilng convention.
  unsigned ClangCallConvToLLVMCallConv(CallingConv CC);

  /// Derives the 'this' type for codegen purposes, i.e. ignoring method CVR
  /// qualification.
  CanQualType DeriveThisType(const CXXRecordDecl *RD, const CXXMethodDecl *MD);

  /// ConvertType - Convert type T into a toolchain::Type.
  toolchain::Type *ConvertType(QualType T);

  /// ConvertTypeForMem - Convert type T into a toolchain::Type.  This differs from
  /// ConvertType in that it is used to convert to the memory representation for
  /// a type.  For example, the scalar representation for _Bool is i1, but the
  /// memory representation is usually i8 or i32, depending on the target.
  toolchain::Type *ConvertTypeForMem(QualType T);

  /// Check whether the given type needs to be laid out in memory
  /// using an opaque byte-array type because its load/store type
  /// does not have the correct alloc size in the LLVM data layout.
  /// If this is false, the load/store type (convertTypeForLoadStore)
  /// and memory representation type (ConvertTypeForMem) will
  /// be the same type.
  bool typeRequiresSplitIntoByteArray(QualType ASTTy,
                                      toolchain::Type *LLVMTy = nullptr);

  /// Given that T is a scalar type, return the IR type that should
  /// be used for load and store operations.  For example, this might
  /// be i8 for _Bool or i96 for _BitInt(65).  The store size of the
  /// load/store type (as reported by LLVM's data layout) is always
  /// the same as the alloc size of the memory representation type
  /// returned by ConvertTypeForMem.
  ///
  /// As an optimization, if you already know the scalar value type
  /// for T (as would be returned by ConvertType), you can pass
  /// it as the second argument so that it does not need to be
  /// recomputed in common cases where the value type and
  /// load/store type are the same.
  toolchain::Type *convertTypeForLoadStore(QualType T, toolchain::Type *LLVMTy = nullptr);

  /// GetFunctionType - Get the LLVM function type for \arg Info.
  toolchain::FunctionType *GetFunctionType(const CGFunctionInfo &Info);

  toolchain::FunctionType *GetFunctionType(GlobalDecl GD);

  /// isFuncTypeConvertible - Utility to check whether a function type can
  /// be converted to an LLVM type (i.e. doesn't depend on an incomplete tag
  /// type).
  bool isFuncTypeConvertible(const FunctionType *FT);
  bool isFuncParamTypeConvertible(QualType Ty);

  /// Determine if a C++ inheriting constructor should have parameters matching
  /// those of its inherited constructor.
  bool inheritingCtorHasParams(const InheritedConstructor &Inherited,
                               CXXCtorType Type);

  /// GetFunctionTypeForVTable - Get the LLVM function type for use in a vtable,
  /// given a CXXMethodDecl. If the method to has an incomplete return type,
  /// and/or incomplete argument types, this will return the opaque type.
  toolchain::Type *GetFunctionTypeForVTable(GlobalDecl GD);

  const CGRecordLayout &getCGRecordLayout(const RecordDecl*);

  /// UpdateCompletedType - When we find the full definition for a TagDecl,
  /// replace the 'opaque' type we previously made for it if applicable.
  void UpdateCompletedType(const TagDecl *TD);

  /// Remove stale types from the type cache when an inheritance model
  /// gets assigned to a class.
  void RefreshTypeCacheForClass(const CXXRecordDecl *RD);

  // The arrangement methods are split into three families:
  //   - those meant to drive the signature and prologue/epilogue
  //     of a function declaration or definition,
  //   - those meant for the computation of the LLVM type for an abstract
  //     appearance of a function, and
  //   - those meant for performing the IR-generation of a call.
  // They differ mainly in how they deal with optional (i.e. variadic)
  // arguments, as well as unprototyped functions.
  //
  // Key points:
  // - The CGFunctionInfo for emitting a specific call site must include
  //   entries for the optional arguments.
  // - The function type used at the call site must reflect the formal
  //   signature of the declaration being called, or else the call will
  //   go awry.
  // - For the most part, unprototyped functions are called by casting to
  //   a formal signature inferred from the specific argument types used
  //   at the call-site.  However, some targets (e.g. x86-64) screw with
  //   this for compatibility reasons.

  const CGFunctionInfo &arrangeGlobalDeclaration(GlobalDecl GD);

  /// Given a function info for a declaration, return the function info
  /// for a call with the given arguments.
  ///
  /// Often this will be able to simply return the declaration info.
  const CGFunctionInfo &arrangeCall(const CGFunctionInfo &declFI,
                                    const CallArgList &args);

  /// Free functions are functions that are compatible with an ordinary
  /// C function pointer type.
  const CGFunctionInfo &arrangeFunctionDeclaration(const GlobalDecl GD);
  const CGFunctionInfo &arrangeFreeFunctionCall(const CallArgList &Args,
                                                const FunctionType *Ty,
                                                bool ChainCall);
  const CGFunctionInfo &arrangeFreeFunctionType(CanQual<FunctionProtoType> Ty);
  const CGFunctionInfo &arrangeFreeFunctionType(CanQual<FunctionNoProtoType> Ty);

  /// A nullary function is a freestanding function of type 'void ()'.
  /// This method works for both calls and declarations.
  const CGFunctionInfo &arrangeNullaryFunction();

  /// A builtin function is a freestanding function using the default
  /// C conventions.
  const CGFunctionInfo &
  arrangeBuiltinFunctionDeclaration(QualType resultType,
                                    const FunctionArgList &args);
  const CGFunctionInfo &
  arrangeBuiltinFunctionDeclaration(CanQualType resultType,
                                    ArrayRef<CanQualType> argTypes);
  const CGFunctionInfo &arrangeBuiltinFunctionCall(QualType resultType,
                                                   const CallArgList &args);

  /// A SYCL kernel caller function is an offload device entry point function
  /// with a target device dependent calling convention such as amdgpu_kernel,
  /// ptx_kernel, or spir_kernel.
  const CGFunctionInfo &
  arrangeSYCLKernelCallerDeclaration(QualType resultType,
                                     const FunctionArgList &args);

  /// Objective-C methods are C functions with some implicit parameters.
  const CGFunctionInfo &arrangeObjCMethodDeclaration(const ObjCMethodDecl *MD);
  const CGFunctionInfo &arrangeObjCMessageSendSignature(const ObjCMethodDecl *MD,
                                                        QualType receiverType);
  const CGFunctionInfo &arrangeUnprototypedObjCMessageSend(
                                                     QualType returnType,
                                                     const CallArgList &args);

  /// Block invocation functions are C functions with an implicit parameter.
  const CGFunctionInfo &arrangeBlockFunctionDeclaration(
                                                 const FunctionProtoType *type,
                                                 const FunctionArgList &args);
  const CGFunctionInfo &arrangeBlockFunctionCall(const CallArgList &args,
                                                 const FunctionType *type);

  /// C++ methods have some special rules and also have implicit parameters.
  const CGFunctionInfo &arrangeCXXMethodDeclaration(const CXXMethodDecl *MD);
  const CGFunctionInfo &arrangeCXXStructorDeclaration(GlobalDecl GD);
  const CGFunctionInfo &arrangeCXXConstructorCall(const CallArgList &Args,
                                                  const CXXConstructorDecl *D,
                                                  CXXCtorType CtorKind,
                                                  unsigned ExtraPrefixArgs,
                                                  unsigned ExtraSuffixArgs,
                                                  bool PassProtoArgs = true);

  const CGFunctionInfo &arrangeCXXMethodCall(const CallArgList &args,
                                             const FunctionProtoType *type,
                                             RequiredArgs required,
                                             unsigned numPrefixArgs);
  const CGFunctionInfo &
  arrangeUnprototypedMustTailThunk(const CXXMethodDecl *MD);
  const CGFunctionInfo &arrangeMSCtorClosure(const CXXConstructorDecl *CD,
                                                 CXXCtorType CT);
  const CGFunctionInfo &arrangeCXXMethodType(const CXXRecordDecl *RD,
                                             const FunctionProtoType *FTP,
                                             const CXXMethodDecl *MD);

  /// "Arrange" the LLVM information for a call or type with the given
  /// signature.  This is largely an internal method; other clients
  /// should use one of the above routines, which ultimately defer to
  /// this.
  ///
  /// \param argTypes - must all actually be canonical as params
  const CGFunctionInfo &arrangeLLVMFunctionInfo(
      CanQualType returnType, FnInfoOpts opts, ArrayRef<CanQualType> argTypes,
      FunctionType::ExtInfo info,
      ArrayRef<FunctionProtoType::ExtParameterInfo> paramInfos,
      RequiredArgs args);

  /// Compute a new LLVM record layout object for the given record.
  std::unique_ptr<CGRecordLayout> ComputeRecordLayout(const RecordDecl *D,
                                                      toolchain::StructType *Ty);

  /// addRecordTypeName - Compute a name from the given record decl with an
  /// optional suffix and name the given LLVM type using it.
  void addRecordTypeName(const RecordDecl *RD, toolchain::StructType *Ty,
                         StringRef suffix);


public:  // These are internal details of CGT that shouldn't be used externally.
  /// ConvertRecordDeclType - Lay out a tagged decl type like struct or union.
  toolchain::StructType *ConvertRecordDeclType(const RecordDecl *TD);

  /// getExpandedTypes - Expand the type \arg Ty into the LLVM
  /// argument types it would be passed as. See ABIArgInfo::Expand.
  void getExpandedTypes(QualType Ty,
                        SmallVectorImpl<toolchain::Type *>::iterator &TI);

  /// IsZeroInitializable - Return whether a type can be
  /// zero-initialized (in the C++ sense) with an LLVM zeroinitializer.
  bool isZeroInitializable(QualType T);

  /// Check if the pointer type can be zero-initialized (in the C++ sense)
  /// with an LLVM zeroinitializer.
  bool isPointerZeroInitializable(QualType T);

  /// IsZeroInitializable - Return whether a record type can be
  /// zero-initialized (in the C++ sense) with an LLVM zeroinitializer.
  bool isZeroInitializable(const RecordDecl *RD);

  bool isLongDoubleReferenced() const { return LongDoubleReferenced; }
  bool isRecordLayoutComplete(const Type *Ty) const;
  unsigned getTargetAddressSpace(QualType T) const;
};

}  // end namespace CodeGen
}  // end namespace language::Core

#endif
