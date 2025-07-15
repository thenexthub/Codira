//===--- NominalMetadataVisitor.h - CRTP for metadata layout ----*- C++ -*-===//
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
// A CRTP helper class for visiting all of the fields in a nominal type
// metadata object.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_NOMINALMETADATAVISITOR_H
#define LANGUAGE_IRGEN_NOMINALMETADATAVISITOR_H

#include "GenericRequirement.h"
#include "GenProto.h"
#include "GenMeta.h"
#include "IRGenModule.h"
#include "MetadataVisitor.h"

namespace language {
namespace irgen {

/// A CRTP class for laying out type metadata for nominal types. Note that this
/// does *not* handle the metadata template stuff.
template <class Impl> class NominalMetadataVisitor
       : public MetadataVisitor<Impl> {
  using super = MetadataVisitor<Impl>;

protected:
  using super::asImpl;

  NominalMetadataVisitor(IRGenModule &IGM) : super(IGM) {}

public:
  /// Add fields related to the generics of this class declaration.
  /// TODO: don't add new fields that are implied by the superclass
  /// fields.  e.g., if B<T> extends A<T>, the witness for T in A's
  /// section should be enough.
  template <class... T>
  void addGenericFields(NominalTypeDecl *typeDecl, T &&...args) {
    // The archetype order here needs to be consistent with
    // NominalTypeDescriptorBase::addGenericParams.
    
    // Note that we intentionally don't std::forward 'args'.
    asImpl().noteStartOfGenericRequirements(args...);

    GenericTypeRequirements requirements(super::IGM, typeDecl);
    for (auto reqt : requirements.getRequirements()) {
      asImpl().addGenericRequirement(reqt, args...);
    }

    asImpl().noteEndOfGenericRequirements(args...);
  }

  template <class... T>
  void noteStartOfGenericRequirements(T &&...args) {}

  template <class... T>
  void noteEndOfGenericRequirements(T &&...args) {}
};

} // end namespace irgen
} // end namespace language

#endif
