//===- RecordLayout.h - Layout information for a struct/union ---*- C++ -*-===//
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
//  This file defines the RecordLayout interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_RECORDLAYOUT_H
#define LANGUAGE_CORE_AST_RECORDLAYOUT_H

#include "language/Core/AST/ASTVector.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#include <cassert>
#include <cstdint>

namespace language::Core {

class ASTContext;
class CXXRecordDecl;

/// ASTRecordLayout -
/// This class contains layout information for one RecordDecl,
/// which is a struct/union/class.  The decl represented must be a definition,
/// not a forward declaration.
/// This class is also used to contain layout information for one
/// ObjCInterfaceDecl. FIXME - Find appropriate name.
/// These objects are managed by ASTContext.
class ASTRecordLayout {
public:
  struct VBaseInfo {
    /// The offset to this virtual base in the complete-object layout
    /// of this class.
    CharUnits VBaseOffset;

  private:
    /// Whether this virtual base requires a vtordisp field in the
    /// Microsoft ABI.  These fields are required for certain operations
    /// in constructors and destructors.
    bool HasVtorDisp = false;

  public:
    VBaseInfo() = default;
    VBaseInfo(CharUnits VBaseOffset, bool hasVtorDisp)
        : VBaseOffset(VBaseOffset), HasVtorDisp(hasVtorDisp) {}

    bool hasVtorDisp() const { return HasVtorDisp; }
  };

  using VBaseOffsetsMapTy = toolchain::DenseMap<const CXXRecordDecl *, VBaseInfo>;

private:
  friend class ASTContext;

  /// Size - Size of record in characters.
  CharUnits Size;

  /// DataSize - Size of record in characters without tail padding.
  CharUnits DataSize;

  // Alignment - Alignment of record in characters.
  CharUnits Alignment;

  // PreferredAlignment - Preferred alignment of record in characters. This
  // can be different than Alignment in cases where it is beneficial for
  // performance or backwards compatibility preserving (e.g. AIX-ABI).
  CharUnits PreferredAlignment;

  // UnadjustedAlignment - Alignment of record in characters before alignment
  // adjustments. Maximum of the alignments of the record members and base
  // classes in characters.
  CharUnits UnadjustedAlignment;

  /// RequiredAlignment - The required alignment of the object.  In the MS-ABI
  /// the __declspec(align()) trumps #pramga pack and must always be obeyed.
  CharUnits RequiredAlignment;

  /// FieldOffsets - Array of field offsets in bits.
  ASTVector<uint64_t> FieldOffsets;

  /// CXXRecordLayoutInfo - Contains C++ specific layout information.
  struct CXXRecordLayoutInfo {
    /// NonVirtualSize - The non-virtual size (in chars) of an object, which is
    /// the size of the object without virtual bases.
    CharUnits NonVirtualSize;

    /// NonVirtualAlignment - The non-virtual alignment (in chars) of an object,
    /// which is the alignment of the object without virtual bases.
    CharUnits NonVirtualAlignment;

    /// PreferredNVAlignment - The preferred non-virtual alignment (in chars) of
    /// an object, which is the preferred alignment of the object without
    /// virtual bases.
    CharUnits PreferredNVAlignment;

    /// SizeOfLargestEmptySubobject - The size of the largest empty subobject
    /// (either a base or a member). Will be zero if the class doesn't contain
    /// any empty subobjects.
    CharUnits SizeOfLargestEmptySubobject;

    /// VBPtrOffset - Virtual base table offset (Microsoft-only).
    CharUnits VBPtrOffset;

    /// HasOwnVFPtr - Does this class provide a virtual function table
    /// (vtable in Itanium, vftbl in Microsoft) that is independent from
    /// its base classes?
    bool HasOwnVFPtr : 1;

    /// HasVFPtr - Does this class have a vftable that could be extended by
    /// a derived class.  The class may have inherited this pointer from
    /// a primary base class.
    bool HasExtendableVFPtr : 1;

    /// EndsWithZeroSizedObject - True if this class contains a zero sized
    /// member or base or a base with a zero sized member or base.
    /// Only used for MS-ABI.
    bool EndsWithZeroSizedObject : 1;

    /// True if this class is zero sized or first base is zero sized or
    /// has this property.  Only used for MS-ABI.
    bool LeadsWithZeroSizedBase : 1;

    /// PrimaryBase - The primary base info for this record.
    toolchain::PointerIntPair<const CXXRecordDecl *, 1, bool> PrimaryBase;

    /// BaseSharingVBPtr - The base we share vbptr with.
    const CXXRecordDecl *BaseSharingVBPtr;

    /// FIXME: This should really use a SmallPtrMap, once we have one in LLVM :)
    using BaseOffsetsMapTy = toolchain::DenseMap<const CXXRecordDecl *, CharUnits>;

    /// BaseOffsets - Contains a map from base classes to their offset.
    BaseOffsetsMapTy BaseOffsets;

    /// VBaseOffsets - Contains a map from vbase classes to their offset.
    VBaseOffsetsMapTy VBaseOffsets;
  };

  /// CXXInfo - If the record layout is for a C++ record, this will have
  /// C++ specific information about the record.
  CXXRecordLayoutInfo *CXXInfo = nullptr;

  ASTRecordLayout(const ASTContext &Ctx, CharUnits size, CharUnits alignment,
                  CharUnits preferredAlignment, CharUnits unadjustedAlignment,
                  CharUnits requiredAlignment, CharUnits datasize,
                  ArrayRef<uint64_t> fieldoffsets);

  using BaseOffsetsMapTy = CXXRecordLayoutInfo::BaseOffsetsMapTy;

  // Constructor for C++ records.
  ASTRecordLayout(const ASTContext &Ctx, CharUnits size, CharUnits alignment,
                  CharUnits preferredAlignment, CharUnits unadjustedAlignment,
                  CharUnits requiredAlignment, bool hasOwnVFPtr,
                  bool hasExtendableVFPtr, CharUnits vbptroffset,
                  CharUnits datasize, ArrayRef<uint64_t> fieldoffsets,
                  CharUnits nonvirtualsize, CharUnits nonvirtualalignment,
                  CharUnits preferrednvalignment,
                  CharUnits SizeOfLargestEmptySubobject,
                  const CXXRecordDecl *PrimaryBase, bool IsPrimaryBaseVirtual,
                  const CXXRecordDecl *BaseSharingVBPtr,
                  bool EndsWithZeroSizedObject, bool LeadsWithZeroSizedBase,
                  const BaseOffsetsMapTy &BaseOffsets,
                  const VBaseOffsetsMapTy &VBaseOffsets);

  ~ASTRecordLayout() = default;

  void Destroy(ASTContext &Ctx);

public:
  ASTRecordLayout(const ASTRecordLayout &) = delete;
  ASTRecordLayout &operator=(const ASTRecordLayout &) = delete;

  /// getAlignment - Get the record alignment in characters.
  CharUnits getAlignment() const { return Alignment; }

  /// getPreferredFieldAlignment - Get the record preferred alignment in
  /// characters.
  CharUnits getPreferredAlignment() const { return PreferredAlignment; }

  /// getUnadjustedAlignment - Get the record alignment in characters, before
  /// alignment adjustment.
  CharUnits getUnadjustedAlignment() const { return UnadjustedAlignment; }

  /// getSize - Get the record size in characters.
  CharUnits getSize() const { return Size; }

  /// getFieldCount - Get the number of fields in the layout.
  unsigned getFieldCount() const { return FieldOffsets.size(); }

  /// getFieldOffset - Get the offset of the given field index, in
  /// bits.
  uint64_t getFieldOffset(unsigned FieldNo) const {
    return FieldOffsets[FieldNo];
  }

  /// getDataSize() - Get the record data size, which is the record size
  /// without tail padding, in characters.
  CharUnits getDataSize() const { return DataSize; }

  /// getNonVirtualSize - Get the non-virtual size (in chars) of an object,
  /// which is the size of the object without virtual bases.
  CharUnits getNonVirtualSize() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    return CXXInfo->NonVirtualSize;
  }

  /// getNonVirtualAlignment - Get the non-virtual alignment (in chars) of an
  /// object, which is the alignment of the object without virtual bases.
  CharUnits getNonVirtualAlignment() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    return CXXInfo->NonVirtualAlignment;
  }

  /// getPreferredNVAlignment - Get the preferred non-virtual alignment (in
  /// chars) of an object, which is the preferred alignment of the object
  /// without virtual bases.
  CharUnits getPreferredNVAlignment() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    return CXXInfo->PreferredNVAlignment;
  }

  /// getPrimaryBase - Get the primary base for this record.
  const CXXRecordDecl *getPrimaryBase() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    return CXXInfo->PrimaryBase.getPointer();
  }

  /// isPrimaryBaseVirtual - Get whether the primary base for this record
  /// is virtual or not.
  bool isPrimaryBaseVirtual() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    return CXXInfo->PrimaryBase.getInt();
  }

  /// getBaseClassOffset - Get the offset, in chars, for the given base class.
  CharUnits getBaseClassOffset(const CXXRecordDecl *Base) const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    Base = Base->getDefinition();
    assert(CXXInfo->BaseOffsets.count(Base) && "Did not find base!");

    return CXXInfo->BaseOffsets[Base];
  }

  /// getVBaseClassOffset - Get the offset, in chars, for the given base class.
  CharUnits getVBaseClassOffset(const CXXRecordDecl *VBase) const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");

    VBase = VBase->getDefinition();
    assert(CXXInfo->VBaseOffsets.count(VBase) && "Did not find base!");

    return CXXInfo->VBaseOffsets[VBase].VBaseOffset;
  }

  CharUnits getSizeOfLargestEmptySubobject() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->SizeOfLargestEmptySubobject;
  }

  /// hasOwnVFPtr - Does this class provide its own virtual-function
  /// table pointer, rather than inheriting one from a primary base
  /// class?  If so, it is at offset zero.
  ///
  /// This implies that the ABI has no primary base class, meaning
  /// that it has no base classes that are suitable under the conditions
  /// of the ABI.
  bool hasOwnVFPtr() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->HasOwnVFPtr;
  }

  /// hasVFPtr - Does this class have a virtual function table pointer
  /// that can be extended by a derived class?  This is synonymous with
  /// this class having a VFPtr at offset zero.
  bool hasExtendableVFPtr() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->HasExtendableVFPtr;
  }

  /// hasOwnVBPtr - Does this class provide its own virtual-base
  /// table pointer, rather than inheriting one from a primary base
  /// class?
  ///
  /// This implies that the ABI has no primary base class, meaning
  /// that it has no base classes that are suitable under the conditions
  /// of the ABI.
  bool hasOwnVBPtr() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return hasVBPtr() && !CXXInfo->BaseSharingVBPtr;
  }

  /// hasVBPtr - Does this class have a virtual function table pointer.
  bool hasVBPtr() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return !CXXInfo->VBPtrOffset.isNegative();
  }

  CharUnits getRequiredAlignment() const { return RequiredAlignment; }

  bool endsWithZeroSizedObject() const {
    return CXXInfo && CXXInfo->EndsWithZeroSizedObject;
  }

  bool leadsWithZeroSizedBase() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->LeadsWithZeroSizedBase;
  }

  /// getVBPtrOffset - Get the offset for virtual base table pointer.
  /// This is only meaningful with the Microsoft ABI.
  CharUnits getVBPtrOffset() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->VBPtrOffset;
  }

  const CXXRecordDecl *getBaseSharingVBPtr() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->BaseSharingVBPtr;
  }

  const VBaseOffsetsMapTy &getVBaseOffsetsMap() const {
    assert(CXXInfo && "Record layout does not have C++ specific info!");
    return CXXInfo->VBaseOffsets;
  }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_RECORDLAYOUT_H
