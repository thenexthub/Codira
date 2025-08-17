//===- VTTBuilder.h - C++ VTT layout builder --------------------*- C++ -*-===//
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
// This contains code dealing with generation of the layout of virtual table
// tables (VTT).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_VTTBUILDER_H
#define LANGUAGE_CORE_AST_VTTBUILDER_H

#include "language/Core/AST/BaseSubobject.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"
#include <cstdint>

namespace language::Core {

class ASTContext;
class ASTRecordLayout;
class CXXRecordDecl;

class VTTVTable {
  toolchain::PointerIntPair<const CXXRecordDecl *, 1, bool> BaseAndIsVirtual;
  CharUnits BaseOffset;

public:
  VTTVTable() = default;
  VTTVTable(const CXXRecordDecl *Base, CharUnits BaseOffset, bool BaseIsVirtual)
      : BaseAndIsVirtual(Base, BaseIsVirtual), BaseOffset(BaseOffset) {}
  VTTVTable(BaseSubobject Base, bool BaseIsVirtual)
      : BaseAndIsVirtual(Base.getBase(), BaseIsVirtual),
        BaseOffset(Base.getBaseOffset()) {}

  const CXXRecordDecl *getBase() const {
    return BaseAndIsVirtual.getPointer();
  }

  CharUnits getBaseOffset() const {
    return BaseOffset;
  }

  bool isVirtual() const {
    return BaseAndIsVirtual.getInt();
  }

  BaseSubobject getBaseSubobject() const {
    return BaseSubobject(getBase(), getBaseOffset());
  }
};

struct VTTComponent {
  uint64_t VTableIndex;
  BaseSubobject VTableBase;

  VTTComponent() = default;
  VTTComponent(uint64_t VTableIndex, BaseSubobject VTableBase)
     : VTableIndex(VTableIndex), VTableBase(VTableBase) {}
};

/// Class for building VTT layout information.
class VTTBuilder {
  ASTContext &Ctx;

  /// The most derived class for which we're building this vtable.
  const CXXRecordDecl *MostDerivedClass;

  using VTTVTablesVectorTy = SmallVector<VTTVTable, 64>;

  /// The VTT vtables.
  VTTVTablesVectorTy VTTVTables;

  using VTTComponentsVectorTy = SmallVector<VTTComponent, 64>;

  /// The VTT components.
  VTTComponentsVectorTy VTTComponents;

  /// The AST record layout of the most derived class.
  const ASTRecordLayout &MostDerivedClassLayout;

  using VisitedVirtualBasesSetTy = toolchain::SmallPtrSet<const CXXRecordDecl *, 4>;

  using AddressPointsMapTy = toolchain::DenseMap<BaseSubobject, uint64_t>;

  /// The sub-VTT indices for the bases of the most derived class.
  toolchain::DenseMap<BaseSubobject, uint64_t> SubVTTIndices;

  /// The secondary virtual pointer indices of all subobjects of
  /// the most derived class.
  toolchain::DenseMap<BaseSubobject, uint64_t> SecondaryVirtualPointerIndices;

  /// Whether the VTT builder should generate LLVM IR for the VTT.
  bool GenerateDefinition;

  /// Add a vtable pointer to the VTT currently being built.
  void AddVTablePointer(BaseSubobject Base, uint64_t VTableIndex,
                        const CXXRecordDecl *VTableClass);

  /// Lay out the secondary VTTs of the given base subobject.
  void LayoutSecondaryVTTs(BaseSubobject Base);

  /// Lay out the secondary virtual pointers for the given base
  /// subobject.
  ///
  /// \param BaseIsMorallyVirtual whether the base subobject is a virtual base
  /// or a direct or indirect base of a virtual base.
  void LayoutSecondaryVirtualPointers(BaseSubobject Base,
                                      bool BaseIsMorallyVirtual,
                                      uint64_t VTableIndex,
                                      const CXXRecordDecl *VTableClass,
                                      VisitedVirtualBasesSetTy &VBases);

  /// Lay out the secondary virtual pointers for the given base
  /// subobject.
  void LayoutSecondaryVirtualPointers(BaseSubobject Base,
                                      uint64_t VTableIndex);

  /// Lay out the VTTs for the virtual base classes of the given
  /// record declaration.
  void LayoutVirtualVTTs(const CXXRecordDecl *RD,
                         VisitedVirtualBasesSetTy &VBases);

  /// Lay out the VTT for the given subobject, including any
  /// secondary VTTs, secondary virtual pointers and virtual VTTs.
  void LayoutVTT(BaseSubobject Base, bool BaseIsVirtual);

public:
  VTTBuilder(ASTContext &Ctx, const CXXRecordDecl *MostDerivedClass,
             bool GenerateDefinition);

  // Returns a reference to the VTT components.
  const VTTComponentsVectorTy &getVTTComponents() const {
    return VTTComponents;
  }

  // Returns a reference to the VTT vtables.
  const VTTVTablesVectorTy &getVTTVTables() const {
    return VTTVTables;
  }

  /// Returns a reference to the sub-VTT indices.
  const toolchain::DenseMap<BaseSubobject, uint64_t> &getSubVTTIndices() const {
    return SubVTTIndices;
  }

  /// Returns a reference to the secondary virtual pointer indices.
  const toolchain::DenseMap<BaseSubobject, uint64_t> &
  getSecondaryVirtualPointerIndices() const {
    return SecondaryVirtualPointerIndices;
  }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_VTTBUILDER_H
