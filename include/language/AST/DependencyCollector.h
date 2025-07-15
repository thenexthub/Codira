//===--- DependencyCollector.h -------------------------------- -*- C++ -*-===//
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
// This file defines a class for recording incremental dependencies.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_DEPENDENCY_COLLECTOR_H
#define LANGUAGE_AST_DEPENDENCY_COLLECTOR_H

#include "language/AST/Identifier.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"

namespace language {

class DeclContext;

namespace evaluator {

class DependencyRecorder;

/// A \c DependencyCollector defines an abstract write-only buffer of
/// \c Reference objects. References are added to a collector during the write
/// phase of request evaluation (in \c writeDependencySink) with the various
/// \c add* functions below.
///
/// A \c DependencyCollector should not be created directly; instances are
/// vended by the request evaluator infrastructure itself.
struct DependencyCollector {
  friend DependencyRecorder;

  struct Reference {
  public:
    enum class Kind {
      Empty,
      Tombstone,
      UsedMember,
      PotentialMember,
      TopLevel,
      Dynamic,
    } kind;

    DeclContext *subject;
    DeclBaseName name;

  private:
    Reference(Kind kind, DeclContext *subject, DeclBaseName name)
        : kind(kind), subject(subject), name(name) {}

  public:
    static Reference empty() {
      return {Kind::Empty, toolchain::DenseMapInfo<DeclContext *>::getEmptyKey(),
              toolchain::DenseMapInfo<DeclBaseName>::getEmptyKey()};
    }

    static Reference tombstone() {
      return {Kind::Tombstone,
              toolchain::DenseMapInfo<DeclContext *>::getTombstoneKey(),
              toolchain::DenseMapInfo<DeclBaseName>::getTombstoneKey()};
    }

  public:
    static Reference usedMember(DeclContext *subject, DeclBaseName name) {
      return {Kind::UsedMember, subject, name};
    }

    static Reference potentialMember(DeclContext *subject) {
      return {Kind::PotentialMember, subject, DeclBaseName()};
    }

    static Reference topLevel(DeclBaseName name) {
      return {Kind::TopLevel, nullptr, name};
    }

    static Reference dynamic(DeclBaseName name) {
      return {Kind::Dynamic, nullptr, name};
    }

  public:
    struct Info {
      static inline Reference getEmptyKey() { return Reference::empty(); }
      static inline Reference getTombstoneKey() {
        return Reference::tombstone();
      }
      static inline unsigned getHashValue(const Reference &Val) {
        return toolchain::hash_combine(Val.kind, Val.subject,
                                  Val.name.getAsOpaquePointer());
      }
      static bool isEqual(const Reference &LHS, const Reference &RHS) {
        return LHS.kind == RHS.kind && LHS.subject == RHS.subject &&
               LHS.name == RHS.name;
      }
    };
  };

private:
  DependencyRecorder &parent;

public:
  explicit DependencyCollector(DependencyRecorder &parent);
  ~DependencyCollector();

public:
  /// Registers a named reference from the current dependency scope to a member
  /// defined in the given \p subject type.
  ///
  /// Used member constraints are typically the by-product of direct lookups,
  /// where the name being looked up and the target of the lookup are known
  /// up front. A used member dependency causes the file to be rebuilt if the
  /// definition of that member changes in any way - via
  /// deletion, addition, or mutation of a member with that same name.
  void addUsedMember(DeclContext *subject, DeclBaseName name);
  /// Registers a reference from the current dependency scope to a
  /// "potential member" of the given \p subject type.
  ///
  /// A single potential member dependency can be thought of as many used member
  /// dependencies - one for each current member of the subject type, but also
  /// one for every member that will be added or removed from the type in the
  /// future. As such, these dependencies cause rebuilds when any members are
  /// added, removed, or changed in the \p subject type. It also indicates a
  /// dependency on the \p subject type's existence, so deleting the \p subject
  /// type will also cause a rebuild.
  ///
  /// These dependencies are most appropriate for protocol conformances,
  /// superclass constraints, and other requirements involving entire types.
  void addPotentialMember(DeclContext *subject);
  /// Registers a reference from the current dependency scope to a given
  /// top-level \p name.
  ///
  /// A top level dependency causes a rebuild when another top-level entity with
  /// that name is added, removed, or modified.
  void addTopLevelName(DeclBaseName name);
  /// Registers a reference from the current dependency scope to a given
  /// dynamic member \p name.
  ///
  /// A dynamic lookup dependency is a special kind of member dependency on
  /// a name that is found by \c AnyObject lookup.
  void addDynamicLookupName(DeclBaseName name);

public:
  /// Retrieves the dependency recorder that created this dependency collector.
  const DependencyRecorder &getRecorder() const { return parent; }
};

} // end namespace evaluator

} // end namespace language

#endif // LANGUAGE_AST_DEPENDENCY_COLLECTOR_H
