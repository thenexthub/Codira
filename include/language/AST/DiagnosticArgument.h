//===--- DiagnosticArgument.h -----------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_DIAGNOSTIC_ARGUMENT_H
#define LANGUAGE_AST_DIAGNOSTIC_ARGUMENT_H

#include "language/AST/ActorIsolation.h"
#include "language/AST/AvailabilityDomain.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/Identifier.h"
#include "language/AST/LayoutConstraint.h"
#include "language/AST/TypeLoc.h"
#include "language/Basic/Version.h"
#include "toolchain/Support/VersionTuple.h"

namespace clang {
class NamedDecl;
class Type;
} // namespace clang

namespace language {

class Decl;
class DeclAttribute;
class TypeAttribute;
class TypeRepr;

enum class DescriptivePatternKind : uint8_t;
enum class DescriptiveDeclKind : uint8_t;
enum class ReferenceOwnership : uint8_t;
enum class SelfAccessKind : uint8_t;
enum class StaticSpellingKind : uint8_t;
enum class StmtKind;

/// A family of wrapper types for compiler data types that forces its
/// underlying data to be formatted with full qualification.
///
/// So far, this is only useful for \c Type, hence the SFINAE'ing.
template <typename T, typename = void>
struct FullyQualified {};

template <typename T>
struct FullyQualified<
    T, typename std::enable_if<std::is_convertible<T, Type>::value>::type> {
  Type t;

public:
  FullyQualified(T t) : t(t) {};

  Type getType() const { return t; }
};

struct WitnessType {
  Type t;

  WitnessType(Type t) : t(t) {}

  Type getType() { return t; }
};

/// Describes the kind of diagnostic argument we're storing.
enum class DiagnosticArgumentKind {
  String,
  Integer,
  Unsigned,
  Identifier,
  ObjCSelector,
  Decl,
  Type,
  TypeRepr,
  FullyQualifiedType,
  WitnessType,
  DescriptivePatternKind,
  SelfAccessKind,
  ReferenceOwnership,
  StaticSpellingKind,
  DescriptiveDeclKind,
  DescriptiveStmtKind,
  DeclAttribute,
  TypeAttribute,
  AvailabilityDomain,
  AvailabilityRange,
  VersionTuple,
  LayoutConstraint,
  ActorIsolation,
  IsolationSource,
  Diagnostic,
  ClangDecl,
  ClangType,
};

/// Variant type that holds a single diagnostic argument of a known
/// type.
///
/// All diagnostic arguments are converted to an instance of this class.
class DiagnosticArgument {
  DiagnosticArgumentKind Kind;
  union {
    int IntegerVal;
    unsigned UnsignedVal;
    StringRef StringVal;
    DeclNameRef IdentifierVal;
    ObjCSelector ObjCSelectorVal;
    const Decl *TheDecl;
    Type TypeVal;
    TypeRepr *TyR;
    FullyQualified<Type> FullyQualifiedTypeVal;
    WitnessType WitnessTypeVal;
    DescriptivePatternKind DescriptivePatternKindVal;
    SelfAccessKind SelfAccessKindVal;
    ReferenceOwnership ReferenceOwnershipVal;
    StaticSpellingKind StaticSpellingKindVal;
    DescriptiveDeclKind DescriptiveDeclKindVal;
    StmtKind DescriptiveStmtKindVal;
    const DeclAttribute *DeclAttributeVal;
    const TypeAttribute *TypeAttributeVal;
    AvailabilityDomain AvailabilityDomainVal;
    AvailabilityRange AvailabilityRangeVal;
    toolchain::VersionTuple VersionVal;
    LayoutConstraint LayoutConstraintVal;
    ActorIsolation ActorIsolationVal;
    IsolationSource IsolationSourceVal;
    DiagnosticInfo *DiagnosticVal;
    const clang::NamedDecl *ClangDecl;
    const clang::Type *ClangType;
  };

public:
  DiagnosticArgument(StringRef S);
  DiagnosticArgument(int I);
  DiagnosticArgument(unsigned I);
  DiagnosticArgument(DeclNameRef R);
  DiagnosticArgument(DeclName D);
  DiagnosticArgument(DeclBaseName D);
  DiagnosticArgument(Identifier I);
  DiagnosticArgument(ObjCSelector S);
  DiagnosticArgument(const Decl *VD);
  DiagnosticArgument(Type T);
  DiagnosticArgument(TypeRepr *T);
  DiagnosticArgument(FullyQualified<Type> FQT);
  DiagnosticArgument(WitnessType WT);
  DiagnosticArgument(const TypeLoc &TL);
  DiagnosticArgument(DescriptivePatternKind DPK);
  DiagnosticArgument(ReferenceOwnership RO);
  DiagnosticArgument(SelfAccessKind SAK);
  DiagnosticArgument(StaticSpellingKind SSK);
  DiagnosticArgument(DescriptiveDeclKind DDK);
  DiagnosticArgument(StmtKind SK);
  DiagnosticArgument(const DeclAttribute *attr);
  DiagnosticArgument(const TypeAttribute *attr);
  DiagnosticArgument(const AvailabilityDomain domain);
  DiagnosticArgument(const AvailabilityRange &range);
  DiagnosticArgument(toolchain::VersionTuple version);
  DiagnosticArgument(LayoutConstraint L);
  DiagnosticArgument(ActorIsolation AI);
  DiagnosticArgument(IsolationSource IS);
  DiagnosticArgument(DiagnosticInfo *D);
  DiagnosticArgument(const clang::NamedDecl *ND);
  DiagnosticArgument(const clang::Type *Ty);

  /// Initializes a diagnostic argument using the underlying type of the
  /// given enum.
  template <
      typename EnumType,
      typename std::enable_if<std::is_enum<EnumType>::value>::type * = nullptr>
  DiagnosticArgument(EnumType value)
      : DiagnosticArgument(
            static_cast<typename std::underlying_type<EnumType>::type>(value)) {
  }

  DiagnosticArgumentKind getKind() const;

  StringRef getAsString() const;
  int getAsInteger() const;
  unsigned getAsUnsigned() const;
  DeclNameRef getAsIdentifier() const;
  ObjCSelector getAsObjCSelector() const;
  const Decl *getAsDecl() const;
  Type getAsType() const;
  TypeRepr *getAsTypeRepr() const;
  FullyQualified<Type> getAsFullyQualifiedType() const;
  WitnessType getAsWitnessType() const;
  DescriptivePatternKind getAsDescriptivePatternKind() const;
  ReferenceOwnership getAsReferenceOwnership() const;
  SelfAccessKind getAsSelfAccessKind() const;
  StaticSpellingKind getAsStaticSpellingKind() const;
  DescriptiveDeclKind getAsDescriptiveDeclKind() const;
  StmtKind getAsDescriptiveStmtKind() const;
  const DeclAttribute *getAsDeclAttribute() const;
  const TypeAttribute *getAsTypeAttribute() const;
  const AvailabilityDomain getAsAvailabilityDomain() const;
  const AvailabilityRange getAsAvailabilityRange() const;
  toolchain::VersionTuple getAsVersionTuple() const;
  LayoutConstraint getAsLayoutConstraint() const;
  ActorIsolation getAsActorIsolation() const;
  IsolationSource getAsIsolationSource() const;
  DiagnosticInfo *getAsDiagnostic() const;
  const clang::NamedDecl *getAsClangDecl() const;
  const clang::Type *getAsClangType() const;
};

} // namespace language

#endif /* LANGUAGE_AST_DIAGNOSTIC_ARGUMENT_H */
