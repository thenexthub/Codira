//===--- SILLayout.h - Defines SIL-level aggregate layouts ------*- C++ -*-===//
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
// This file defines classes that describe the physical layout of nominal
// types in SIL, including structs, classes, and boxes. This is distinct from
// the AST-level layout for several reasons:
// - It avoids redundant work lowering the layout of aggregates from the AST.
// - It allows optimizations to manipulate the layout of aggregates without
//   requiring changes to the AST. For instance, optimizations can eliminate
//   dead fields from instances or turn invariant fields into global variables.
// - It allows for SIL-only aggregates to exist, such as boxes.
// - It improves the robustness of code in the face of resilience. A resilient
//   type can be modeled in SIL as not having a layout at all, preventing the
//   inappropriate use of fragile projection and injection operations on the
//   type.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_LAYOUT_H
#define LANGUAGE_SIL_LAYOUT_H

#include "toolchain/ADT/PointerIntPair.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Identifier.h"
#include "language/AST/Type.h"

namespace language {

class SILType;

/// A field of a SIL aggregate layout.
class SILField final {
  enum : unsigned {
    IsMutable = 0x1,
  };
  
  static constexpr const unsigned NumFlags = 1;

  toolchain::PointerIntPair<CanType, NumFlags, unsigned> LoweredTypeAndFlags;
  
  static unsigned getFlagsValue(bool Mutable) {
    unsigned flags = 0;
    if (Mutable) flags |= IsMutable;
    
    assert(flags >> NumFlags == 0
           && "more flags than flag bits?!");
    return flags;
  }
  
public:
  SILField(CanType LoweredType, bool Mutable)
    : LoweredTypeAndFlags(LoweredType, getFlagsValue(Mutable))
  {}
  
  /// Get the lowered type of the field in the aggregate.
  ///
  /// This must be a lowered SIL type. If the containing aggregate is generic,
  /// then this type specifies the abstraction pattern at which values stored
  /// in this aggregate should be lowered.
  CanType getLoweredType() const { return LoweredTypeAndFlags.getPointer(); }
  
  SILType getAddressType() const; // In SILType.h
  SILType getObjectType() const; // In SILType.h
  
  /// True if this field is mutable inside its aggregate.
  ///
  /// This is only effectively a constraint on shared mutable reference types,
  /// such as classes and boxes. A value type or uniquely-owned immutable
  /// reference can always be mutated, since doing so is equivalent to
  /// destroying the old value and emplacing a new value with the modified
  /// field in the same place.
  bool isMutable() const { return LoweredTypeAndFlags.getInt() & IsMutable; }
};

/// A layout.
class SILLayout final : public toolchain::FoldingSetNode,
                        private toolchain::TrailingObjects<SILLayout, SILField>
{
  friend TrailingObjects;

  enum : unsigned {
    IsMutable = 0x1,
    CapturesGenericEnvironment = 0x2,
  };
  
  static constexpr const unsigned NumFlags = 2;
  
  static unsigned getFlagsValue(bool Mutable, bool CapturesGenerics) {
    unsigned flags = 0;
    if (Mutable)
      flags |= IsMutable;
    if (CapturesGenerics)
      flags |= CapturesGenericEnvironment;
    
    assert(flags >> NumFlags == 0
           && "more flags than flag bits?!");
    return flags;
  }
  
  toolchain::PointerIntPair<CanGenericSignature, NumFlags, unsigned>
    GenericSigAndFlags;
  
  unsigned NumFields;
  
  SILLayout(CanGenericSignature Signature,
            ArrayRef<SILField> Fields,
            bool CapturesGenericEnvironment);
  
  SILLayout(const SILLayout &) = delete;
  SILLayout &operator=(const SILLayout &) = delete;
public:
  /// Get or create a layout.
  static SILLayout *get(ASTContext &C,
                        CanGenericSignature Generics,
                        ArrayRef<SILField> Fields,
                        bool CapturesGenericEnvironment);
  
  /// Get the generic signature in which this layout exists.
  CanGenericSignature getGenericSignature() const {
    return GenericSigAndFlags.getPointer();
  }
  
  /// True if the layout contains any mutable fields.
  bool isMutable() const {
    return GenericSigAndFlags.getInt() & IsMutable;
  }
  
  /// True if the layout captures the generic arguments it is substituted with
  /// and can provide generic bindings when passed as a closure argument.
  bool capturesGenericEnvironment() const {
    return GenericSigAndFlags.getInt() & CapturesGenericEnvironment;
  }
  
  /// Get the fields inside the layout.
  ArrayRef<SILField> getFields() const {
    return toolchain::ArrayRef(getTrailingObjects<SILField>(), NumFields);
  }
  
  /// Produce a profile of this layout, for use in a folding set.
  static void Profile(toolchain::FoldingSetNodeID &id,
                      CanGenericSignature Generics,
                      ArrayRef<SILField> Fields,
                      bool CapturesGenericEnvironment);
  
  /// Produce a profile of this locator, for use in a folding set.
  void Profile(toolchain::FoldingSetNodeID &id) {
    Profile(id, getGenericSignature(), getFields(),
            capturesGenericEnvironment());
  }
};

} // end namespace language

#endif // LANGUAGE_SIL_LAYOUT_H
