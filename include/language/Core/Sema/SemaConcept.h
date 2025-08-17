//===-- SemaConcept.h - Semantic Analysis for Constraints and Concepts ----===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
//  This file provides semantic analysis for C++ constraints and concepts.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMACONCEPT_H
#define LANGUAGE_CORE_SEMA_SEMACONCEPT_H
#include "language/Core/AST/ASTConcept.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclTemplate.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/STLFunctionalExtras.h"
#include "toolchain/ADT/SmallVector.h"
#include <optional>
#include <utility>

namespace language::Core {
class Sema;

enum { ConstraintAlignment = 8 };

struct alignas(ConstraintAlignment) AtomicConstraint {
  const Expr *ConstraintExpr;
  const NamedDecl *ConstraintDecl;
  std::optional<ArrayRef<TemplateArgumentLoc>> ParameterMapping;

  AtomicConstraint(const Expr *ConstraintExpr, const NamedDecl *ConstraintDecl)
      : ConstraintExpr(ConstraintExpr), ConstraintDecl(ConstraintDecl) {};

  bool hasMatchingParameterMapping(ASTContext &C,
                                   const AtomicConstraint &Other) const {
    if (!ParameterMapping != !Other.ParameterMapping)
      return false;
    if (!ParameterMapping)
      return true;
    if (ParameterMapping->size() != Other.ParameterMapping->size())
      return false;

    for (unsigned I = 0, S = ParameterMapping->size(); I < S; ++I) {
      toolchain::FoldingSetNodeID IDA, IDB;
      C.getCanonicalTemplateArgument((*ParameterMapping)[I].getArgument())
          .Profile(IDA, C);
      C.getCanonicalTemplateArgument((*Other.ParameterMapping)[I].getArgument())
          .Profile(IDB, C);
      if (IDA != IDB)
        return false;
    }
    return true;
  }
};

struct alignas(ConstraintAlignment) NormalizedConstraintPair;
struct alignas(ConstraintAlignment) FoldExpandedConstraint;

/// \brief A normalized constraint, as defined in C++ [temp.constr.normal], is
/// either an atomic constraint, a conjunction of normalized constraints or a
/// disjunction of normalized constraints.
struct NormalizedConstraint {
  friend class Sema;

  enum CompoundConstraintKind { CCK_Conjunction, CCK_Disjunction };

  using CompoundConstraint = toolchain::PointerIntPair<NormalizedConstraintPair *, 1,
                                                  CompoundConstraintKind>;

  toolchain::PointerUnion<AtomicConstraint *, FoldExpandedConstraint *,
                     CompoundConstraint>
      Constraint;

  NormalizedConstraint(AtomicConstraint *C): Constraint{C} { };
  NormalizedConstraint(FoldExpandedConstraint *C) : Constraint{C} {};

  NormalizedConstraint(ASTContext &C, NormalizedConstraint LHS,
                       NormalizedConstraint RHS, CompoundConstraintKind Kind);

  NormalizedConstraint(ASTContext &C, const NormalizedConstraint &Other);
  NormalizedConstraint(NormalizedConstraint &&Other):
      Constraint(Other.Constraint) {
    Other.Constraint = nullptr;
  }
  NormalizedConstraint &operator=(const NormalizedConstraint &Other) = delete;
  NormalizedConstraint &operator=(NormalizedConstraint &&Other) {
    if (&Other != this) {
      NormalizedConstraint Temp(std::move(Other));
      std::swap(Constraint, Temp.Constraint);
    }
    return *this;
  }

  bool isAtomic() const { return toolchain::isa<AtomicConstraint *>(Constraint); }
  bool isFoldExpanded() const {
    return toolchain::isa<FoldExpandedConstraint *>(Constraint);
  }
  bool isCompound() const { return toolchain::isa<CompoundConstraint>(Constraint); }

  CompoundConstraintKind getCompoundKind() const;

  NormalizedConstraint &getLHS() const;
  NormalizedConstraint &getRHS() const;

  AtomicConstraint *getAtomicConstraint() const;

  FoldExpandedConstraint *getFoldExpandedConstraint() const;

private:
  static std::optional<NormalizedConstraint>
  fromAssociatedConstraints(Sema &S, const NamedDecl *D,
                            ArrayRef<AssociatedConstraint> ACs);
  static std::optional<NormalizedConstraint>
  fromConstraintExpr(Sema &S, const NamedDecl *D, const Expr *E);
};

struct alignas(ConstraintAlignment) NormalizedConstraintPair {
  NormalizedConstraint LHS, RHS;
};

struct alignas(ConstraintAlignment) FoldExpandedConstraint {
  enum class FoldOperatorKind { And, Or } Kind;
  NormalizedConstraint Constraint;
  const Expr *Pattern;

  FoldExpandedConstraint(FoldOperatorKind K, NormalizedConstraint C,
                         const Expr *Pattern)
      : Kind(K), Constraint(std::move(C)), Pattern(Pattern) {};

  static bool AreCompatibleForSubsumption(const FoldExpandedConstraint &A,
                                          const FoldExpandedConstraint &B);
};

const NormalizedConstraint *getNormalizedAssociatedConstraints(
    Sema &S, const NamedDecl *ConstrainedDecl,
    ArrayRef<AssociatedConstraint> AssociatedConstraints);

/// \brief SubsumptionChecker establishes subsumption
/// between two set of constraints.
class SubsumptionChecker {
public:
  using SubsumptionCallable = toolchain::function_ref<bool(
      const AtomicConstraint &, const AtomicConstraint &)>;

  SubsumptionChecker(Sema &SemaRef, SubsumptionCallable Callable = {});

  std::optional<bool> Subsumes(const NamedDecl *DP,
                               ArrayRef<AssociatedConstraint> P,
                               const NamedDecl *DQ,
                               ArrayRef<AssociatedConstraint> Q);

  bool Subsumes(const NormalizedConstraint *P, const NormalizedConstraint *Q);

private:
  Sema &SemaRef;
  SubsumptionCallable Callable;

  // Each Literal has a unique value that is enough to establish
  // its identity.
  // Some constraints (fold expended) require special subsumption
  // handling logic beyond comparing values, so we store a flag
  // to let us quickly dispatch to each kind of variable.
  struct Literal {
    enum Kind { Atomic, FoldExpanded };

    unsigned Value : 16;
    LLVM_PREFERRED_TYPE(Kind)
    unsigned Kind : 1;

    bool operator==(const Literal &Other) const { return Value == Other.Value; }
    bool operator<(const Literal &Other) const { return Value < Other.Value; }
  };
  using Clause = toolchain::SmallVector<Literal>;
  using Formula = toolchain::SmallVector<Clause, 5>;

  struct CNFFormula : Formula {
    static constexpr auto Kind = NormalizedConstraint::CCK_Conjunction;
    using Formula::Formula;
  };
  struct DNFFormula : Formula {
    static constexpr auto Kind = NormalizedConstraint::CCK_Disjunction;
    using Formula::Formula;
  };

  struct MappedAtomicConstraint {
    AtomicConstraint *Constraint;
    Literal ID;
  };

  struct FoldExpendedConstraintKey {
    FoldExpandedConstraint::FoldOperatorKind Kind;
    AtomicConstraint *Constraint;
    Literal ID;
  };

  toolchain::DenseMap<const Expr *, toolchain::SmallDenseMap<toolchain::FoldingSetNodeID,
                                                   MappedAtomicConstraint>>
      AtomicMap;

  toolchain::DenseMap<const Expr *, std::vector<FoldExpendedConstraintKey>> FoldMap;

  // A map from a literal to a corresponding associated constraint.
  // We do not have enough bits left for a pointer union here :(
  toolchain::DenseMap<uint16_t, void *> ReverseMap;

  // Fold expanded constraints ask us to recursively establish subsumption.
  // This caches the result.
  toolchain::SmallDenseMap<
      std::pair<const FoldExpandedConstraint *, const FoldExpandedConstraint *>,
      bool>
      FoldSubsumptionCache;

  // Each <atomic, fold expanded constraint> is represented as a single ID.
  // This is intentionally kept small we can't handle a large number of
  // constraints anyway.
  uint16_t NextID;

  bool Subsumes(const DNFFormula &P, const CNFFormula &Q);
  bool Subsumes(Literal A, Literal B);
  bool Subsumes(const FoldExpandedConstraint *A,
                const FoldExpandedConstraint *B);
  bool DNFSubsumes(const Clause &P, const Clause &Q);

  CNFFormula CNF(const NormalizedConstraint &C);
  DNFFormula DNF(const NormalizedConstraint &C);

  template <typename FormulaType>
  FormulaType Normalize(const NormalizedConstraint &C);
  void AddUniqueClauseToFormula(Formula &F, Clause C);

  Literal find(AtomicConstraint *);
  Literal find(FoldExpandedConstraint *);

  uint16_t getNewLiteralId();
};

} // clang

#endif // LANGUAGE_CORE_SEMA_SEMACONCEPT_H
