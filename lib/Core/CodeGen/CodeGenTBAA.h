//===--- CodeGenTBAA.h - TBAA information for LLVM CodeGen ------*- C++ -*-===//
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
// This is the code that manages TBAA information and defines the TBAA policy
// for the optimizer to use.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CODEGENTBAA_H
#define LANGUAGE_CORE_LIB_CODEGEN_CODEGENTBAA_H

#include "clang/AST/Type.h"
#include "clang/Basic/LLVM.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/IR/MDBuilder.h"
#include "toolchain/IR/Metadata.h"

namespace language::Core {
  class ASTContext;
  class CodeGenOptions;
  class LangOptions;
  class MangleContext;
  class QualType;
  class Type;

namespace CodeGen {
class CodeGenTypes;

// TBAAAccessKind - A kind of TBAA memory access descriptor.
enum class TBAAAccessKind : unsigned {
  Ordinary,
  MayAlias,
  Incomplete,
};

// TBAAAccessInfo - Describes a memory access in terms of TBAA.
struct TBAAAccessInfo {
  TBAAAccessInfo(TBAAAccessKind Kind, toolchain::MDNode *BaseType,
                 toolchain::MDNode *AccessType, uint64_t Offset, uint64_t Size)
    : Kind(Kind), BaseType(BaseType), AccessType(AccessType),
      Offset(Offset), Size(Size)
  {}

  TBAAAccessInfo(toolchain::MDNode *BaseType, toolchain::MDNode *AccessType,
                 uint64_t Offset, uint64_t Size)
    : TBAAAccessInfo(TBAAAccessKind::Ordinary, BaseType, AccessType,
                     Offset, Size)
  {}

  explicit TBAAAccessInfo(toolchain::MDNode *AccessType, uint64_t Size)
    : TBAAAccessInfo(/* BaseType= */ nullptr, AccessType, /* Offset= */ 0, Size)
  {}

  TBAAAccessInfo()
    : TBAAAccessInfo(/* AccessType= */ nullptr, /* Size= */ 0)
  {}

  static TBAAAccessInfo getMayAliasInfo() {
    return TBAAAccessInfo(TBAAAccessKind::MayAlias,
                          /* BaseType= */ nullptr, /* AccessType= */ nullptr,
                          /* Offset= */ 0, /* Size= */ 0);
  }

  bool isMayAlias() const { return Kind == TBAAAccessKind::MayAlias; }

  static TBAAAccessInfo getIncompleteInfo() {
    return TBAAAccessInfo(TBAAAccessKind::Incomplete,
                          /* BaseType= */ nullptr, /* AccessType= */ nullptr,
                          /* Offset= */ 0, /* Size= */ 0);
  }

  bool isIncomplete() const { return Kind == TBAAAccessKind::Incomplete; }

  bool operator==(const TBAAAccessInfo &Other) const {
    return Kind == Other.Kind &&
           BaseType == Other.BaseType &&
           AccessType == Other.AccessType &&
           Offset == Other.Offset &&
           Size == Other.Size;
  }

  bool operator!=(const TBAAAccessInfo &Other) const {
    return !(*this == Other);
  }

  explicit operator bool() const {
    return *this != TBAAAccessInfo();
  }

  /// Kind - The kind of the access descriptor.
  TBAAAccessKind Kind;

  /// BaseType - The base/leading access type. May be null if this access
  /// descriptor represents an access that is not considered to be an access
  /// to an aggregate or union member.
  toolchain::MDNode *BaseType;

  /// AccessType - The final access type. May be null if there is no TBAA
  /// information available about this access.
  toolchain::MDNode *AccessType;

  /// Offset - The byte offset of the final access within the base one. Must be
  /// zero if the base access type is not specified.
  uint64_t Offset;

  /// Size - The size of access, in bytes.
  uint64_t Size;
};

/// CodeGenTBAA - This class organizes the cross-module state that is used
/// while lowering AST types to LLVM types.
class CodeGenTBAA {
  ASTContext &Context;
  CodeGenTypes &CGTypes;
  toolchain::Module &Module;
  const CodeGenOptions &CodeGenOpts;
  const LangOptions &Features;
  std::unique_ptr<MangleContext> MangleCtx;

  // MDHelper - Helper for creating metadata.
  toolchain::MDBuilder MDHelper;

  /// MetadataCache - This maps clang::Types to scalar toolchain::MDNodes describing
  /// them.
  toolchain::DenseMap<const Type *, toolchain::MDNode *> MetadataCache;
  /// This maps clang::Types to a base access type in the type DAG.
  toolchain::DenseMap<const Type *, toolchain::MDNode *> BaseTypeMetadataCache;
  /// This maps TBAA access descriptors to tag nodes.
  toolchain::DenseMap<TBAAAccessInfo, toolchain::MDNode *> AccessTagMetadataCache;

  /// StructMetadataCache - This maps clang::Types to toolchain::MDNodes describing
  /// them for struct assignments.
  toolchain::DenseMap<const Type *, toolchain::MDNode *> StructMetadataCache;

  toolchain::MDNode *Root;
  toolchain::MDNode *Char;
  toolchain::SmallVector<toolchain::MDNode *, 4> AnyPtrs;

  /// getRoot - This is the mdnode for the root of the metadata type graph
  /// for this translation unit.
  toolchain::MDNode *getRoot();

  /// getChar - This is the mdnode for "char", which is special, and any types
  /// considered to be equivalent to it.
  toolchain::MDNode *getChar();

  /// getAnyPtr - This is the mdnode for any pointer type of (at least) the
  /// given pointer depth.
  toolchain::MDNode *getAnyPtr(unsigned PtrDepth = 1);

  /// CollectFields - Collect information about the fields of a type for
  /// !tbaa.struct metadata formation. Return false for an unsupported type.
  bool CollectFields(uint64_t BaseOffset,
                     QualType Ty,
                     SmallVectorImpl<toolchain::MDBuilder::TBAAStructField> &Fields,
                     bool MayAlias);

  /// createScalarTypeNode - A wrapper function to create a metadata node
  /// describing a scalar type.
  toolchain::MDNode *createScalarTypeNode(StringRef Name, toolchain::MDNode *Parent,
                                     uint64_t Size);

  /// getTypeInfoHelper - An internal helper function to generate metadata used
  /// to describe accesses to objects of the given type.
  toolchain::MDNode *getTypeInfoHelper(const Type *Ty);

  /// getBaseTypeInfoHelper - An internal helper function to generate metadata
  /// used to describe accesses to objects of the given base type.
  toolchain::MDNode *getBaseTypeInfoHelper(const Type *Ty);

  /// getValidBaseTypeInfo - Return metadata that describes the given base
  /// access type. The type must be suitable.
  toolchain::MDNode *getValidBaseTypeInfo(QualType QTy);

public:
  CodeGenTBAA(ASTContext &Ctx, CodeGenTypes &CGTypes, toolchain::Module &M,
              const CodeGenOptions &CGO, const LangOptions &Features);
  ~CodeGenTBAA();

  /// getTypeInfo - Get metadata used to describe accesses to objects of the
  /// given type.
  toolchain::MDNode *getTypeInfo(QualType QTy);

  /// getAccessInfo - Get TBAA information that describes an access to
  /// an object of the given type.
  TBAAAccessInfo getAccessInfo(QualType AccessType);

  /// getVTablePtrAccessInfo - Get the TBAA information that describes an
  /// access to a virtual table pointer.
  TBAAAccessInfo getVTablePtrAccessInfo(toolchain::Type *VTablePtrType);

  /// getTBAAStructInfo - Get the TBAAStruct MDNode to be used for a memcpy of
  /// the given type.
  toolchain::MDNode *getTBAAStructInfo(QualType QTy);

  /// getBaseTypeInfo - Get metadata that describes the given base access
  /// type. Return null if the type is not suitable for use in TBAA access
  /// tags.
  toolchain::MDNode *getBaseTypeInfo(QualType QTy);

  /// getAccessTagInfo - Get TBAA tag for a given memory access.
  toolchain::MDNode *getAccessTagInfo(TBAAAccessInfo Info);

  /// mergeTBAAInfoForCast - Get merged TBAA information for the purpose of
  /// type casts.
  TBAAAccessInfo mergeTBAAInfoForCast(TBAAAccessInfo SourceInfo,
                                      TBAAAccessInfo TargetInfo);

  /// mergeTBAAInfoForConditionalOperator - Get merged TBAA information for the
  /// purpose of conditional operator.
  TBAAAccessInfo mergeTBAAInfoForConditionalOperator(TBAAAccessInfo InfoA,
                                                     TBAAAccessInfo InfoB);

  /// mergeTBAAInfoForMemoryTransfer - Get merged TBAA information for the
  /// purpose of memory transfer calls.
  TBAAAccessInfo mergeTBAAInfoForMemoryTransfer(TBAAAccessInfo DestInfo,
                                                TBAAAccessInfo SrcInfo);
};

}  // end namespace CodeGen
}  // end namespace language::Core

namespace toolchain {

template<> struct DenseMapInfo<clang::CodeGen::TBAAAccessInfo> {
  static clang::CodeGen::TBAAAccessInfo getEmptyKey() {
    unsigned UnsignedKey = DenseMapInfo<unsigned>::getEmptyKey();
    return clang::CodeGen::TBAAAccessInfo(
      static_cast<clang::CodeGen::TBAAAccessKind>(UnsignedKey),
      DenseMapInfo<MDNode *>::getEmptyKey(),
      DenseMapInfo<MDNode *>::getEmptyKey(),
      DenseMapInfo<uint64_t>::getEmptyKey(),
      DenseMapInfo<uint64_t>::getEmptyKey());
  }

  static clang::CodeGen::TBAAAccessInfo getTombstoneKey() {
    unsigned UnsignedKey = DenseMapInfo<unsigned>::getTombstoneKey();
    return clang::CodeGen::TBAAAccessInfo(
      static_cast<clang::CodeGen::TBAAAccessKind>(UnsignedKey),
      DenseMapInfo<MDNode *>::getTombstoneKey(),
      DenseMapInfo<MDNode *>::getTombstoneKey(),
      DenseMapInfo<uint64_t>::getTombstoneKey(),
      DenseMapInfo<uint64_t>::getTombstoneKey());
  }

  static unsigned getHashValue(const clang::CodeGen::TBAAAccessInfo &Val) {
    auto KindValue = static_cast<unsigned>(Val.Kind);
    return DenseMapInfo<unsigned>::getHashValue(KindValue) ^
           DenseMapInfo<MDNode *>::getHashValue(Val.BaseType) ^
           DenseMapInfo<MDNode *>::getHashValue(Val.AccessType) ^
           DenseMapInfo<uint64_t>::getHashValue(Val.Offset) ^
           DenseMapInfo<uint64_t>::getHashValue(Val.Size);
  }

  static bool isEqual(const clang::CodeGen::TBAAAccessInfo &LHS,
                      const clang::CodeGen::TBAAAccessInfo &RHS) {
    return LHS == RHS;
  }
};

}  // end namespace toolchain

#endif
