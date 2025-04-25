//===--- RequirementSignature.h - Requirement Signature AST -----*- C++ -*-===//
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
// This file defines the RequirementSignature class.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_REQUIREMENT_SIGNATURE_H
#define SWIFT_AST_REQUIREMENT_SIGNATURE_H

#include "language/AST/GenericSignature.h"
#include "language/AST/Type.h"

namespace language {

/// A description of a typealias defined in a protocol.
class ProtocolTypeAlias final {
  Identifier Name;
  Type UnderlyingType;

public:
  ProtocolTypeAlias(Identifier name, Type underlyingType)
    : Name(name), UnderlyingType(underlyingType) {}

  /// Returns the name of the typealias.
  Identifier getName() const { return Name; }

  /// Returns the underlying type of the typealias.
  Type getUnderlyingType() const { return UnderlyingType; }
};

/// The requirements that describe a protocol from the viewpoint of the
/// generics system.
class RequirementSignature final {
  ArrayRef<Requirement> Requirements;
  ArrayRef<ProtocolTypeAlias> TypeAliases;
  GenericSignatureErrors Errors;

public:
  RequirementSignature(GenericSignatureErrors errors = GenericSignatureErrors())
    : Errors(errors) {}

  RequirementSignature(ArrayRef<Requirement> requirements,
                       ArrayRef<ProtocolTypeAlias> typeAliases,
                       GenericSignatureErrors errors = GenericSignatureErrors())
    : Requirements(requirements), TypeAliases(typeAliases), Errors(errors) {}

  /// The requirements including any inherited protocols and conformances for
  /// associated types that are introduced in this protocol.
  ///
  /// Requirements implied via any other protocol (e.g., inherited protocols
  /// of the inherited protocols) are not mentioned.
  ///
  /// The conformance requirements listed here become entries in witness tables
  /// for conformances to this protocol.
  ArrayRef<Requirement> getRequirements() const {
    return Requirements;
  }

  ArrayRef<ProtocolTypeAlias> getTypeAliases() const {
    return TypeAliases;
  }

  GenericSignatureErrors getErrors() const {
    return Errors;
  }

  void getRequirementsWithInverses(
      ProtocolDecl *owner,
      SmallVector<Requirement, 2> &reqs,
      SmallVector<InverseRequirement, 2> &inverses) const;

  void print(ProtocolDecl *owner, raw_ostream &OS,
             const PrintOptions &Options = PrintOptions()) const;
  void print(ProtocolDecl *owner, ASTPrinter &Printer,
             const PrintOptions &Opts = PrintOptions()) const;

  static RequirementSignature getPlaceholderRequirementSignature(
      const ProtocolDecl *proto, GenericSignatureErrors errors);
};

} // end namespace language

#endif // SWIFT_AST_REQUIREMENT_SIGNATURE_H
