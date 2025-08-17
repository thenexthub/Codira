//===-- Decomposer.cpp -- Compound directive decomposition ----------------===//
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

#include "Decomposer.h"

#include "Utils.h"
#include "language/Compability/Lower/OpenMP/Clauses.h"
#include "language/Compability/Lower/PFTBuilder.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Tools/CrossToolHelpers.h"
#include "mlir/IR/BuiltinOps.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Frontend/OpenMP/ClauseT.h"
#include "toolchain/Frontend/OpenMP/ConstructDecompositionT.h"
#include "toolchain/Frontend/OpenMP/OMP.h"
#include "toolchain/Support/raw_ostream.h"

#include <optional>
#include <utility>
#include <variant>

using namespace language::Compability;

namespace {
using namespace language::Compability::lower::omp;

struct ConstructDecomposition {
  ConstructDecomposition(mlir::ModuleOp modOp,
                         semantics::SemanticsContext &semaCtx,
                         lower::pft::Evaluation &ev,
                         toolchain::omp::Directive compound,
                         const List<Clause> &clauses)
      : semaCtx(semaCtx), mod(modOp), eval(ev) {
    tomp::ConstructDecompositionT decompose(getOpenMPVersionAttribute(modOp),
                                            *this, compound,
                                            toolchain::ArrayRef(clauses));
    output = std::move(decompose.output);
  }

  // Given an object, return its base object if one exists.
  std::optional<Object> getBaseObject(const Object &object) {
    return lower::omp::getBaseObject(object, semaCtx);
  }

  // Return the iteration variable of the associated loop if any.
  std::optional<Object> getLoopIterVar() {
    if (semantics::Symbol *symbol = getIterationVariableSymbol(eval))
      return Object{symbol, /*designator=*/{}};
    return std::nullopt;
  }

  semantics::SemanticsContext &semaCtx;
  mlir::ModuleOp mod;
  lower::pft::Evaluation &eval;
  List<UnitConstruct> output;
};
} // namespace

namespace language::Compability::lower::omp {
LLVM_DUMP_METHOD toolchain::raw_ostream &operator<<(toolchain::raw_ostream &os,
                                               const UnitConstruct &uc) {
  os << toolchain::omp::getOpenMPDirectiveName(uc.id, toolchain::omp::FallbackVersion);
  for (auto [index, clause] : toolchain::enumerate(uc.clauses)) {
    os << (index == 0 ? '\t' : ' ');
    os << toolchain::omp::getOpenMPClauseName(clause.id);
  }
  return os;
}

ConstructQueue buildConstructQueue(
    mlir::ModuleOp modOp, language::Compability::semantics::SemanticsContext &semaCtx,
    language::Compability::lower::pft::Evaluation &eval, const parser::CharBlock &source,
    toolchain::omp::Directive compound, const List<Clause> &clauses) {

  ConstructDecomposition decompose(modOp, semaCtx, eval, compound, clauses);
  assert(!decompose.output.empty() && "Construct decomposition failed");

  for (UnitConstruct &uc : decompose.output) {
    assert(getLeafConstructs(uc.id).empty() && "unexpected compound directive");
    //  If some clauses are left without source information, use the directive's
    //  source.
    for (auto &clause : uc.clauses)
      if (clause.source.empty())
        clause.source = source;
  }

  return decompose.output;
}

bool matchLeafSequence(ConstructQueue::const_iterator item,
                       const ConstructQueue &queue,
                       toolchain::omp::Directive directive) {
  toolchain::ArrayRef<toolchain::omp::Directive> leafDirs =
      toolchain::omp::getLeafConstructsOrSelf(directive);

  for (auto [dir, leaf] :
       toolchain::zip_longest(leafDirs, toolchain::make_range(item, queue.end()))) {
    if (!dir.has_value() || !leaf.has_value())
      return false;

    if (*dir != leaf->id)
      return false;
  }

  return true;
}

bool isLastItemInQueue(ConstructQueue::const_iterator item,
                       const ConstructQueue &queue) {
  return std::next(item) == queue.end();
}
} // namespace language::Compability::lower::omp
