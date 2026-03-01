//===- PDLExtensionOps.cpp - PDL extension for the Transform dialect ------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "mlir/Dialect/Transform/PDLExtension/PDLExtensionOps.h"
#include "mlir/Dialect/PDL/IR/PDLOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/Rewrite/FrozenRewritePatternSet.h"
#include "mlir/Rewrite/PatternApplicator.h"
#include "vm/core/ADT/ScopeExit.h"

using namespace mlir;

MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::transform::PDLMatchHooks)

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/PDLExtension/PDLExtensionOps.cpp.inc"

//===----------------------------------------------------------------------===//
// PatternApplicatorExtension
//===----------------------------------------------------------------------===//

namespace {
/// A TransformState extension that keeps track of compiled PDL pattern sets.
/// This is intended to be used along the WithPDLPatterns op. The extension
/// can be constructed given an operation that has a SymbolTable trait and
/// contains pdl::PatternOp instances. The patterns are compiled lazily and one
/// by one when requested; this behavior is subject to change.
class PatternApplicatorExtension : public transform::TransformState::Extension {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(PatternApplicatorExtension)

  /// Creates the extension for patterns contained in `patternContainer`.
  explicit PatternApplicatorExtension(transform::TransformState &state,
                                      Operation *patternContainer)
      : Extension(state), patterns(patternContainer) {}

  /// Appends to `results` the operations contained in `root` that matched the
  /// PDL pattern with the given name. Note that `root` may or may not be the
  /// operation that contains PDL patterns. Reports an error if the pattern
  /// cannot be found. Note that when no operations are matched, this still
  /// succeeds as long as the pattern exists.
  LogicalResult findAllMatches(StringRef patternName, Operation *root,
                               SmallVectorImpl<Operation *> &results);

private:
  /// Map from the pattern name to a singleton set of rewrite patterns that only
  /// contains the pattern with this name. Populated when the pattern is first
  /// requested.
  // TODO: reconsider the efficiency of this storage when more usage data is
  // available. Storing individual patterns in a set and triggering compilation
  // for each of them has overhead. So does compiling a large set of patterns
  // only to apply a handful of them.
  toolchain::StringMap<FrozenRewritePatternSet> compiledPatterns;

  /// A symbol table operation containing the relevant PDL patterns.
  SymbolTable patterns;
};

LogicalResult PatternApplicatorExtension::findAllMatches(
    StringRef patternName, Operation *root,
    SmallVectorImpl<Operation *> &results) {
  auto it = compiledPatterns.find(patternName);
  if (it == compiledPatterns.end()) {
    auto patternOp = patterns.lookup<pdl::PatternOp>(patternName);
    if (!patternOp)
      return failure();

    // Copy the pattern operation into a new module that is compiled and
    // consumed by the PDL interpreter.
    OwningOpRef<ModuleOp> pdlModuleOp = ModuleOp::create(patternOp.getLoc());
    auto builder = OpBuilder::atBlockEnd(pdlModuleOp->getBody());
    builder.clone(*patternOp);
    PDLPatternModule patternModule(std::move(pdlModuleOp));

    // Merge in the hooks owned by the dialect. Make a copy as they may be
    // also used by the following operations.
    auto *dialect =
        root->getContext()->getLoadedDialect<transform::TransformDialect>();
    for (const auto &[name, constraintFn] :
         dialect->getExtraData<transform::PDLMatchHooks>()
             .getPDLConstraintHooks()) {
      patternModule.registerConstraintFunction(name, constraintFn);
    }

    // Register a noop rewriter because PDL requires patterns to end with some
    // rewrite call.
    patternModule.registerRewriteFunction(
        "transform.dialect", [](PatternRewriter &, Operation *) {});

    it = compiledPatterns
             .try_emplace(patternOp.getName(), std::move(patternModule))
             .first;
  }

  PatternApplicator applicator(it->second);
  // We want to discourage direct use of PatternRewriter in APIs but In this
  // very specific case, an IRRewriter is not enough.
  PatternRewriter rewriter(root->getContext());
  applicator.applyDefaultCostModel();
  root->walk([&](Operation *op) {
    if (succeeded(applicator.matchAndRewrite(op, rewriter)))
      results.push_back(op);
  });

  return success();
}
} // namespace

//===----------------------------------------------------------------------===//
// PDLMatchHooks
//===----------------------------------------------------------------------===//

void transform::PDLMatchHooks::mergeInPDLMatchHooks(
    toolchain::StringMap<PDLConstraintFunction> &&constraintFns) {
  // Steal the constraint functions from the given map.
  for (auto &it : constraintFns)
    pdlMatchHooks.registerConstraintFunction(it.getKey(), std::move(it.second));
}

const toolchain::StringMap<PDLConstraintFunction> &
transform::PDLMatchHooks::getPDLConstraintHooks() const {
  return pdlMatchHooks.getConstraintFunctions();
}

//===----------------------------------------------------------------------===//
// PDLMatchOp
//===----------------------------------------------------------------------===//

DiagnosedSilenceableFailure
transform::PDLMatchOp::apply(transform::TransformRewriter &rewriter,
                             transform::TransformResults &results,
                             transform::TransformState &state) {
  auto *extension = state.getExtension<PatternApplicatorExtension>();
  assert(extension &&
         "expected PatternApplicatorExtension to be attached by the parent op");
  SmallVector<Operation *> targets;
  for (Operation *root : state.getPayloadOps(getRoot())) {
    if (failed(extension->findAllMatches(
            getPatternName().getLeafReference().getValue(), root, targets))) {
      emitDefiniteFailure()
          << "could not find pattern '" << getPatternName() << "'";
    }
  }
  results.set(toolchain::cast<OpResult>(getResult()), targets);
  return DiagnosedSilenceableFailure::success();
}

void transform::PDLMatchOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  onlyReadsHandle(getRootMutable(), effects);
  producesHandle(getOperation()->getOpResults(), effects);
  onlyReadsPayload(effects);
}

//===----------------------------------------------------------------------===//
// WithPDLPatternsOp
//===----------------------------------------------------------------------===//

DiagnosedSilenceableFailure
transform::WithPDLPatternsOp::apply(transform::TransformRewriter &rewriter,
                                    transform::TransformResults &results,
                                    transform::TransformState &state) {
  TransformOpInterface transformOp = nullptr;
  for (Operation &nested : getBody().front()) {
    if (!isa<pdl::PatternOp>(nested)) {
      transformOp = cast<TransformOpInterface>(nested);
      break;
    }
  }

  state.addExtension<PatternApplicatorExtension>(getOperation());
  toolchain::scope_exit guard(
      [&]() { state.removeExtension<PatternApplicatorExtension>(); });

  auto scope = state.make_region_scope(getBody());
  if (failed(mapBlockArguments(state)))
    return DiagnosedSilenceableFailure::definiteFailure();
  return state.applyTransform(transformOp);
}

void transform::WithPDLPatternsOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  getPotentialTopLevelEffects(effects);
}

LogicalResult transform::WithPDLPatternsOp::verify() {
  Block *body = getBodyBlock();
  Operation *topLevelOp = nullptr;
  for (Operation &op : body->getOperations()) {
    if (isa<pdl::PatternOp>(op))
      continue;

    if (op.hasTrait<::mlir::transform::PossibleTopLevelTransformOpTrait>()) {
      if (topLevelOp) {
        InFlightDiagnostic diag =
            emitOpError() << "expects only one non-pattern op in its body";
        diag.attachNote(topLevelOp->getLoc()) << "first non-pattern op";
        diag.attachNote(op.getLoc()) << "second non-pattern op";
        return diag;
      }
      topLevelOp = &op;
      continue;
    }

    InFlightDiagnostic diag =
        emitOpError()
        << "expects only pattern and top-level transform ops in its body";
    diag.attachNote(op.getLoc()) << "offending op";
    return diag;
  }

  if (auto parent = getOperation()->getParentOfType<WithPDLPatternsOp>()) {
    InFlightDiagnostic diag = emitOpError() << "cannot be nested";
    diag.attachNote(parent.getLoc()) << "parent operation";
    return diag;
  }

  if (!topLevelOp) {
    InFlightDiagnostic diag = emitOpError()
                              << "expects at least one non-pattern op";
    return diag;
  }

  return success();
}
