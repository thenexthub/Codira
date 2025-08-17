//===-- Lower/OpenMP/ClauseProcessor.h --------------------------*- C++ -*-===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_COMPABILITY_LOWER_CLAUSEPROCESSOR_H
#define LANGUAGE_COMPABILITY_LOWER_CLAUSEPROCESSOR_H

#include "ClauseFinder.h"
#include "Utils.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/Bridge.h"
#include "language/Compability/Lower/DirectivesCommon.h"
#include "language/Compability/Lower/OpenMP/Clauses.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Parser/dump-parse-tree.h"
#include "language/Compability/Parser/parse-tree.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"

namespace fir {
class FirOpBuilder;
} // namespace fir

namespace language::Compability {
namespace lower {
namespace omp {

// Container type for tracking user specified Defaultmaps for a target region
using DefaultMapsTy = std::map<clause::Defaultmap::VariableCategory,
                               clause::Defaultmap::ImplicitBehavior>;

/// Class that handles the processing of OpenMP clauses.
///
/// Its `process<ClauseName>()` methods perform MLIR code generation for their
/// corresponding clause if it is present in the clause list. Otherwise, they
/// will return `false` to signal that the clause was not found.
///
/// The intended use of this class is to move clause processing outside of
/// construct processing, since the same clauses can appear attached to
/// different constructs and constructs can be combined, so that code
/// duplication is minimized.
///
/// Each construct-lowering function only calls the `process<ClauseName>()`
/// methods that relate to clauses that can impact the lowering of that
/// construct.
class ClauseProcessor {
public:
  ClauseProcessor(lower::AbstractConverter &converter,
                  semantics::SemanticsContext &semaCtx,
                  const List<Clause> &clauses)
      : converter(converter), semaCtx(semaCtx), clauses(clauses) {}

  // 'Unique' clauses: They can appear at most once in the clause list.
  bool processBare(mlir::omp::BareClauseOps &result) const;
  bool processBind(mlir::omp::BindClauseOps &result) const;
  bool processCancelDirectiveName(
      mlir::omp::CancelDirectiveNameClauseOps &result) const;
  bool
  processCollapse(mlir::Location currentLocation, lower::pft::Evaluation &eval,
                  mlir::omp::LoopRelatedClauseOps &result,
                  toolchain::SmallVectorImpl<const semantics::Symbol *> &iv) const;
  bool processDevice(lower::StatementContext &stmtCtx,
                     mlir::omp::DeviceClauseOps &result) const;
  bool processDeviceType(mlir::omp::DeviceTypeClauseOps &result) const;
  bool processDistSchedule(lower::StatementContext &stmtCtx,
                           mlir::omp::DistScheduleClauseOps &result) const;
  bool processExclusive(mlir::Location currentLocation,
                        mlir::omp::ExclusiveClauseOps &result) const;
  bool processFilter(lower::StatementContext &stmtCtx,
                     mlir::omp::FilterClauseOps &result) const;
  bool processFinal(lower::StatementContext &stmtCtx,
                    mlir::omp::FinalClauseOps &result) const;
  bool processGrainsize(lower::StatementContext &stmtCtx,
                        mlir::omp::GrainsizeClauseOps &result) const;
  bool processHasDeviceAddr(
      lower::StatementContext &stmtCtx,
      mlir::omp::HasDeviceAddrClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &hasDeviceSyms) const;
  bool processHint(mlir::omp::HintClauseOps &result) const;
  bool processInclusive(mlir::Location currentLocation,
                        mlir::omp::InclusiveClauseOps &result) const;
  bool processMergeable(mlir::omp::MergeableClauseOps &result) const;
  bool processNowait(mlir::omp::NowaitClauseOps &result) const;
  bool processNumTasks(lower::StatementContext &stmtCtx,
                       mlir::omp::NumTasksClauseOps &result) const;
  bool processNumTeams(lower::StatementContext &stmtCtx,
                       mlir::omp::NumTeamsClauseOps &result) const;
  bool processNumThreads(lower::StatementContext &stmtCtx,
                         mlir::omp::NumThreadsClauseOps &result) const;
  bool processOrder(mlir::omp::OrderClauseOps &result) const;
  bool processOrdered(mlir::omp::OrderedClauseOps &result) const;
  bool processPriority(lower::StatementContext &stmtCtx,
                       mlir::omp::PriorityClauseOps &result) const;
  bool processProcBind(mlir::omp::ProcBindClauseOps &result) const;
  bool processSafelen(mlir::omp::SafelenClauseOps &result) const;
  bool processSchedule(lower::StatementContext &stmtCtx,
                       mlir::omp::ScheduleClauseOps &result) const;
  bool processSimdlen(mlir::omp::SimdlenClauseOps &result) const;
  bool processThreadLimit(lower::StatementContext &stmtCtx,
                          mlir::omp::ThreadLimitClauseOps &result) const;
  bool processUntied(mlir::omp::UntiedClauseOps &result) const;

  bool processDetach(mlir::omp::DetachClauseOps &result) const;
  // 'Repeatable' clauses: They can appear multiple times in the clause list.
  bool processAligned(mlir::omp::AlignedClauseOps &result) const;
  bool processAllocate(mlir::omp::AllocateClauseOps &result) const;
  bool processCopyin() const;
  bool processCopyprivate(mlir::Location currentLocation,
                          mlir::omp::CopyprivateClauseOps &result) const;
  bool processDefaultMap(lower::StatementContext &stmtCtx,
                         DefaultMapsTy &result) const;
  bool processDepend(lower::SymMap &symMap, lower::StatementContext &stmtCtx,
                     mlir::omp::DependClauseOps &result) const;
  bool
  processEnter(toolchain::SmallVectorImpl<DeclareTargetCaptureInfo> &result) const;
  bool processIf(omp::clause::If::DirectiveNameModifier directiveName,
                 mlir::omp::IfClauseOps &result) const;
  bool processInReduction(
      mlir::Location currentLocation, mlir::omp::InReductionClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &outReductionSyms) const;
  bool processIsDevicePtr(
      mlir::omp::IsDevicePtrClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &isDeviceSyms) const;
  bool processLinear(mlir::omp::LinearClauseOps &result) const;
  bool
  processLink(toolchain::SmallVectorImpl<DeclareTargetCaptureInfo> &result) const;

  // This method is used to process a map clause.
  // The optional parameter mapSyms is used to store the original Fortran symbol
  // for the map operands. It may be used later on to create the block_arguments
  // for some of the directives that require it.
  bool processMap(mlir::Location currentLocation,
                  lower::StatementContext &stmtCtx,
                  mlir::omp::MapClauseOps &result,
                  toolchain::omp::Directive directive = toolchain::omp::OMPD_unknown,
                  toolchain::SmallVectorImpl<const semantics::Symbol *> *mapSyms =
                      nullptr) const;
  bool processMotionClauses(lower::StatementContext &stmtCtx,
                            mlir::omp::MapClauseOps &result);
  bool processNontemporal(mlir::omp::NontemporalClauseOps &result) const;
  bool processReduction(
      mlir::Location currentLocation, mlir::omp::ReductionClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &reductionSyms) const;
  bool processTaskReduction(
      mlir::Location currentLocation, mlir::omp::TaskReductionClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &outReductionSyms) const;
  bool processTo(toolchain::SmallVectorImpl<DeclareTargetCaptureInfo> &result) const;
  bool processUseDeviceAddr(
      lower::StatementContext &stmtCtx,
      mlir::omp::UseDeviceAddrClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &useDeviceSyms) const;
  bool processUseDevicePtr(
      lower::StatementContext &stmtCtx,
      mlir::omp::UseDevicePtrClauseOps &result,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &useDeviceSyms) const;

  // Call this method for these clauses that should be supported but are not
  // implemented yet. It triggers a compilation error if any of the given
  // clauses is found.
  template <typename... Ts>
  void processTODO(mlir::Location currentLocation,
                   toolchain::omp::Directive directive) const;

private:
  using ClauseIterator = List<Clause>::const_iterator;

  /// Return the first instance of the given clause found in the clause list or
  /// `nullptr` if not present. If more than one instance is expected, use
  /// `findRepeatableClause` instead.
  template <typename T>
  const T *findUniqueClause(const parser::CharBlock **source = nullptr) const;

  /// Call `callbackFn` for each occurrence of the given clause. Return `true`
  /// if at least one instance was found.
  template <typename T>
  bool findRepeatableClause(
      std::function<void(const T &, const parser::CharBlock &source)>
          callbackFn) const;

  /// Set the `result` to a new `mlir::UnitAttr` if the clause is present.
  template <typename T>
  bool markClauseOccurrence(mlir::UnitAttr &result) const;

  void processMapObjects(
      lower::StatementContext &stmtCtx, mlir::Location clauseLocation,
      const omp::ObjectList &objects,
      toolchain::omp::OpenMPOffloadMappingFlags mapTypeBits,
      std::map<Object, OmpMapParentAndMemberData> &parentMemberIndices,
      toolchain::SmallVectorImpl<mlir::Value> &mapVars,
      toolchain::SmallVectorImpl<const semantics::Symbol *> &mapSyms,
      toolchain::StringRef mapperIdNameRef = "") const;

  lower::AbstractConverter &converter;
  semantics::SemanticsContext &semaCtx;
  List<Clause> clauses;
};

template <typename... Ts>
void ClauseProcessor::processTODO(mlir::Location currentLocation,
                                  toolchain::omp::Directive directive) const {
  auto checkUnhandledClause = [&](toolchain::omp::Clause id, const auto *x) {
    if (!x)
      return;
    unsigned version = semaCtx.langOptions().OpenMPVersion;
    bool isSimdDirective = toolchain::omp::getOpenMPDirectiveName(directive, version)
                               .upper()
                               .find("SIMD") != toolchain::StringRef::npos;
    if (!semaCtx.langOptions().OpenMPSimd || isSimdDirective)
      TODO(currentLocation,
           "Unhandled clause " + toolchain::omp::getOpenMPClauseName(id).upper() +
               " in " +
               toolchain::omp::getOpenMPDirectiveName(directive, version).upper() +
               " construct");
  };

  for (ClauseIterator it = clauses.begin(); it != clauses.end(); ++it)
    (checkUnhandledClause(it->id, std::get_if<Ts>(&it->u)), ...);
}

template <typename T>
const T *
ClauseProcessor::findUniqueClause(const parser::CharBlock **source) const {
  return ClauseFinder::findUniqueClause<T>(clauses, source);
}

template <typename T>
bool ClauseProcessor::findRepeatableClause(
    std::function<void(const T &, const parser::CharBlock &source)> callbackFn)
    const {
  return ClauseFinder::findRepeatableClause<T>(clauses, callbackFn);
}

template <typename T>
bool ClauseProcessor::markClauseOccurrence(mlir::UnitAttr &result) const {
  if (findUniqueClause<T>()) {
    result = converter.getFirOpBuilder().getUnitAttr();
    return true;
  }
  return false;
}

} // namespace omp
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_CLAUSEPROCESSOR_H
