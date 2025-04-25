//===--- CompileTimeInterpolationUtils.h ----------------------------------===//
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

// Utilities for the compile-time string interpolation approach used by the
// OSLogOptimization pass.

#ifndef SWIFT_SILOPTIMIZER_COMPILE_TIME_INTERPOLATION_H
#define SWIFT_SILOPTIMIZER_COMPILE_TIME_INTERPOLATION_H

#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILConstants.h"
#include "language/SILOptimizer/Utils/ConstExpr.h"

namespace language {

/// Decide if the given instruction (which could possibly be a call) should
/// be constant evaluated.
///
/// \returns true iff the given instruction is not a call or if it is, it calls
/// a known constant-evaluable function such as string append etc., or calls
/// a function annotate as "constant_evaluable".
bool shouldAttemptEvaluation(SILInstruction *inst);

/// Skip or evaluate the given instruction based on the evaluation policy and
/// handle errors. The policy is to evaluate all non-apply instructions as well
/// as apply instructions that are marked as "constant_evaluable".
std::pair<std::optional<SILBasicBlock::iterator>, std::optional<SymbolicValue>>
evaluateOrSkip(ConstExprStepEvaluator &stepEval, SILBasicBlock::iterator instI);

/// Given a vector of SILValues \p worklist, compute the set of transitive
/// users of these values (excluding the worklist values) by following the
/// use-def chain starting at value. Note that this function does not follow
/// use-def chains though branches.
void getTransitiveUsers(SILInstructionResultArray values,
                        SmallVectorImpl<SILInstruction *> &users);
} // end namespace language
#endif
