//===-----ConstantEvaluatorTester.cpp - Test Constant Evaluator -----------===//
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

#define DEBUG_TYPE "sil-constant-evaluation-tester"
#include "language/AST/DiagnosticsSIL.h"
#include "language/SIL/SILConstants.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/ConstExpr.h"

using namespace language;

namespace {

template <typename... T, typename... U>
static InFlightDiagnostic diagnose(ASTContext &Context, SourceLoc loc,
                                   Diag<T...> diag, U &&... args) {
  return Context.Diags.diagnose(loc, diag, std::forward<U>(args)...);
}

/// A compiler pass for testing constant evaluator in the step-wise evaluation
/// mode. The pass evaluates SIL functions whose names start with "interpret"
/// and outputs the constant value returned by the function or diagnostics if
/// the evaluation fails.
class ConstantEvaluatorTester : public SILFunctionTransform {

  bool shouldInterpret() {
    auto *fun = getFunction();
    return fun->getName().starts_with("interpret");
  }

  bool shouldSkipInstruction(SILInstruction *inst) {
    auto *applyInst = dyn_cast<ApplyInst>(inst);
    if (!applyInst)
      return false;

    auto *callee = applyInst->getReferencedFunctionOrNull();
    if (!callee)
      return false;

    return callee->getName().starts_with("skip");
  }

  void run() override {
    SILFunction *fun = getFunction();

    if (!shouldInterpret() || fun->empty())
      return;

    toolchain::errs() << "@" << fun->getName() << "\n";

    SymbolicValueBumpAllocator allocator;
    ConstExprStepEvaluator stepEvaluator(allocator, fun,
                                         getOptions().AssertConfig);

    for (auto currI = fun->getEntryBlock()->begin();;) {
      auto *inst = &(*currI);

      if (auto *returnInst = dyn_cast<ReturnInst>(inst)) {
        auto returnVal =
            stepEvaluator.lookupConstValue(returnInst->getOperand());

        if (!returnVal) {
          toolchain::errs() << "Returns unknown"
                       << "\n";
          break;
        }
        toolchain::errs() << "Returns " << returnVal.value() << "\n";
        break;
      }

      std::optional<SILBasicBlock::iterator> nextInstOpt;
      std::optional<SymbolicValue> errorVal;

      // If the instruction is marked as skip, skip it and make its effects
      // non-constant. Otherwise, try evaluating the instruction and if the
      // evaluation fails due to a previously skipped instruction,
      // skip the current instruction.
      if (shouldSkipInstruction(inst)) {
        std::tie(nextInstOpt, errorVal) =
            stepEvaluator.skipByMakingEffectsNonConstant(currI);
      } else {
        std::tie(nextInstOpt, errorVal) =
            stepEvaluator.tryEvaluateOrElseMakeEffectsNonConstant(currI);
      }

      // Diagnose errors in the evaluation. Unknown symbolic values produced
      // by skipping instructions are not considered errors.
      if (errorVal.has_value() &&
          !errorVal->isUnknownDueToUnevaluatedInstructions()) {
        errorVal->emitUnknownDiagnosticNotes(inst->getLoc());
        break;
      }

      if (!nextInstOpt) {
        diagnose(fun->getASTContext(), inst->getLoc().getSourceLoc(),
                 diag::constexpr_unknown_control_flow_due_to_skip);
        errorVal->emitUnknownDiagnosticNotes(inst->getLoc());
        break;
      }

      currI = nextInstOpt.value();
    }
  }
};

} // end anonymous namespace

SILTransform *language::createConstantEvaluatorTester() {
  return new ConstantEvaluatorTester();
}
