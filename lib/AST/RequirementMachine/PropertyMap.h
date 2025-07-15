//===--- PropertyMap.h - Properties of type parameter terms 0000-*- C++ -*-===//
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
// A description of this data structure and its purpose can be found in
// PropertyMap.cpp.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_PROPERTYMAP_H
#define LANGUAGE_PROPERTYMAP_H

#include "language/AST/LayoutConstraint.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include <algorithm>
#include <vector>

#include "Debug.h"
#include "RewriteContext.h"
#include "RewriteSystem.h"

namespace toolchain {
  class raw_ostream;
}

namespace language {

class ProtocolConformance;
class ProtocolDecl;
enum class RequirementKind : unsigned;

namespace rewriting {

class MutableTerm;
class Term;

/// Records superclass requirements at a given level in the class hierarchy.
struct SuperclassRequirement {
  /// The most specific superclass constraint (in type difference order) for
  /// this level in the class hierarchy.
  std::optional<Symbol> SuperclassType;

  /// Superclass rules that apply to this key.
  toolchain::SmallVector<std::pair<Symbol, unsigned>, 1> SuperclassRules;
};

/// Stores a convenient representation of all "property-like" rewrite rules of
/// the form T.[p] => T, where [p] is a property symbol, for some term 'T'.
class PropertyBag {
  friend class PropertyMap;

  /// The fully reduced term whose properties are recorded in this property bag.
  Term Key;

  /// All protocols this type conforms to.
  toolchain::TinyPtrVector<const ProtocolDecl *> ConformsTo;

  /// The corresponding protocol conformance rules.
  toolchain::SmallVector<unsigned, 1> ConformsToRules;

  /// The most specific layout constraint this type satisfies.
  LayoutConstraint Layout;

  /// The corresponding layout rule for the above.
  std::optional<unsigned> LayoutRule;

  /// The most specific superclass declaration for which this type has a
  /// superclass constraint.
  const ClassDecl *SuperclassDecl = nullptr;

  /// Used for unifying superclass rules at different levels in the class
  /// hierarchy. For each class declaration, stores a symbol and rule pair
  /// for the most specific substituted type.
  toolchain::SmallDenseMap<const ClassDecl *, SuperclassRequirement, 2> Superclasses;

  /// The most specific concrete type constraint this type satisfies.
  std::optional<Symbol> ConcreteType;

  /// Concrete type rules that apply to this key.
  toolchain::SmallVector<std::pair<Symbol, unsigned>, 1> ConcreteTypeRules;

  /// Cache of associated type declarations.
  toolchain::SmallDenseMap<Identifier, AssociatedTypeDecl *, 2> AssocTypes;

  explicit PropertyBag(Term key) : Key(key) {}

  void copyPropertiesFrom(const PropertyBag *next,
                          RewriteContext &ctx);

  PropertyBag(const PropertyBag &) = delete;
  PropertyBag(PropertyBag &&) = delete;
  PropertyBag &operator=(const PropertyBag &) = delete;
  PropertyBag &operator=(PropertyBag &&) = delete;

  const SuperclassRequirement &getSuperclassRequirement() const {
    ASSERT(SuperclassDecl != nullptr);
    auto found = Superclasses.find(SuperclassDecl);
    ASSERT(found != Superclasses.end());
    return found->second;
  }

  MutableTerm getPrefixAfterStrippingKey(const MutableTerm &lookupTerm) const;

public:
  Term getKey() const { return Key; }
  void dump(toolchain::raw_ostream &out) const;

  bool hasSuperclassBound() const {
    return SuperclassDecl != nullptr;
  }

  CanType getSuperclassBound() const {
    return getSuperclassRequirement().SuperclassType->getConcreteType();
  }

  Type getSuperclassBound(
      ArrayRef<GenericTypeParamType *> genericParams,
      const MutableTerm &lookupTerm,
      const PropertyMap &map) const;

  bool isConcreteType() const {
    return ConcreteType.has_value();
  }

  CanType getConcreteType() const {
    return ConcreteType->getConcreteType();
  }

  Type getConcreteType(
      ArrayRef<GenericTypeParamType *> genericParams,
      const MutableTerm &lookupTerm,
      const PropertyMap &map) const;

  LayoutConstraint getLayoutConstraint() const {
    return Layout;
  }

  ArrayRef<const ProtocolDecl *> getConformsTo() const {
    return ConformsTo;
  }

  AssociatedTypeDecl *getAssociatedType(Identifier name);

  Symbol concretelySimplifySubstitution(const MutableTerm &mutTerm,
                                        RewriteContext &ctx,
                                        RewritePath *path) const;

  void verify(const RewriteSystem &system) const;
};

/// Stores all rewrite rules of the form T.[p] => T, where [p] is a property
/// symbol, for all terms 'T'.
///
/// Out-of-line methods are documented in PropertyMap.cpp.
class PropertyMap {
  RewriteContext &Context;
  RewriteSystem &System;
  std::vector<PropertyBag *> Entries;
  Trie<PropertyBag *, MatchKind::Longest> Trie;

  // Building the property map introduces new induced rules, which
  // runs another round of Knuth-Bendix completion, which rebuilds the
  // property map again.
  //
  // To avoid wasted work from re-introducing the same induced rules,
  // we track the rules we've seen already on previous builds.

  /// Superclass requirements always imply a layout requirement, and
  /// concrete type requirements where the type is a class imply a
  /// superclass requirement.
  ///
  /// Keep track of such rules to avoid wasted work from recording the
  /// same rewrite loop more than once.
  toolchain::DenseSet<unsigned> CheckedRules;

  /// When a type parameter is subject to two requirements of the same
  /// kind, we have a pair of rewrite rules T.[p1] => T and T.[p2] => T.
  ///
  /// One of these rules might imply the other. Keep track of these pairs
  /// to avoid wasted work from recording the same rewrite loop more than
  /// once.
  toolchain::DenseSet<std::pair<unsigned, unsigned>> CheckedRulePairs;

  DebugOptions Debug;

  PropertyBag *getOrCreateProperties(Term key);

  PropertyMap(const PropertyMap &) = delete;
  PropertyMap(PropertyMap &&) = delete;
  PropertyMap &operator=(const PropertyMap &) = delete;
  PropertyMap &operator=(PropertyMap &&) = delete;

public:
  explicit PropertyMap(RewriteSystem &system)
      : Context(system.getRewriteContext()),
        System(system) {
    Debug = Context.getDebugOptions();
  }

  ~PropertyMap();

  PropertyBag *lookUpProperties(std::reverse_iterator<const Symbol *> begin,
                                std::reverse_iterator<const Symbol *> end) const;
  PropertyBag *lookUpProperties(const MutableTerm &key) const;

  void buildPropertyMap();

  void dump(toolchain::raw_ostream &out) const;

  /// Return the rewrite context used for allocating memory.
  RewriteContext &getRewriteContext() const { return Context; }

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Term to type conversion. The opposite direction is implemented in
  /// RewriteContext because it does not depend on the current rewrite system.
  ///
  //////////////////////////////////////////////////////////////////////////////

  Type getTypeForTerm(Term term,
                      ArrayRef<GenericTypeParamType *> genericParams) const;

  Type getTypeForTerm(const MutableTerm &term,
                      ArrayRef<GenericTypeParamType *> genericParams) const;

  Type getTypeFromSubstitutionSchema(
                      Type schema,
                      ArrayRef<Term> substitutions,
                      ArrayRef<GenericTypeParamType *> genericParams,
                      const MutableTerm &prefix) const;

private:
  void clear();

  bool checkRuleOnce(unsigned ruleID);
  bool checkRulePairOnce(unsigned firstRuleID, unsigned secondRuleID);

  void addProperty(Term key, Symbol property, unsigned ruleID);

  void addConformanceProperty(Term key, Symbol property, unsigned ruleID);
  void addLayoutProperty(Term key, Symbol property, unsigned ruleID);

  void unifyConcreteTypes(Term key,
                          Symbol lhsProperty, unsigned lhsRuleID,
                          Symbol rhsProperty, unsigned rhsRuleID);

  void unifyConcreteTypes(
      Term key, std::optional<Symbol> &bestProperty,
      toolchain::SmallVectorImpl<std::pair<Symbol, unsigned>> &existingRules,
      Symbol property, unsigned ruleID);

  void recordSuperclassRelation(Term key,
                                Symbol superclassType,
                                unsigned superclassRuleID,
                                const ClassDecl *otherClass);

  void addSuperclassProperty(Term key, Symbol property, unsigned ruleID);
  void addConcreteTypeProperty(Term key, Symbol property, unsigned ruleID);

  void checkConcreteTypeRequirements();

  void concretizeNestedTypesFromConcreteParents();

  void concretizeNestedTypesFromConcreteParent(
                   Term key, RequirementKind requirementKind,
                   unsigned concreteRuleID,
                   CanType concreteType,
                   ArrayRef<Term> substitutions,
                   ArrayRef<unsigned> conformsToRules,
                   ArrayRef<const ProtocolDecl *> conformsTo);

  void concretizeTypeWitnessInConformance(
                   Term key, RequirementKind requirementKind,
                   Symbol concreteConformanceSymbol,
                   ProtocolConformanceRef conformance,
                   AssociatedTypeDecl *assocType) const;

  void inferConditionalRequirements(
                   ProtocolConformance *concrete,
                   ArrayRef<Term> substitutions) const;

  MutableTerm computeConstraintTermForTypeWitness(
      Term key, RequirementKind requirementKind,
      CanType concreteType, CanType typeWitness,
      const MutableTerm &subjectType,
      ArrayRef<Term> substitutions,
      RewritePath &path) const;

  void recordConcreteConformanceRule(
    unsigned concreteRuleID,
    unsigned conformanceRuleID,
    RequirementKind requirementKind,
    Symbol concreteConformanceSymbol) const;

  void verify() const;
};

} // end namespace rewriting

} // end namespace language

#endif
