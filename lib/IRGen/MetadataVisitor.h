//===--- MetadataVisitor.h - CRTP for metadata layout -----------*- C++ -*-===//
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
// A CRTP helper class for preparing the common metadata of all metadata
// objects.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_METADATAVISITOR_H
#define SWIFT_IRGEN_METADATAVISITOR_H

namespace language {
namespace irgen {

/// A CRTP class for laying out a type metadata's common fields. Note that this
/// does *not* handle the metadata template stuff.
template <class Impl> class MetadataVisitor {
protected:
  Impl &asImpl() { return *static_cast<Impl*>(this); }

  IRGenModule &IGM;

  MetadataVisitor(IRGenModule &IGM) : IGM(IGM) {}

public:
  void layout() {
    // Common fields.
    asImpl().addValueWitnessTable();
    asImpl().noteAddressPoint();
    asImpl().addMetadataFlags();
  }

  /// This is the address point.
  void noteAddressPoint() {}
};

} // end namespace irgen
} // end namespace language

#endif
