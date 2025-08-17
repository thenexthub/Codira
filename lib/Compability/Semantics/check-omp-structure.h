//===-- lib/Semantics/check-omp-structure.h ---------------------*- C++ -*-===//
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

// OpenMP structure validity check list
//    1. invalid clauses on directive
//    2. invalid repeated clauses on directive
//    3. TODO: invalid nesting of regions

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_OMP_STRUCTURE_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_OMP_STRUCTURE_H_

#include "check-directive-structure.h"
#include "language/Compability/Common/enum-set.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/openmp-directive-sets.h"
#include "language/Compability/Semantics/semantics.h"
#include "toolchain/Frontend/OpenMP/OMPConstants.h"

using OmpClauseSet =
    language::Compability::common::EnumSet<toolchain::omp::Clause, toolchain::omp::Clause_enumSize>;

#define GEN_FLANG_DIRECTIVE_CLAUSE_SETS
#include "toolchain/Frontend/OpenMP/OMP.inc"

namespace toolchain {
namespace omp {
static OmpClauseSet privateSet{
    Clause::OMPC_private, Clause::OMPC_firstprivate, Clause::OMPC_lastprivate};
static OmpClauseSet privateReductionSet{
    OmpClauseSet{Clause::OMPC_reduction} | privateSet};
// omp.td cannot differentiate allowed/not allowed clause list for few
// directives for fortran. nowait is not allowed on begin directive clause list
// for below list of directives. Directives with conflicting list of clauses are
// included in below list.
static const OmpDirectiveSet noWaitClauseNotAllowedSet{
    Directive::OMPD_do,
    Directive::OMPD_do_simd,
    Directive::OMPD_sections,
    Directive::OMPD_single,
    Directive::OMPD_workshare,
};
} // namespace omp
} // namespace toolchain

namespace language::Compability::semantics {
struct AnalyzedCondStmt;

// Mapping from 'Symbol' to 'Source' to keep track of the variables
// used in multiple clauses
using SymbolSourceMap = std::multimap<const Symbol *, parser::CharBlock>;
// Multimap to check the triple <current_dir, enclosing_dir, enclosing_clause>
using DirectivesClauseTriple = std::multimap<toolchain::omp::Directive,
    std::pair<toolchain::omp::Directive, const OmpClauseSet>>;

class OmpStructureChecker
    : public DirectiveStructureChecker<toolchain::omp::Directive, toolchain::omp::Clause,
          parser::OmpClause, toolchain::omp::Clause_enumSize> {
public:
  using Base = DirectiveStructureChecker<toolchain::omp::Directive,
      toolchain::omp::Clause, parser::OmpClause, toolchain::omp::Clause_enumSize>;

  OmpStructureChecker(SemanticsContext &context)
      : DirectiveStructureChecker(context,
#define GEN_FLANG_DIRECTIVE_CLAUSE_MAP
#include "toolchain/Frontend/OpenMP/OMP.inc"
        ) {
  }
  using toolchainOmpClause = const toolchain::omp::Clause;

  void Enter(const parser::OpenMPConstruct &);
  void Leave(const parser::OpenMPConstruct &);
  void Enter(const parser::OpenMPInteropConstruct &);
  void Leave(const parser::OpenMPInteropConstruct &);
  void Enter(const parser::OpenMPDeclarativeConstruct &);
  void Leave(const parser::OpenMPDeclarativeConstruct &);

  void Enter(const parser::OpenMPLoopConstruct &);
  void Leave(const parser::OpenMPLoopConstruct &);
  void Enter(const parser::OmpEndLoopDirective &);
  void Leave(const parser::OmpEndLoopDirective &);

  void Enter(const parser::OpenMPAssumeConstruct &);
  void Leave(const parser::OpenMPAssumeConstruct &);
  void Enter(const parser::OpenMPDeclarativeAssumes &);
  void Leave(const parser::OpenMPDeclarativeAssumes &);
  void Enter(const parser::OpenMPBlockConstruct &);
  void Leave(const parser::OpenMPBlockConstruct &);
  void Leave(const parser::OmpBeginDirective &);
  void Enter(const parser::OmpEndDirective &);
  void Leave(const parser::OmpEndDirective &);

  void Enter(const parser::OpenMPSectionsConstruct &);
  void Leave(const parser::OpenMPSectionsConstruct &);
  void Enter(const parser::OmpEndSectionsDirective &);
  void Leave(const parser::OmpEndSectionsDirective &);

  void Enter(const parser::OmpDeclareVariantDirective &);
  void Leave(const parser::OmpDeclareVariantDirective &);
  void Enter(const parser::OpenMPDeclareSimdConstruct &);
  void Leave(const parser::OpenMPDeclareSimdConstruct &);
  void Enter(const parser::OpenMPDeclarativeAllocate &);
  void Leave(const parser::OpenMPDeclarativeAllocate &);
  void Enter(const parser::OpenMPDeclareMapperConstruct &);
  void Leave(const parser::OpenMPDeclareMapperConstruct &);
  void Enter(const parser::OpenMPDeclareReductionConstruct &);
  void Leave(const parser::OpenMPDeclareReductionConstruct &);
  void Enter(const parser::OpenMPDeclareTargetConstruct &);
  void Leave(const parser::OpenMPDeclareTargetConstruct &);
  void Enter(const parser::OpenMPDepobjConstruct &);
  void Leave(const parser::OpenMPDepobjConstruct &);
  void Enter(const parser::OmpDeclareTargetWithList &);
  void Enter(const parser::OmpDeclareTargetWithClause &);
  void Leave(const parser::OmpDeclareTargetWithClause &);
  void Enter(const parser::OpenMPDispatchConstruct &);
  void Leave(const parser::OpenMPDispatchConstruct &);
  void Enter(const parser::OmpErrorDirective &);
  void Leave(const parser::OmpErrorDirective &);
  void Enter(const parser::OpenMPExecutableAllocate &);
  void Leave(const parser::OpenMPExecutableAllocate &);
  void Enter(const parser::OpenMPAllocatorsConstruct &);
  void Leave(const parser::OpenMPAllocatorsConstruct &);
  void Enter(const parser::OpenMPRequiresConstruct &);
  void Leave(const parser::OpenMPRequiresConstruct &);
  void Enter(const parser::OpenMPThreadprivate &);
  void Leave(const parser::OpenMPThreadprivate &);

  void Enter(const parser::OpenMPSimpleStandaloneConstruct &);
  void Leave(const parser::OpenMPSimpleStandaloneConstruct &);
  void Enter(const parser::OpenMPFlushConstruct &);
  void Leave(const parser::OpenMPFlushConstruct &);
  void Enter(const parser::OpenMPCancelConstruct &);
  void Leave(const parser::OpenMPCancelConstruct &);
  void Enter(const parser::OpenMPCancellationPointConstruct &);
  void Leave(const parser::OpenMPCancellationPointConstruct &);
  void Enter(const parser::OpenMPCriticalConstruct &);
  void Leave(const parser::OpenMPCriticalConstruct &);
  void Enter(const parser::OpenMPAtomicConstruct &);
  void Leave(const parser::OpenMPAtomicConstruct &);

  void Leave(const parser::OmpClauseList &);
  void Enter(const parser::OmpClause &);

  void Enter(const parser::DoConstruct &);
  void Leave(const parser::DoConstruct &);

  void Enter(const parser::OmpDirectiveSpecification &);
  void Leave(const parser::OmpDirectiveSpecification &);

  void Enter(const parser::OmpMetadirectiveDirective &);
  void Leave(const parser::OmpMetadirectiveDirective &);

  void Enter(const parser::OmpContextSelector &);
  void Leave(const parser::OmpContextSelector &);

#define GEN_FLANG_CLAUSE_CHECK_ENTER
#include "toolchain/Frontend/OpenMP/OMP.inc"

private:
  bool CheckAllowedClause(toolchainOmpClause clause);
  void CheckVariableListItem(const SymbolSourceMap &symbols);
  void CheckDirectiveSpelling(
      parser::CharBlock spelling, toolchain::omp::Directive id);
  void CheckMultipleOccurrence(semantics::UnorderedSymbolSet &listVars,
      const std::list<parser::Name> &nameList, const parser::CharBlock &item,
      const std::string &clauseName);
  void CheckMultListItems();
  void CheckStructureComponent(
      const parser::OmpObjectList &objects, toolchain::omp::Clause clauseId);
  bool HasInvalidWorksharingNesting(
      const parser::CharBlock &, const OmpDirectiveSet &);
  bool IsCloselyNestedRegion(const OmpDirectiveSet &set);
  void HasInvalidTeamsNesting(
      const toolchain::omp::Directive &dir, const parser::CharBlock &source);
  void HasInvalidDistributeNesting(const parser::OpenMPLoopConstruct &x);
  void HasInvalidLoopBinding(const parser::OpenMPLoopConstruct &x);
  // specific clause related
  void CheckAllowedMapTypes(
      parser::OmpMapType::Value, toolchain::ArrayRef<parser::OmpMapType::Value>);

  const std::list<parser::OmpTraitProperty> &GetTraitPropertyList(
      const parser::OmpTraitSelector &);
  std::optional<toolchain::omp::Clause> GetClauseFromProperty(
      const parser::OmpTraitProperty &);

  void CheckTraitSelectorList(const std::list<parser::OmpTraitSelector> &);
  void CheckTraitSetSelector(const parser::OmpTraitSetSelector &);
  void CheckTraitScore(const parser::OmpTraitScore &);
  bool VerifyTraitPropertyLists(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);
  void CheckTraitSelector(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);
  void CheckTraitADMO(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);
  void CheckTraitCondition(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);
  void CheckTraitDeviceNum(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);
  void CheckTraitRequires(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);
  void CheckTraitSimd(
      const parser::OmpTraitSetSelector &, const parser::OmpTraitSelector &);

  toolchain::StringRef getClauseName(toolchain::omp::Clause clause) override;
  toolchain::StringRef getDirectiveName(toolchain::omp::Directive directive) override;

  template < //
      typename LessTy, typename RangeTy,
      typename IterTy = decltype(std::declval<RangeTy>().begin())>
  std::optional<IterTy> FindDuplicate(RangeTy &&);

  void CheckDependList(const parser::DataRef &);
  void CheckDependArraySection(
      const common::Indirection<parser::ArrayElement> &, const parser::Name &);
  void CheckDoacross(const parser::OmpDoacross &doa);
  bool IsDataRefTypeParamInquiry(const parser::DataRef *dataRef);
  void CheckVarIsNotPartOfAnotherVar(const parser::CharBlock &source,
      const parser::OmpObject &obj, toolchain::StringRef clause = "");
  void CheckVarIsNotPartOfAnotherVar(const parser::CharBlock &source,
      const parser::OmpObjectList &objList, toolchain::StringRef clause = "");
  void CheckThreadprivateOrDeclareTargetVar(
      const parser::OmpObjectList &objList);
  void CheckSymbolNames(
      const parser::CharBlock &source, const parser::OmpObjectList &objList);
  void CheckIntentInPointer(SymbolSourceMap &, const toolchain::omp::Clause);
  void CheckProcedurePointer(SymbolSourceMap &, const toolchain::omp::Clause);
  void CheckCrayPointee(const parser::OmpObjectList &objectList,
      toolchain::StringRef clause, bool suggestToUseCrayPointer = true);
  void GetSymbolsInObjectList(const parser::OmpObjectList &, SymbolSourceMap &);
  void CheckDefinableObjects(SymbolSourceMap &, const toolchain::omp::Clause);
  void CheckCopyingPolymorphicAllocatable(
      SymbolSourceMap &, const toolchain::omp::Clause);
  void CheckPrivateSymbolsInOuterCxt(
      SymbolSourceMap &, DirectivesClauseTriple &, const toolchain::omp::Clause);
  const parser::Name GetLoopIndex(const parser::DoConstruct *x);
  void SetLoopInfo(const parser::OpenMPLoopConstruct &x);
  void CheckIsLoopIvPartOfClause(
      toolchainOmpClause clause, const parser::OmpObjectList &ompObjectList);
  bool CheckTargetBlockOnlyTeams(const parser::Block &);
  void CheckWorkshareBlockStmts(const parser::Block &, parser::CharBlock);

  void CheckIteratorRange(const parser::OmpIteratorSpecifier &x);
  void CheckIteratorModifier(const parser::OmpIterator &x);
  void CheckLoopItrVariableIsInt(const parser::OpenMPLoopConstruct &x);
  void CheckDoWhile(const parser::OpenMPLoopConstruct &x);
  void CheckAssociatedLoopConstraints(const parser::OpenMPLoopConstruct &x);
  template <typename T, typename D> bool IsOperatorValid(const T &, const D &);

  void CheckStorageOverlap(const evaluate::Expr<evaluate::SomeType> &,
      toolchain::ArrayRef<evaluate::Expr<evaluate::SomeType>>, parser::CharBlock);
  void ErrorShouldBeVariable(const MaybeExpr &expr, parser::CharBlock source);
  void CheckAtomicType(
      SymbolRef sym, parser::CharBlock source, std::string_view name);
  void CheckAtomicVariable(
      const evaluate::Expr<evaluate::SomeType> &, parser::CharBlock);
  std::pair<const parser::ExecutionPartConstruct *,
      const parser::ExecutionPartConstruct *>
  CheckUpdateCapture(const parser::ExecutionPartConstruct *ec1,
      const parser::ExecutionPartConstruct *ec2, parser::CharBlock source);
  void CheckAtomicCaptureAssignment(const evaluate::Assignment &capture,
      const SomeExpr &atom, parser::CharBlock source);
  void CheckAtomicReadAssignment(
      const evaluate::Assignment &read, parser::CharBlock source);
  void CheckAtomicWriteAssignment(
      const evaluate::Assignment &write, parser::CharBlock source);
  std::optional<evaluate::Assignment> CheckAtomicUpdateAssignment(
      const evaluate::Assignment &update, parser::CharBlock source);
  std::pair<bool, bool> CheckAtomicUpdateAssignmentRhs(const SomeExpr &atom,
      const SomeExpr &rhs, parser::CharBlock source, bool suppressDiagnostics);
  void CheckAtomicConditionalUpdateAssignment(const SomeExpr &cond,
      parser::CharBlock condSource, const evaluate::Assignment &assign,
      parser::CharBlock assignSource);
  void CheckAtomicConditionalUpdateStmt(
      const AnalyzedCondStmt &update, parser::CharBlock source);
  void CheckAtomicUpdateOnly(const parser::OpenMPAtomicConstruct &x,
      const parser::Block &body, parser::CharBlock source);
  void CheckAtomicConditionalUpdate(const parser::OpenMPAtomicConstruct &x,
      const parser::Block &body, parser::CharBlock source);
  void CheckAtomicUpdateCapture(const parser::OpenMPAtomicConstruct &x,
      const parser::Block &body, parser::CharBlock source);
  void CheckAtomicConditionalUpdateCapture(
      const parser::OpenMPAtomicConstruct &x, const parser::Block &body,
      parser::CharBlock source);
  void CheckAtomicRead(const parser::OpenMPAtomicConstruct &x);
  void CheckAtomicWrite(const parser::OpenMPAtomicConstruct &x);
  void CheckAtomicUpdate(const parser::OpenMPAtomicConstruct &x);

  void CheckDistLinear(const parser::OpenMPLoopConstruct &x);
  void CheckSIMDNest(const parser::OpenMPConstruct &x);
  void CheckTargetNest(const parser::OpenMPConstruct &x);
  void CheckTargetUpdate();
  void CheckDependenceType(const parser::OmpDependenceType::Value &x);
  void CheckTaskDependenceType(const parser::OmpTaskDependenceType::Value &x);
  std::optional<toolchain::omp::Directive> GetCancelType(
      toolchain::omp::Directive cancelDir, const parser::CharBlock &cancelSource,
      const std::optional<parser::OmpClauseList> &maybeClauses);
  void CheckCancellationNest(
      const parser::CharBlock &source, toolchain::omp::Directive type);
  std::int64_t GetOrdCollapseLevel(const parser::OpenMPLoopConstruct &x);
  void CheckReductionObjects(
      const parser::OmpObjectList &objects, toolchain::omp::Clause clauseId);
  bool CheckReductionOperator(const parser::OmpReductionIdentifier &ident,
      parser::CharBlock source, toolchain::omp::Clause clauseId);
  void CheckReductionObjectTypes(const parser::OmpObjectList &objects,
      const parser::OmpReductionIdentifier &ident);
  void CheckReductionModifier(const parser::OmpReductionModifier &);
  void CheckLastprivateModifier(const parser::OmpLastprivateModifier &);
  void CheckMasterNesting(const parser::OpenMPBlockConstruct &x);
  void ChecksOnOrderedAsBlock();
  void CheckBarrierNesting(const parser::OpenMPSimpleStandaloneConstruct &x);
  void CheckScan(const parser::OpenMPSimpleStandaloneConstruct &x);
  void ChecksOnOrderedAsStandalone();
  void CheckOrderedDependClause(std::optional<std::int64_t> orderedValue);
  void CheckReductionArraySection(
      const parser::OmpObjectList &ompObjectList, toolchain::omp::Clause clauseId);
  void CheckArraySection(const parser::ArrayElement &arrayElement,
      const parser::Name &name, const toolchain::omp::Clause clause);
  void CheckSharedBindingInOuterContext(
      const parser::OmpObjectList &ompObjectList);
  void CheckIfContiguous(const parser::OmpObject &object);
  const parser::Name *GetObjectName(const parser::OmpObject &object);
  const parser::OmpObjectList *GetOmpObjectList(const parser::OmpClause &);
  void CheckPredefinedAllocatorRestriction(const parser::CharBlock &source,
      const parser::OmpObjectList &ompObjectList);
  void CheckPredefinedAllocatorRestriction(
      const parser::CharBlock &source, const parser::Name &name);
  bool isPredefinedAllocator{false};

  void CheckAllowedRequiresClause(toolchainOmpClause clause);
  bool deviceConstructFound_{false};

  void CheckAlignValue(const parser::OmpClause &);

  void AddEndDirectiveClauses(const parser::OmpClauseList &clauses);

  void EnterDirectiveNest(const int index) { directiveNest_[index]++; }
  void ExitDirectiveNest(const int index) { directiveNest_[index]--; }
  int GetDirectiveNest(const int index) { return directiveNest_[index]; }
  inline void ErrIfAllocatableVariable(const parser::Variable &);
  inline void ErrIfLHSAndRHSSymbolsMatch(
      const parser::Variable &, const parser::Expr &);
  inline void ErrIfNonScalarAssignmentStmt(
      const parser::Variable &, const parser::Expr &);
  enum directiveNestType : int {
    SIMDNest,
    TargetBlockOnlyTeams,
    TargetNest,
    DeclarativeNest,
    ContextSelectorNest,
    MetadirectiveNest,
    LastType = MetadirectiveNest,
  };
  int directiveNest_[LastType + 1] = {0};

  parser::CharBlock visitedAtomicSource_;
  SymbolSourceMap deferredNonVariables_;

  using LoopConstruct = std::variant<const parser::DoConstruct *,
      const parser::OpenMPLoopConstruct *>;
  std::vector<LoopConstruct> loopStack_;
};

/// Find a duplicate entry in the range, and return an iterator to it.
/// If there are no duplicate entries, return nullopt.
template <typename LessTy, typename RangeTy, typename IterTy>
std::optional<IterTy> OmpStructureChecker::FindDuplicate(RangeTy &&range) {
  // Deal with iterators, since the actual elements may be rvalues (i.e.
  // have no addresses), for example with custom-constructed ranges that
  // are not simple c.begin()..c.end().
  std::set<IterTy, LessTy> uniq;
  for (auto it{range.begin()}, end{range.end()}; it != end; ++it) {
    if (!uniq.insert(it).second) {
      return it;
    }
  }
  return std::nullopt;
}

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_CHECK_OMP_STRUCTURE_H_
