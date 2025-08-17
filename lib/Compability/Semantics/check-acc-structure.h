//===-- lib/Semantics/check-acc-structure.h ---------------------*- C++ -*-===//
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
// OpenACC 3.3 structure validity check list
//    1. invalid clauses on directive
//    2. invalid repeated clauses on directive
//    3. invalid nesting of regions
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_ACC_STRUCTURE_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_ACC_STRUCTURE_H_

#include "check-directive-structure.h"
#include "language/Compability/Common/enum-set.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/semantics.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Frontend/OpenACC/ACC.h.inc"

using AccDirectiveSet = language::Compability::common::EnumSet<toolchain::acc::Directive,
    toolchain::acc::Directive_enumSize>;

using AccClauseSet =
    language::Compability::common::EnumSet<toolchain::acc::Clause, toolchain::acc::Clause_enumSize>;

#define GEN_FLANG_DIRECTIVE_CLAUSE_SETS
#include "toolchain/Frontend/OpenACC/ACC.inc"

namespace language::Compability::semantics {

class AccStructureChecker
    : public DirectiveStructureChecker<toolchain::acc::Directive, toolchain::acc::Clause,
          parser::AccClause, toolchain::acc::Clause_enumSize> {
public:
  AccStructureChecker(SemanticsContext &context)
      : DirectiveStructureChecker(context,
#define GEN_FLANG_DIRECTIVE_CLAUSE_MAP
#include "toolchain/Frontend/OpenACC/ACC.inc"
        ) {
  }

  // Construct and directives
  void Enter(const parser::OpenACCBlockConstruct &);
  void Leave(const parser::OpenACCBlockConstruct &);
  void Enter(const parser::OpenACCCombinedConstruct &);
  void Leave(const parser::OpenACCCombinedConstruct &);
  void Enter(const parser::OpenACCLoopConstruct &);
  void Leave(const parser::OpenACCLoopConstruct &);
  void Enter(const parser::OpenACCRoutineConstruct &);
  void Leave(const parser::OpenACCRoutineConstruct &);
  void Enter(const parser::OpenACCStandaloneConstruct &);
  void Leave(const parser::OpenACCStandaloneConstruct &);
  void Enter(const parser::OpenACCStandaloneDeclarativeConstruct &);
  void Leave(const parser::OpenACCStandaloneDeclarativeConstruct &);
  void Enter(const parser::OpenACCWaitConstruct &);
  void Leave(const parser::OpenACCWaitConstruct &);
  void Enter(const parser::OpenACCAtomicConstruct &);
  void Leave(const parser::OpenACCAtomicConstruct &);
  void Enter(const parser::OpenACCCacheConstruct &);
  void Leave(const parser::OpenACCCacheConstruct &);
  void Enter(const parser::AccAtomicUpdate &);
  void Enter(const parser::AccAtomicCapture &);
  void Enter(const parser::AccAtomicWrite &);
  void Enter(const parser::AccAtomicRead &);
  void Enter(const parser::OpenACCEndConstruct &);

  // Clauses
  void Leave(const parser::AccClauseList &);
  void Enter(const parser::AccClause &);

  void Enter(const parser::Module &);
  void Enter(const parser::SubroutineSubprogram &);
  void Enter(const parser::FunctionSubprogram &);
  void Enter(const parser::SeparateModuleSubprogram &);
  void Enter(const parser::DoConstruct &);
  void Leave(const parser::DoConstruct &);

#define GEN_FLANG_CLAUSE_CHECK_ENTER
#include "toolchain/Frontend/OpenACC/ACC.inc"

private:
  void CheckAtomicStmt(
      const parser::AssignmentStmt &assign, const std::string &construct);
  void CheckAtomicUpdateStmt(const parser::AssignmentStmt &assign,
      const SomeExpr &updateVar, const SomeExpr *captureVar);
  void CheckAtomicCaptureStmt(const parser::AssignmentStmt &assign,
      const SomeExpr *updateVar, const SomeExpr &captureVar);
  void CheckAtomicWriteStmt(const parser::AssignmentStmt &assign,
      const SomeExpr &updateVar, const SomeExpr *captureVar);
  void CheckAtomicUpdateVariable(
      const parser::Variable &updateVar, const parser::Variable &captureVar);
  void CheckAtomicCaptureVariable(
      const parser::Variable &captureVar, const parser::Variable &updateVar);

  bool CheckAllowedModifier(toolchain::acc::Clause clause);
  bool IsComputeConstruct(toolchain::acc::Directive directive) const;
  bool IsLoopConstruct(toolchain::acc::Directive directive) const;
  std::optional<toolchain::acc::Directive> getParentComputeConstruct() const;
  bool IsInsideComputeConstruct() const;
  bool IsInsideParallelConstruct() const;
  void CheckNotInComputeConstruct();
  std::optional<std::int64_t> getGangDimensionSize(
      DirectiveContext &dirContext);
  void CheckNotInSameOrSubLevelLoopConstruct();
  void CheckMultipleOccurrenceInDeclare(
      const parser::AccObjectList &, toolchain::acc::Clause);
  void CheckMultipleOccurrenceInDeclare(
      const parser::AccObjectListWithModifier &, toolchain::acc::Clause);
  toolchain::StringRef getClauseName(toolchain::acc::Clause clause) override;
  toolchain::StringRef getDirectiveName(toolchain::acc::Directive directive) override;

  toolchain::SmallDenseMap<Symbol *, toolchain::acc::Clause> declareSymbols;
  unsigned loopNestLevel = 0;
};

} // namespace language::Compability::semantics

#endif // FORTRAN_SEMANTICS_CHECK_ACC_STRUCTURE_H_
