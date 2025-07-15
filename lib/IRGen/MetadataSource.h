//===--- MetadataSource.h - structure for the source of metadata *- C++ -*-===//
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

#ifndef LANGUAGE_IRGEN_METADATA_SOURCE_H
#define LANGUAGE_IRGEN_METADATA_SOURCE_H

#include "language/AST/Types.h"

namespace language {
namespace irgen {

class MetadataSource {
public:
  enum class Kind {
    /// Metadata is derived from a source class pointer.
    ClassPointer,

    /// Metadata is derived from a type metadata pointer.
    Metadata,

    /// Metadata is derived from the origin type parameter.
    GenericLValueMetadata,

    /// Metadata is obtained directly from the from a Self metadata
    /// parameter passed via the WitnessMethod convention.
    SelfMetadata,

    /// Metadata is derived from the Self witness table parameter
    /// passed via the WitnessMethod convention.
    SelfWitnessTable,

    /// Metadata is obtained directly from the FixedType indicated. Used with
    /// Objective-C generics, where the actual argument is erased at runtime
    /// and its existential bound is used instead.
    ErasedTypeMetadata,
  };

  static bool requiresSourceIndex(Kind kind) {
    return (kind == Kind::ClassPointer ||
            kind == Kind::Metadata ||
            kind == Kind::GenericLValueMetadata);
  }

  static bool requiresFixedType(Kind kind) {
    return (kind == Kind::ErasedTypeMetadata);
  }

  enum : unsigned { InvalidSourceIndex = ~0U };

private:
  /// The kind of source this is.
  Kind TheKind;

  /// For ClassPointer, Metadata, and GenericLValueMetadata, the source index;
  /// for ErasedTypeMetadata, the type; for others, Index should be set to
  /// InvalidSourceIndex.
  union {
    unsigned Index;
    CanType FixedType;
  };

public:
  CanType Type;

  MetadataSource(Kind kind, CanType type)
    : TheKind(kind), Index(InvalidSourceIndex), Type(type)
  {
    assert(!requiresSourceIndex(kind) && !requiresFixedType(kind));
  }


  MetadataSource(Kind kind, CanType type, unsigned index)
    : TheKind(kind), Index(index), Type(type) {
    assert(requiresSourceIndex(kind));
    assert(index != InvalidSourceIndex);
  }

  MetadataSource(Kind kind, CanType type, CanType fixedType)
    : TheKind(kind), FixedType(fixedType), Type(type) {
    assert(requiresFixedType(kind));
  }

  Kind getKind() const { return TheKind; }

  unsigned getParamIndex() const {
    assert(requiresSourceIndex(getKind()));
    return Index;
  }

  CanType getFixedType() const {
    assert(requiresFixedType(getKind()));
    return FixedType;
  }
};

} // end namespace irgen
} // end namespace language

#endif
