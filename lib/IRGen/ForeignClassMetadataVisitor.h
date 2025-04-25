//===-- ForeignClassMetadataVisitor.h - foreign class metadata -*- C++ --*-===//
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
// A CRTP class useful for laying out foreign class metadata.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_FOREIGNCLASSMETADATAVISITOR_H
#define SWIFT_IRGEN_FOREIGNCLASSMETADATAVISITOR_H

#include "NominalMetadataVisitor.h"

namespace language {
namespace irgen {

/// A CRTP layout class for foreign class metadata.
template <class Impl>
class ForeignClassMetadataVisitor
       : public NominalMetadataVisitor<Impl> {
  using super = NominalMetadataVisitor<Impl>;
protected:
  ClassDecl *Target;
  using super::asImpl;
public:
  ForeignClassMetadataVisitor(IRGenModule &IGM, ClassDecl *target)
    : super(IGM), Target(target) {}

  void layout() {
    asImpl().addLayoutStringPointer();
    super::layout();
    asImpl().addNominalTypeDescriptor();
    asImpl().addSuperclass();
    asImpl().addReservedWord();
  }

  CanType getTargetType() const {
    return Target->getDeclaredType()->getCanonicalType();
  }
};

/// An "implementation" of ForeignClassMetadataVisitor that just scans through
/// the metadata layout, maintaining the offset of the next field.
template <class Impl>
class ForeignClassMetadataScanner : public ForeignClassMetadataVisitor<Impl> {
  using super = ForeignClassMetadataVisitor<Impl>;

protected:
  Size NextOffset = Size(0);

  ForeignClassMetadataScanner(IRGenModule &IGM, ClassDecl *target)
    : super(IGM, target) {}

public:
  void addMetadataFlags() { addPointer(); }
  void addLayoutStringPointer() { addPointer(); }
  void addValueWitnessTable() { addPointer(); }
  void addNominalTypeDescriptor() { addPointer(); }
  void addSuperclass() { addPointer(); }
  void addReservedWord() { addPointer(); }

private:
  void addPointer() {
    NextOffset += super::IGM.getPointerSize();
  }
};

template <class Impl>
class ForeignReferenceTypeMetadataVisitor
    : public NominalMetadataVisitor<Impl> {
  using super = NominalMetadataVisitor<Impl>;
protected:
  ClassDecl *Target;
  using super::asImpl;
public:
  ForeignReferenceTypeMetadataVisitor(IRGenModule &IGM, ClassDecl *target)
      : super(IGM), Target(target) {}

  void layout() {
    asImpl().addLayoutStringPointer();
    super::layout();
    asImpl().addNominalTypeDescriptor();
    asImpl().addReservedWord();
  }

  CanType getTargetType() const {
    return Target->getDeclaredType()->getCanonicalType();
  }
};

} // end namespace irgen
} // end namespace language

#endif // SWIFT_IRGEN_FOREIGNCLASSMETADATAVISITOR_H
