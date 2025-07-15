//===--- Field.h - Abstract stored field ------------------------*- C++ -*-===//
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
//  This file provides an abstraction for some sort of stored field
//  in a type.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_FIELD_H
#define LANGUAGE_IRGEN_FIELD_H

#include "language/AST/Decl.h"

namespace language {
class SILType;

namespace irgen {
class IRGenModule;

/// Return the number of fields that will be visited by forEachField,
/// which determines a number of things in the ABI, including the length
/// of the field vector in the type metadata.
///
/// Generally this is the length of the stored properties, but 
/// root default actors have an implicit field for their default-actor
/// storage, and there may also be missing members.
///
/// Even if you're sure that you're in a case where only the stored
/// properties are needed, calling this (and forEachField) instead of
/// directly accessing the stored properties is good practice.
unsigned getNumFields(const NominalTypeDecl *type);

struct Field {
public:
  enum Kind: uintptr_t {
    Var,
    MissingMember,
    DefaultActorStorage,
    NonDefaultDistributedActorStorage,
    FirstArtificial = DefaultActorStorage
  };
  enum : uintptr_t { KindMask = 0x3 };
private:
  uintptr_t declOrKind;
public:
  Field(VarDecl *decl)
      : declOrKind(reinterpret_cast<uintptr_t>(decl) | Var) {
    assert(decl);
    assert(getKind() == Var);
  }
  Field(MissingMemberDecl *decl)
      : declOrKind(reinterpret_cast<uintptr_t>(decl) | MissingMember) {
    assert(decl);
    assert(getKind() == MissingMember);
  }
  Field(Kind kind) : declOrKind(kind) {
    assert(kind >= FirstArtificial);
  }

  Kind getKind() const {
    return Kind(declOrKind & KindMask);
  }
  VarDecl *getVarDecl() const {
    assert(getKind() == Var);
    return reinterpret_cast<VarDecl*>(declOrKind);
  }
  MissingMemberDecl *getMissingMemberDecl() const {
    assert(getKind() == MissingMember);
    return reinterpret_cast<MissingMemberDecl*>(declOrKind & ~KindMask);
  }

  /// Is this a concrete field?  When we're emitting a type, all the
  /// fields should be concrete; non-concrete fields occur only with
  /// imported declarations.
  bool isConcrete() const { return getKind() != MissingMember; }

  /// Return the type of this concrete field.
  SILType getType(IRGenModule &IGM, SILType baseType) const;

  /// Return the interface type of this concrete field.
  Type getInterfaceType(IRGenModule &IGM) const;

  /// Return the nam eof this concrete field.
  toolchain::StringRef getName() const;

  bool operator==(Field other) const { return declOrKind == other.declOrKind; }
  bool operator!=(Field other) const { return declOrKind != other.declOrKind; }
};

// Don't export private C++ fields that were imported as private Codira fields.
// The type of a private field might not have all the type witness operations
// that Codira requires, for instance, `std::unique_ptr<IncompleteType>` would
// not have a destructor.
bool isExportableField(Field field);

/// Iterate all the fields of the given struct or class type, including
/// any implicit fields that might be accounted for in
/// getFieldVectorLength.
void forEachField(IRGenModule &IGM, const NominalTypeDecl *typeDecl,
                  toolchain::function_ref<void(Field field)> fn);

unsigned countExportableFields(IRGenModule &IGM, const NominalTypeDecl *type);

} // end namespace irgen
} // end namespace language

#endif
