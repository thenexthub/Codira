//===--- CGVTables.h - Emit LLVM Code for C++ vtables -----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGVTABLES_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGVTABLES_H

#include "language/Core/AST/BaseSubobject.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/GlobalDecl.h"
#include "language/Core/AST/VTableBuilder.h"
#include "language/Core/Basic/ABI.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/IR/GlobalVariable.h"

namespace language::Core {
  class CXXRecordDecl;

namespace CodeGen {
  class CodeGenModule;
  class ConstantArrayBuilder;
  class ConstantStructBuilder;

class CodeGenVTables {
  CodeGenModule &CGM;

  VTableContextBase *VTContext;

  /// VTableAddressPointsMapTy - Address points for a single vtable.
  typedef VTableLayout::AddressPointsMapTy VTableAddressPointsMapTy;

  typedef std::pair<const CXXRecordDecl *, BaseSubobject> BaseSubobjectPairTy;
  typedef toolchain::DenseMap<BaseSubobjectPairTy, uint64_t> SubVTTIndicesMapTy;

  /// SubVTTIndices - Contains indices into the various sub-VTTs.
  SubVTTIndicesMapTy SubVTTIndices;

  typedef toolchain::DenseMap<BaseSubobjectPairTy, uint64_t>
    SecondaryVirtualPointerIndicesMapTy;

  /// SecondaryVirtualPointerIndices - Contains the secondary virtual pointer
  /// indices.
  SecondaryVirtualPointerIndicesMapTy SecondaryVirtualPointerIndices;

  /// Cache for the pure virtual member call function.
  toolchain::Constant *PureVirtualFn = nullptr;

  /// Cache for the deleted virtual member call function.
  toolchain::Constant *DeletedVirtualFn = nullptr;

  /// Get the address of a thunk and emit it if necessary.
  toolchain::Constant *maybeEmitThunk(GlobalDecl GD,
                                 const ThunkInfo &ThunkAdjustments,
                                 bool ForVTable);

  void addVTableComponent(ConstantArrayBuilder &builder,
                          const VTableLayout &layout, unsigned componentIndex,
                          toolchain::Constant *rtti, unsigned &nextVTableThunkIndex,
                          unsigned vtableAddressPoint,
                          bool vtableHasLocalLinkage);

  /// Add a 32-bit offset to a component relative to the vtable when using the
  /// relative vtables ABI. The array builder points to the start of the vtable.
  void addRelativeComponent(ConstantArrayBuilder &builder,
                            toolchain::Constant *component,
                            unsigned vtableAddressPoint,
                            bool vtableHasLocalLinkage,
                            bool isCompleteDtor) const;

public:
  /// Add vtable components for the given vtable layout to the given
  /// global initializer.
  void createVTableInitializer(ConstantStructBuilder &builder,
                               const VTableLayout &layout, toolchain::Constant *rtti,
                               bool vtableHasLocalLinkage);

  CodeGenVTables(CodeGenModule &CGM);

  ItaniumVTableContext &getItaniumVTableContext() {
    return *cast<ItaniumVTableContext>(VTContext);
  }

  const ItaniumVTableContext &getItaniumVTableContext() const {
    return *cast<ItaniumVTableContext>(VTContext);
  }

  MicrosoftVTableContext &getMicrosoftVTableContext() {
    return *cast<MicrosoftVTableContext>(VTContext);
  }

  /// getSubVTTIndex - Return the index of the sub-VTT for the base class of the
  /// given record decl.
  uint64_t getSubVTTIndex(const CXXRecordDecl *RD, BaseSubobject Base);

  /// getSecondaryVirtualPointerIndex - Return the index in the VTT where the
  /// virtual pointer for the given subobject is located.
  uint64_t getSecondaryVirtualPointerIndex(const CXXRecordDecl *RD,
                                           BaseSubobject Base);

  /// GenerateConstructionVTable - Generate a construction vtable for the given
  /// base subobject.
  toolchain::GlobalVariable *
  GenerateConstructionVTable(const CXXRecordDecl *RD, const BaseSubobject &Base,
                             bool BaseIsVirtual,
                             toolchain::GlobalVariable::LinkageTypes Linkage,
                             VTableAddressPointsMapTy& AddressPoints);


  /// GetAddrOfVTT - Get the address of the VTT for the given record decl.
  toolchain::GlobalVariable *GetAddrOfVTT(const CXXRecordDecl *RD);

  /// EmitVTTDefinition - Emit the definition of the given vtable.
  void EmitVTTDefinition(toolchain::GlobalVariable *VTT,
                         toolchain::GlobalVariable::LinkageTypes Linkage,
                         const CXXRecordDecl *RD);

  /// EmitThunks - Emit the associated thunks for the given global decl.
  void EmitThunks(GlobalDecl GD);

  /// GenerateClassData - Generate all the class data required to be
  /// generated upon definition of a KeyFunction.  This includes the
  /// vtable, the RTTI data structure (if RTTI is enabled) and the VTT
  /// (if the class has virtual bases).
  void GenerateClassData(const CXXRecordDecl *RD);

  bool isVTableExternal(const CXXRecordDecl *RD);

  /// Returns the type of a vtable with the given layout. Normally a struct of
  /// arrays of pointers, with one struct element for each vtable in the vtable
  /// group.
  toolchain::Type *getVTableType(const VTableLayout &layout);

  /// Generate a public facing alias for the vtable and make the vtable either
  /// hidden or private. The alias will have the original linkage and visibility
  /// of the vtable. This is used for cases under the relative vtables ABI
  /// when a vtable may not be dso_local.
  void GenerateRelativeVTableAlias(toolchain::GlobalVariable *VTable,
                                   toolchain::StringRef AliasNameRef);

  /// Specify a global should not be instrumented with hwasan.
  void RemoveHwasanMetadata(toolchain::GlobalValue *GV) const;

  /// Return the type used as components for a vtable.
  toolchain::Type *getVTableComponentType() const;

  /// Return true if the relative vtable layout is used.
  bool useRelativeLayout() const;
};

} // end namespace CodeGen
} // end namespace language::Core
#endif
