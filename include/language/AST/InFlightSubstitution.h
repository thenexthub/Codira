//===--- InFlightSubstitution.h - In-flight substitution data ---*- C++ -*-===//
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
// This file defines the InFlightSubstitution structure, which captures
// all the information about a type substitution that's currently in
// progress.  For now, this is meant to be an internal implementation
// detail of the substitution system, and other systems should not use
// it (unless they are part of the extended substitution system, such as
// the SIL type substituter)
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_INFLIGHTSUBSTITUTION_H
#define LANGUAGE_AST_INFLIGHTSUBSTITUTION_H

#include "language/AST/SubstitutionMap.h"

namespace language {
class SubstitutionMap;

class InFlightSubstitution {
  SubstOptions Options;
  TypeSubstitutionFn BaselineSubstType;
  LookupConformanceFn BaselineLookupConformance;
  RecursiveTypeProperties Props;

  struct ActivePackExpansion {
    bool isSubstExpansion = false;
    unsigned expansionIndex = 0;
  };
  SmallVector<ActivePackExpansion, 4> ActivePackExpansions;

public:
  InFlightSubstitution(TypeSubstitutionFn substType,
                       LookupConformanceFn lookupConformance,
                       SubstOptions options);

  InFlightSubstitution(const InFlightSubstitution &) = delete;
  InFlightSubstitution &operator=(const InFlightSubstitution &) = delete;

  /// Perform primitive substitution on the given type.  Returns Type()
  /// if the type should not be substituted as a whole.
  Type substType(SubstitutableType *origType, unsigned level);

  /// Perform primitive conformance lookup on the given type.
  ProtocolConformanceRef lookupConformance(Type dependentType,
                                           ProtocolDecl *conformedProtocol,
                                           unsigned level);

  /// Given the shape type of a pack expansion, invoke the given callback
  /// for each expanded component of it.  If the substituted component
  /// is an expansion component, the desired shape of that expansion
  /// is passed as the argument; otherwise, the argument is Type().
  /// In either case, an active expansion is entered on this IFS for
  /// the duration of the call to handleComponent, and subsequent
  /// pack-element type references will substitute to the corresponding
  /// element of the substitution of the pack.
  void expandPackExpansionShape(Type origShape,
      toolchain::function_ref<void(Type substComponentShape)> handleComponent);

  /// Call the given function for each expanded component type of the
  /// given pack expansion type.  The function will be invoked with the
  /// active expansion still active.
  void expandPackExpansionType(PackExpansionType *origExpansionType,
      toolchain::function_ref<void(Type substType)> handleComponentType) {
    expandPackExpansionShape(origExpansionType->getCountType(),
                             [&](Type substComponentShape) {
      auto origPatternType = origExpansionType->getPatternType();
      auto substEltType = origPatternType.subst(*this);

      auto substComponentType =
        (substComponentShape
           ? PackExpansionType::get(substEltType, substComponentShape)
           : substEltType);
      handleComponentType(substComponentType);
    });
  }

  /// Return a list of component types that the pack expansion expands to.
  SmallVector<Type, 8>
  expandPackExpansionType(PackExpansionType *origExpansionType) {
    SmallVector<Type, 8> substComponentTypes;
    expandPackExpansionType(origExpansionType, substComponentTypes);
    return substComponentTypes;
  }

  /// Expand the list of component types that the pack expansion expands
  /// to into the given array.
  void expandPackExpansionType(PackExpansionType *origExpansionType,
                               SmallVectorImpl<Type> &substComponentTypes) {
    expandPackExpansionType(origExpansionType, [&](Type substComponentType) {
      substComponentTypes.push_back(substComponentType);
    });
  }

  class OptionsAdjustmentScope {
    InFlightSubstitution &IFS;
    SubstOptions SavedOptions;

  public:
    OptionsAdjustmentScope(InFlightSubstitution &IFS, SubstOptions newOptions)
      : IFS(IFS), SavedOptions(IFS.Options) {
      IFS.Options = newOptions;
    }

    OptionsAdjustmentScope(const OptionsAdjustmentScope &) = delete;
    OptionsAdjustmentScope &operator=(const OptionsAdjustmentScope &) = delete;

    ~OptionsAdjustmentScope() {
      IFS.Options = SavedOptions;
    }
  };

  template <class Fn>
  auto withNewOptions(SubstOptions options, Fn &&fn)
      -> decltype(std::forward<Fn>(fn)()) {
    OptionsAdjustmentScope scope(*this, options);
    return std::forward<Fn>(fn)();
  }

  SubstOptions getOptions() const {
    return Options;
  }

  bool shouldSubstitutePrimaryArchetypes() const {
    return Options.contains(SubstFlags::SubstitutePrimaryArchetypes);
  }

  bool shouldSubstituteOpaqueArchetypes() const {
    return Options.contains(SubstFlags::SubstituteOpaqueArchetypes);
  }

  bool shouldSubstituteLocalArchetypes() const {
    return Options.contains(SubstFlags::SubstituteLocalArchetypes);
  }

  /// Is the given type invariant to substitution?
  bool isInvariant(Type type) const;
};

/// A helper classes that provides stable storage for the query
/// functions against a SubstitutionMap.
struct InFlightSubstitutionViaSubMapHelper {
  QuerySubstitutionMap QueryType;
  LookUpConformanceInSubstitutionMap QueryConformance;

  InFlightSubstitutionViaSubMapHelper(SubstitutionMap subMap)
    : QueryType{subMap}, QueryConformance(subMap) {}
};
class InFlightSubstitutionViaSubMap :
  private InFlightSubstitutionViaSubMapHelper,
  public InFlightSubstitution {

public:
  InFlightSubstitutionViaSubMap(SubstitutionMap subMap,
                                SubstOptions options)
    : InFlightSubstitutionViaSubMapHelper(subMap),
      InFlightSubstitution(QueryType, QueryConformance, options) {}
};

} // end namespace language

#endif
