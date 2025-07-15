//===--- PreparedOverload.h - A Choice from an Overload Set  ----*- C++ -*-===//
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
#ifndef LANGUAGE_SEMA_PREPAREDOVERLOAD_H
#define LANGUAGE_SEMA_PREPAREDOVERLOAD_H

#include "language/AST/PropertyWrappers.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/TrailingObjects.h"

namespace language {

class ExistentialArchetypeType;
class GenericTypeParamType;
class TypeVariableType;

namespace constraints {

class ConstraintLocatorBuilder;
class ConstraintSystem;

/// Describes the type produced when referencing a declaration.
struct DeclReferenceType {
  /// The "opened" type, which is the type of the declaration where any
  /// generic parameters have been replaced with type variables.
  ///
  /// The mapping from generic parameters to type variables will have been
  /// recorded by \c recordOpenedTypes when this type is produced.
  Type openedType;

  /// The opened type, after performing contextual type adjustments such as
  /// removing concurrency-related annotations for a `@preconcurrency`
  /// operation.
  Type adjustedOpenedType;

  /// The type of the reference, based on the original opened type. This is the
  /// type that the expression used to form the declaration reference would
  /// have if no adjustments had been applied.
  Type referenceType;

  /// The type of the reference, which is the adjusted opened type after
  /// (e.g.) applying the base of a member access. This is the type of the
  /// expression used to form the declaration reference.
  Type adjustedReferenceType;

  /// The type that could be thrown by accessing this declaration.
  Type thrownErrorTypeOnAccess;
};

/// Describes a dependent type that has been opened to a particular type
/// variable.
using OpenedType = std::pair<GenericTypeParamType *, TypeVariableType *>;

/// A change to be introduced into the constraint system when this
/// overload choice is chosen.
struct PreparedOverloadChange {
  enum ChangeKind : unsigned {
    /// A generic parameter was opened to a type variable.
    AddedTypeVariable,

    /// A generic requirement was opened to a constraint.
    AddedConstraint,

    /// Special case of a Bind constraint.
    AddedBindConstraint,

    /// A mapping of generic parameter types to type variables
    /// was recorded.
    OpenedTypes,

    /// An existential type was opened.
    OpenedExistentialType,

    /// A pack expansion type was opened.
    OpenedPackExpansionType,

    /// A property wrapper was applied to a parameter.
    AppliedPropertyWrapper,

    /// A fix was recorded because a property wrapper application failed.
    AddedFix
  };

  /// The kind of change.
  ChangeKind Kind;

  union {
    /// For ChangeKind::AddedTypeVariable.
    TypeVariableType *TypeVar;

    /// For ChangeKind::AddedConstraint.
    Constraint *TheConstraint;

    struct {
      TypeBase *FirstType;
      TypeBase * SecondType;
    } Bind;

    /// For ChangeKind::OpenedTypes.
    struct {
      const OpenedType *Data;
      size_t Count;
    } Replacements;

    /// For ChangeKind::OpenedExistentialType.
    ExistentialArchetypeType *TheExistential;

    /// For ChangeKind::OpenedPackExpansionType.
    struct {
      PackExpansionType *TheExpansion;
      TypeVariableType *TypeVar;
    } PackExpansion;

    /// For ChangeKind::AppliedPropertyWrapper.
    struct {
      TypeBase *WrapperType;
      PropertyWrapperInitKind InitKind;
    } PropertyWrapper;

    /// For ChangeKind::Fix.
    struct {
      ConstraintFix *TheFix;
      unsigned Impact;
    } Fix;
  };
};

/// A "pre-cooked" representation of all type variables and constraints
/// that are generated as part of an overload choice.
class PreparedOverload final :
  public toolchain::TrailingObjects<PreparedOverload, PreparedOverloadChange> {

public:
  using Change = PreparedOverloadChange;

private:
  size_t Count;
  DeclReferenceType DeclType;

  size_t numTrailingObjects(OverloadToken<Change>) const {
    return Count;
  }

public:
  PreparedOverload(const DeclReferenceType &declType, ArrayRef<Change> changes)
    : Count(changes.size()), DeclType(declType) {
    std::uninitialized_copy(changes.begin(), changes.end(),
                            getTrailingObjects<Change>());
  }

  Type getOpenedType() const {
    return DeclType.openedType;
  }

  Type getAdjustedOpenedType() const {
    return DeclType.adjustedOpenedType;
  }

  Type getReferenceType() const {
    return DeclType.referenceType;
  }

  Type getAdjustedReferenceType() const {
    return DeclType.adjustedReferenceType;
  }

  Type getThrownErrorTypeOnAccess() const {
    return DeclType.thrownErrorTypeOnAccess;
  }

  ArrayRef<Change> getChanges() const {
    return ArrayRef<Change>(getTrailingObjects<Change>(), Count);
  }
};

struct PreparedOverloadBuilder {
  SmallVector<PreparedOverload::Change, 8> Changes;

  void addedTypeVariable(TypeVariableType *typeVar) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::AddedTypeVariable;
    change.TypeVar = typeVar;
    Changes.push_back(change);
  }

  void addedConstraint(Constraint *constraint) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::AddedConstraint;
    change.TheConstraint = constraint;
    Changes.push_back(change);
  }

  void addedBindConstraint(Type firstType, Type secondType) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::AddedBindConstraint;
    change.Bind.FirstType = firstType.getPointer();
    change.Bind.SecondType = secondType.getPointer();
    Changes.push_back(change);
  }

  void openedTypes(ArrayRef<OpenedType> replacements) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::OpenedTypes;
    change.Replacements.Data = replacements.data();
    change.Replacements.Count = replacements.size();
    Changes.push_back(change);
  }

  void openedExistentialType(ExistentialArchetypeType *openedExistential) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::OpenedExistentialType;
    change.TheExistential = openedExistential;
    Changes.push_back(change);
  }

  void openedPackExpansionType(PackExpansionType *packExpansion,
                               TypeVariableType *typeVar) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::OpenedPackExpansionType;
    change.PackExpansion.TheExpansion = packExpansion;
    change.PackExpansion.TypeVar = typeVar;
    Changes.push_back(change);
  }

  void appliedPropertyWrapper(AppliedPropertyWrapper wrapper) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::AppliedPropertyWrapper;
    change.PropertyWrapper.WrapperType = wrapper.wrapperType.getPointer();
    change.PropertyWrapper.InitKind = wrapper.initKind;
    Changes.push_back(change);
  }

  void addedFix(ConstraintFix *fix, unsigned impact) {
    PreparedOverload::Change change;
    change.Kind = PreparedOverload::Change::AddedFix;
    change.Fix.TheFix = fix;
    change.Fix.Impact = impact;
    Changes.push_back(change);
  }
};

}  // end namespace constraints
}  // end namespace language

#endif
