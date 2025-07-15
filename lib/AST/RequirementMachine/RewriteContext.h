//===--- RewriteContext.h - Term rewriting allocation arena -----*- C++ -*-===//
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

#ifndef LANGUAGE_REWRITECONTEXT_H
#define LANGUAGE_REWRITECONTEXT_H

#include "language/AST/ASTContext.h"
#include "language/AST/Types.h"
#include "language/Basic/Statistic.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/Support/Allocator.h"
#include "Debug.h"
#include "Histogram.h"
#include "Symbol.h"
#include "Term.h"

namespace language {

namespace rewriting {

class RequirementMachine;

/// A global object that can be shared by multiple rewrite systems.
///
/// It stores uniqued symbols and terms.
///
/// Out-of-line methods are documented in RewriteContext.cpp.
class RewriteContext final {
  friend class Symbol;
  friend class Term;

  /// Allocator for uniquing symbols and terms.
  toolchain::BumpPtrAllocator Allocator;

  /// Folding set for uniquing symbols.
  toolchain::FoldingSet<Symbol::Storage> Symbols;

  /// The singleton storage for shape symbols.
  Symbol::Storage *TheShapeSymbol;

  /// The singleton storage for pack element symbols.
  Symbol::Storage *ThePackElementSymbol;

  /// Folding set for uniquing terms.
  toolchain::FoldingSet<Term::Storage> Terms;

  /// Requirement machines built from generic signatures.
  toolchain::DenseMap<GenericSignature, RequirementMachine *> Machines;

  /// Stores information about a vertex in the protocol dependency graph.
  struct ProtocolNode {
    /// The 'index' value for Tarjan's algorithm.
    unsigned Index;

    /// The 'low link' value for Tarjan's algorithm.
    unsigned LowLink : 31;

    /// The 'on stack' flag for Tarjan's algorithm.
    unsigned OnStack : 1;

    /// The connected component index, which keys the 'Components' DenseMap
    /// below.
    unsigned ComponentID;

    ProtocolNode() {
      Index = 0;
      LowLink = 0;
      OnStack = 0;
      ComponentID = 0;
    }
  };

  /// A strongly-connected component in the protocol dependency graph.
  struct ProtocolComponent {
    /// The members of this connected component.
    ArrayRef<const ProtocolDecl *> Protos;

    /// Whether we have started computing the requirement signatures of
    /// the protocols in this component.
    bool ComputingRequirementSignatures = false;

    /// Whether we have finished computing the requirement signatures of
    /// the protocols in this component.
    bool ComputedRequirementSignatures = false;

    /// Each connected component has a lazily-created requirement machine
    /// built from the requirement signatures of the protocols in this
    /// component.
    RequirementMachine *Machine = nullptr;
  };

  /// We pre-load protocol dependencies here to avoid re-entrancy.
  toolchain::DenseMap<const ProtocolDecl *, ArrayRef<ProtocolDecl *>> Dependencies;

  /// Maps protocols to their connected components.
  toolchain::DenseMap<const ProtocolDecl *, ProtocolNode> Protos;

  /// Used by Tarjan's algorithm.
  unsigned NextComponentIndex = 0;

  /// Prevents re-entrant calls into getProtocolComponentRec().
  bool ProtectProtocolComponentRec = false;

  /// The connected components. Keys are the ComponentID fields of
  /// ProtocolNode.
  toolchain::DenseMap<unsigned, ProtocolComponent> Components;

  /// The stack of timers for performance analysis. See beginTimer() and
  /// endTimer().
  toolchain::SmallVector<uint64_t, 2> Timers;

  ASTContext &Context;

  DebugOptions Debug;

  RewriteContext(const RewriteContext &) = delete;
  RewriteContext(RewriteContext &&) = delete;
  RewriteContext &operator=(const RewriteContext &) = delete;
  RewriteContext &operator=(RewriteContext &&) = delete;

  void getProtocolComponentRec(const ProtocolDecl *proto,
                               SmallVectorImpl<const ProtocolDecl *> &stack);
  ProtocolComponent &getProtocolComponentImpl(const ProtocolDecl *proto);

public:
  /// Statistics.
  UnifiedStatsReporter *Stats;

  /// Histograms.
  Histogram SymbolHistogram;
  Histogram TermHistogram;
  Histogram RuleTrieHistogram;
  Histogram RuleTrieRootHistogram;
  Histogram PropertyTrieHistogram;
  Histogram PropertyTrieRootHistogram;
  Histogram ConformanceRulesHistogram;
  Histogram MinimalConformancesHistogram;

  explicit RewriteContext(ASTContext &ctx);

  DebugOptions getDebugOptions() const { return Debug; }

  ASTContext &getASTContext() const { return Context; }

  void beginTimer(StringRef name);

  void endTimer(StringRef name);

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Reduction order on protocols.
  ///
  //////////////////////////////////////////////////////////////////////////////

  int compareProtocols(const ProtocolDecl *lhs,
                       const ProtocolDecl *rhs);

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Type to term conversion. The opposite direction is implemented in
  /// PropertyMap because it depends on the current rewrite system.
  ///
  //////////////////////////////////////////////////////////////////////////////

  static unsigned getGenericParamIndex(Type type);

  MutableTerm getMutableTermForType(CanType paramType,
                                    const ProtocolDecl *proto);

  MutableTerm getRelativeTermForType(CanType typeWitness,
                                     ArrayRef<Term> substitutions);

  CanType getSubstitutionSchemaFromType(CanType concreteType,
                                        const ProtocolDecl *proto,
                                        SmallVectorImpl<Term> &result);

  CanType getRelativeSubstitutionSchemaFromType(CanType concreteType,
                                                ArrayRef<Term> substitutions,
                                                SmallVectorImpl<Term> &result);

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Construction of requirement machines for connected components in the
  /// protocol dependency graph.
  ///
  //////////////////////////////////////////////////////////////////////////////

  RequirementMachine *getRequirementMachine(CanGenericSignature sig);
  bool isRecursivelyConstructingRequirementMachine(CanGenericSignature sig);

  void installRequirementMachine(CanGenericSignature sig,
                                 std::unique_ptr<RequirementMachine> machine);

  ArrayRef<const ProtocolDecl *>
  startComputingRequirementSignatures(const ProtocolDecl *proto);

  void finishComputingRequirementSignatures(const ProtocolDecl *proto);

  RequirementMachine *getRequirementMachine(const ProtocolDecl *proto);
  bool isRecursivelyConstructingRequirementMachine(const ProtocolDecl *proto);

  void installRequirementMachine(const ProtocolDecl *proto,
                                 std::unique_ptr<RequirementMachine> machine);

  ~RewriteContext();
};

} // end namespace rewriting

} // end namespace language

#endif
