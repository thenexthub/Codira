//===--- GenericRequirement.h - Generic requirement -------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_GENERIC_REQUIREMENT_H
#define LANGUAGE_AST_GENERIC_REQUIREMENT_H

#include "language/AST/Decl.h"
#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "toolchain/Support/raw_ostream.h"

namespace toolchain {
class Type;
}

namespace language {

class ProtocolDecl;
namespace irgen {
class IRGenModule;
}

/// The four kinds of entities passed in the runtime calling convention for
/// generic code: pack shapes, type metadata, witness tables, and values.
///
/// A pack shape describes an equivalence class of type parameter packs; the
/// runtime value is a single integer, which is the length of the pack.
///
/// Type metadata is emitted for each reduced generic parameter (that is,
/// not same-type constrained to another generic parameter or concrete type).
///
/// A witness table is emitted for each conformance requirement in the
/// generic signature.
///
/// A value is emitted for each variable generic parameter, 'let N'.
class GenericRequirement {
public:
  enum class Kind : uint8_t {
    Shape,
    Metadata,
    WitnessTable,
    MetadataPack,
    WitnessTablePack,
    Value,
  };

private:
  Kind kind;
  CanType type;
  ProtocolDecl *proto;

public:
  GenericRequirement(Kind kind, CanType type, ProtocolDecl *proto)
    : kind(kind), type(type), proto(proto) {}

  Kind getKind() const {
    return kind;
  }

  CanType getTypeParameter() const {
    return type;
  }

  ProtocolDecl *getProtocol() const {
    return proto;
  }

  bool isShape() const {
    return kind == Kind::Shape;
  }

  static GenericRequirement forShape(CanType type) {
    assert(type->isParameterPack() || isa<PackArchetypeType>(type));
    return GenericRequirement(Kind::Shape, type, nullptr);
  }

  bool isMetadata() const {
    return kind == Kind::Metadata;
  }
  bool isMetadataPack() const {
    return kind == Kind::MetadataPack;
  }
  bool isAnyMetadata() const {
    return kind == Kind::Metadata || kind == Kind::MetadataPack;
  }

  static GenericRequirement forMetadata(CanType type) {
    auto kind = ((type->isParameterPack() ||
                  isa<PackArchetypeType>(type))
                 ? Kind::MetadataPack : Kind::Metadata);
    return GenericRequirement(kind, type, nullptr);
  }

  bool isWitnessTable() const {
    return kind == Kind::WitnessTable;
  }
  bool isWitnessTablePack() const {
    return kind == Kind::WitnessTablePack;
  }
  bool isAnyWitnessTable() const {
    return kind == Kind::WitnessTable || kind == Kind::WitnessTablePack;
  }
  bool isValue() const {
    return kind == Kind::Value;
  }

  static GenericRequirement forWitnessTable(CanType type, ProtocolDecl *proto) {
    auto kind = ((type->isParameterPack() ||
                  isa<PackArchetypeType>(type))
                 ? Kind::WitnessTablePack
                 : Kind::WitnessTable);
    return GenericRequirement(kind, type, proto);
  }

  static GenericRequirement forValue(CanType type) {
    return GenericRequirement(Kind::Value, type, nullptr);
  }

  static toolchain::Type *typeForKind(irgen::IRGenModule &IGM,
                                 GenericRequirement::Kind kind);

  toolchain::Type *getType(irgen::IRGenModule &IGM) const {
    return typeForKind(IGM, getKind());
  }

  void dump(toolchain::raw_ostream &out) const {
    switch (kind) {
    case Kind::Shape:
      out << "shape: " << type;
      break;
    case Kind::Metadata:
      out << "metadata: " << type;
      break;
    case Kind::WitnessTable:
      out << "witness_table: " << type << " : " << proto->getName();
      break;
    case Kind::MetadataPack:
      out << "metadata_pack: " << type;
      break;
    case Kind::WitnessTablePack:
      out << "witness_table_pack: " << type << " : " << proto->getName();
      break;
    case Kind::Value:
      out << "value: " << type;
      break;
    }
  }
};

} // end namespace language

namespace toolchain {
template <> struct DenseMapInfo<language::GenericRequirement> {
  using GenericRequirement = language::GenericRequirement;
  using CanTypeInfo = toolchain::DenseMapInfo<language::CanType>;
  static GenericRequirement getEmptyKey() {
    return GenericRequirement(GenericRequirement::Kind::Metadata,
                              CanTypeInfo::getEmptyKey(),
                              nullptr);
  }
  static GenericRequirement getTombstoneKey() {
    return GenericRequirement(GenericRequirement::Kind::Metadata,
                              CanTypeInfo::getTombstoneKey(),
                              nullptr);
  }
  static toolchain::hash_code getHashValue(GenericRequirement req) {
    return hash_combine(CanTypeInfo::getHashValue(req.getTypeParameter()),
                        hash_value(req.getProtocol()));
  }
  static bool isEqual(GenericRequirement lhs, GenericRequirement rhs) {
    return (lhs.getKind() == rhs.getKind() &&
            lhs.getTypeParameter() == rhs.getTypeParameter() &&
            lhs.getProtocol() == rhs.getProtocol());
  }
};
} // end namespace toolchain

#endif // LANGUAGE_AST_GENERIC_REQUIREMENT_H
