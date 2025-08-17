//===-- Decomposer.h -- Compound directive decomposition ------------------===//
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
#ifndef LANGUAGE_COMPABILITY_LOWER_OPENMP_DECOMPOSER_H
#define LANGUAGE_COMPABILITY_LOWER_OPENMP_DECOMPOSER_H

#include "language/Compability/Lower/OpenMP/Clauses.h"
#include "mlir/IR/BuiltinOps.h"
#include "toolchain/Frontend/OpenMP/ConstructDecompositionT.h"
#include "toolchain/Frontend/OpenMP/OMP.h"
#include "toolchain/Support/Compiler.h"

namespace toolchain {
class raw_ostream;
}

namespace language::Compability {
namespace semantics {
class SemanticsContext;
}
namespace lower::pft {
struct Evaluation;
}
} // namespace language::Compability

namespace language::Compability::lower::omp {
using UnitConstruct = tomp::DirectiveWithClauses<lower::omp::Clause>;
using ConstructQueue = List<UnitConstruct>;

LLVM_DUMP_METHOD toolchain::raw_ostream &operator<<(toolchain::raw_ostream &os,
                                               const UnitConstruct &uc);

// Given a potentially compound construct with a list of clauses that
// apply to it, break it up into individual sub-constructs each with
// the subset of applicable clauses (plus implicit clauses, if any).
// From that create a work queue where each work item corresponds to
// the sub-construct with its clauses.
ConstructQueue buildConstructQueue(mlir::ModuleOp modOp,
                                   semantics::SemanticsContext &semaCtx,
                                   lower::pft::Evaluation &eval,
                                   const parser::CharBlock &source,
                                   toolchain::omp::Directive compound,
                                   const List<Clause> &clauses);

bool isLastItemInQueue(ConstructQueue::const_iterator item,
                       const ConstructQueue &queue);

/// Try to match the leaf constructs conforming the given \c directive to the
/// range of leaf constructs starting from \c item to the end of the \c queue.
/// If \c directive doesn't represent a compound directive, check that \c item
/// matches that directive and is the only element before the end of the
/// \c queue.
bool matchLeafSequence(ConstructQueue::const_iterator item,
                       const ConstructQueue &queue,
                       toolchain::omp::Directive directive);
} // namespace language::Compability::lower::omp

#endif // FORTRAN_LOWER_OPENMP_DECOMPOSER_H
