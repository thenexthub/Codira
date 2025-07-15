//===-- LocalTypeDataKind.h - Kinds of locally-cached type data -*- C++ -*-===//
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
//  This file defines the LocalTypeDataKind class, which opaquely
//  represents a particular kind of local type data that we might
//  want to cache during emission.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_LOCALTYPEDATAKIND_H
#define LANGUAGE_IRGEN_LOCALTYPEDATAKIND_H

#include "language/AST/PackConformance.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/Type.h"
#include "language/IRGen/ValueWitness.h"
#include <stdint.h>
#include "toolchain/ADT/DenseMapInfo.h"

namespace language {
  class ProtocolDecl;

namespace irgen {

/// The kind of local type data we might want to store for a type.
class LocalTypeDataKind {
public:
  using RawType = uintptr_t;
private:
  RawType Value;
  
  explicit LocalTypeDataKind(RawType Value) : Value(Value) {}
  
  /// Magic values for special kinds of type metadata.  These should be
  /// small so that they should never conflict with a valid pointer.
  ///
  /// Since this representation is opaque, we don't worry about being able
  /// to distinguish different kinds of pointer; we just assume that e.g. a
  /// ProtocolConformance will never have the same address as a Decl.
  enum : RawType {
    FormalTypeMetadata,
    RepresentationTypeMetadata,
    ValueWitnessTable,
    Shape,
    GenericValue,
    // <- add more special cases here

    // The first enumerator for an individual value witness.
    ValueWitnessBase,

    // The first enumerator for an individual value witness discriminator.
    ValueWitnessDiscriminatorBase = ValueWitnessBase + MaxNumValueWitnesses,

    FirstPayloadValue = 2048,
    Kind_Decl = 0b0,
    Kind_Conformance = 0b1,
    Kind_PackConformance = 0b10,
    KindMask = 0b11,
  };

public:
  LocalTypeDataKind() = default;
  
  // The magic values are all odd and so do not collide with pointer values.
  
  /// A reference to the formal type metadata.
  static LocalTypeDataKind forFormalTypeMetadata() {
    return LocalTypeDataKind(FormalTypeMetadata);
  }

  /// A reference to type metadata for a representation-compatible type.
  static LocalTypeDataKind forRepresentationTypeMetadata() {
    return LocalTypeDataKind(RepresentationTypeMetadata);
  }

  /// A reference to the value witness table for a representation-compatible
  /// type.
  static LocalTypeDataKind forValueWitnessTable() {
    return LocalTypeDataKind(ValueWitnessTable);
  }

  /// A reference to a specific value witness for a representation-compatible
  /// type.
  static LocalTypeDataKind forValueWitness(ValueWitness witness) {
    return LocalTypeDataKind(ValueWitnessBase + (unsigned)witness);
  }

  /// The discriminator for a specific value witness.
  static LocalTypeDataKind forValueWitnessDiscriminator(ValueWitness witness) {
    return LocalTypeDataKind(ValueWitnessDiscriminatorBase + (unsigned)witness);
  }
  
  /// A reference to the shape expression of a pack type.
  static LocalTypeDataKind forPackShapeExpression() {
    return LocalTypeDataKind(Shape);
  }

  /// A reference to the value of a variable generic argument.
  static LocalTypeDataKind forValue() {
    return LocalTypeDataKind(GenericValue);
  }

  /// A reference to a protocol witness table for an archetype.
  ///
  /// This only works for non-concrete types because in principle we might
  /// have multiple concrete conformances for a concrete type used in the
  /// same function.
  static LocalTypeDataKind
  forAbstractProtocolWitnessTable(ProtocolDecl *protocol) {
    ASSERT(protocol && "protocol reference may not be null");
    return LocalTypeDataKind(uintptr_t(protocol) | Kind_Decl);
  }

  /// A reference to a protocol witness table for a concrete type.
  static LocalTypeDataKind
  forConcreteProtocolWitnessTable(ProtocolConformance *conformance) {
    ASSERT(conformance && "conformance reference may not be null");
    return LocalTypeDataKind(uintptr_t(conformance) | Kind_Conformance);
  }

  static LocalTypeDataKind forProtocolWitnessTablePack(PackConformance *pack) {
    ASSERT(pack && "pack conformance reference may not be null");
    return LocalTypeDataKind(uintptr_t(pack) | Kind_PackConformance);
  }

  static LocalTypeDataKind
  forProtocolWitnessTable(ProtocolConformanceRef conformance) {
    if (conformance.isConcrete()) {
      return forConcreteProtocolWitnessTable(conformance.getConcrete());
    } else if (conformance.isPack()) {
      return forProtocolWitnessTablePack(conformance.getPack());
    } else {
      return forAbstractProtocolWitnessTable(conformance.getProtocol());
    }
  }

  LocalTypeDataKind getCachingKind() const;

  bool isAnyTypeMetadata() const {
    return Value == FormalTypeMetadata ||
           Value == RepresentationTypeMetadata;
  }

  bool isSingletonKind() const {
    return (Value < FirstPayloadValue);
  }

  bool isConcreteProtocolConformance() const {
    return (!isSingletonKind() &&
            ((Value & KindMask) == Kind_Conformance));
  }

  ProtocolConformance *getConcreteProtocolConformance() const {
    assert(isConcreteProtocolConformance());
    return reinterpret_cast<ProtocolConformance*>(Value - Kind_Conformance);
  }

  bool isAbstractProtocolConformance() const {
    return (!isSingletonKind() &&
            ((Value & KindMask) == Kind_Decl));
  }

  ProtocolDecl *getAbstractProtocolConformance() const {
    assert(isAbstractProtocolConformance());
    return reinterpret_cast<ProtocolDecl*>(Value - Kind_Decl);
  }

  bool isPackProtocolConformance() const {
    return (!isSingletonKind() &&
            ((Value & KindMask) == Kind_PackConformance));
  }

  PackConformance *getPackProtocolConformance() const {
    assert(isPackProtocolConformance());
    return reinterpret_cast<PackConformance*>(Value - Kind_PackConformance);
  }

  ProtocolDecl *getConformedProtocol() const {
    assert(!isSingletonKind());
    if ((Value & KindMask) == Kind_Decl) {
      return getAbstractProtocolConformance();
    } else if ((Value & KindMask) == Kind_PackConformance) {
      return getPackProtocolConformance()->getProtocol();
    } else {
      assert((Value & KindMask) == Kind_Conformance);
      return getConcreteProtocolConformance()->getProtocol();
    }
  }

  ProtocolConformanceRef getProtocolConformance(Type conformingType) const {
    assert(!isSingletonKind());
    if ((Value & KindMask) == Kind_Decl) {
      return ProtocolConformanceRef::forAbstract(
          conformingType, getAbstractProtocolConformance());
    } else if ((Value & KindMask) == Kind_PackConformance) {
      return ProtocolConformanceRef(getPackProtocolConformance());
    } else {
      assert((Value & KindMask) == Kind_Conformance);
      return ProtocolConformanceRef(getConcreteProtocolConformance());
    }
  }
  
  RawType getRawValue() const {
    return Value;
  }

  void dump() const;
  void print(toolchain::raw_ostream &out) const;

  bool operator==(LocalTypeDataKind other) const {
    return Value == other.Value;
  }
  bool operator!=(LocalTypeDataKind other) const {
    return Value != other.Value;
  }
};

class LocalTypeDataKey {
public:
  CanType Type;
  LocalTypeDataKind Kind;

  LocalTypeDataKey(CanType type, LocalTypeDataKind kind)
    : Type(type), Kind(kind) {}

  LocalTypeDataKey getCachingKey() const;

  bool operator==(const LocalTypeDataKey &other) const {
    return Type == other.Type && Kind == other.Kind;
  }

  void dump() const;
  void print(toolchain::raw_ostream &out) const;
};

}
}

namespace toolchain {
template <> struct DenseMapInfo<language::irgen::LocalTypeDataKey> {
  using LocalTypeDataKey = language::irgen::LocalTypeDataKey;
  using CanTypeInfo = DenseMapInfo<language::CanType>;
  static inline LocalTypeDataKey getEmptyKey() {
    return { CanTypeInfo::getEmptyKey(),
             language::irgen::LocalTypeDataKind::forFormalTypeMetadata() };
  }
  static inline LocalTypeDataKey getTombstoneKey() {
    return { CanTypeInfo::getTombstoneKey(),
             language::irgen::LocalTypeDataKind::forFormalTypeMetadata() };
  }
  static unsigned getHashValue(const LocalTypeDataKey &key) {
    return detail::combineHashValue(CanTypeInfo::getHashValue(key.Type),
                                    key.Kind.getRawValue());
  }
  static bool isEqual(const LocalTypeDataKey &a, const LocalTypeDataKey &b) {
    return a == b;
  }
};
}

#endif
