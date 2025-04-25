//===--- EnumMetadataVisitor.h - CRTP for enum metadata ---------*- C++ -*-===//
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
// A CRTP class useful for visiting all of the fields in an
// enum metadata object.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_ENUMMETADATAVISITOR_H
#define SWIFT_IRGEN_ENUMMETADATAVISITOR_H

#include "NominalMetadataVisitor.h"
#include "GenEnum.h"

namespace language {
namespace irgen {

/// A CRTP class for laying out enum metadata.
///
/// This produces an object corresponding to the EnumMetadata type.
/// It does not itself doing anything special for metadata templates.
template <class Impl> class EnumMetadataVisitor
       : public NominalMetadataVisitor<Impl> {
  using super = NominalMetadataVisitor<Impl>;

protected:
  using super::IGM;
  using super::asImpl;

  /// The Enum.
  EnumDecl *const Target;

  EnumMetadataVisitor(IRGenModule &IGM, EnumDecl *target)
    : super(IGM), Target(target) {}

public:
  void layout() {
    static_assert(MetadataAdjustmentIndex::ValueType == 2,
                  "Adjustment index must be synchronized with this layout");

    asImpl().addLayoutStringPointer();

    // Metadata header.
    super::layout();

    // EnumMetadata header.
    asImpl().addNominalTypeDescriptor();

    // Everything after this is type-specific.
    asImpl().noteStartOfTypeSpecificMembers();

    // Generic arguments.
    // This must always be the first piece of trailing data.
    asImpl().addGenericFields(Target);

    // Reserve a word to cache the payload size if the type has dynamic layout.
    auto &strategy = getEnumImplStrategy(IGM,
           Target->getDeclaredTypeInContext()->getCanonicalType());
    if (strategy.needsPayloadSizeInMetadata())
      asImpl().addPayloadSize();

    if (asImpl().hasTrailingFlags())
      asImpl().addTrailingFlags();
  }

  bool hasTrailingFlags() {
    return Target->isGenericContext() &&
           IGM.shouldPrespecializeGenericMetadata();
  }
};

/// An "implementation" of EnumMetadataVisitor that just scans through
/// the metadata layout, maintaining the next index: the offset (in
/// pointer-sized chunks) into the metadata for the next field.
template <class Impl>
class EnumMetadataScanner : public EnumMetadataVisitor<Impl> {
  using super = EnumMetadataVisitor<Impl>;

protected:
  Size NextOffset = Size(0);

  EnumMetadataScanner(IRGenModule &IGM, EnumDecl *target)
    : super(IGM, target) {}

public:
  void addMetadataFlags() { addPointer(); }
  void addLayoutStringPointer() { addPointer(); }
  void addValueWitnessTable() { addPointer(); }
  void addNominalTypeDescriptor() { addPointer(); }
  void addGenericRequirement(GenericRequirement requirement) { addPointer(); }
  void addPayloadSize() { addPointer(); }
  void noteStartOfTypeSpecificMembers() {}
  void addTrailingFlags() { addInt64(); }

private:
  void addPointer() {
    NextOffset += super::IGM.getPointerSize();
  }
  void addInt64() { NextOffset += Size(8); }
};

} // end namespace irgen
} // end namespace language

#endif
