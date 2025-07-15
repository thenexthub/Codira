//===--- NestedSemanticFunctionCheck.cpp - Disallow nested @_semantic -----===//
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
///
/// This pass raises a diagnostic error if any semantic functions in the current
/// module are improperly nested. A semantic function has an @_semantic
/// attribute. Semantic functions may call other semantic functions, but
/// semantic and non-semantic call frames may not be interleaved.
///
/// @_semantics(...)
/// fn funcA() {
///   funcB()
/// }
/// // Error: funcB is called by a semantic function and calls
/// // a semantic function.
/// fn funcB() {
///   funcC()
/// }
/// @_semantics(...)
/// fn funcC() {}
///
/// For the pass pipeline to function well, @_semantic calls need to be
/// processed top-down, while inlining proceeds bottom-up. With proper nesting,
/// this is easily accomplished, but with improper nesting, semantic function
/// can be prematurely "hidden".
///
/// For simplicity, don't bother checking whether semantic functions in one
/// module call non-semantic functions in a different module. Really,
/// "optimizable" @_semantics should only exist in the standard library.
///
/// TODO: The @_semantics tag should only ever be used to convey function
/// semantics that are important to optimizations and which cannot be determined
/// from the function body. Make sure this tag is never used for compiler hints
/// or informational attributes, which do not require special pass pipeline and
/// module serialization behavior.
///
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticsSIL.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/PerformanceInlinerUtils.h"

using namespace language;

namespace {

class NestedSemanticFunctionCheck : public SILFunctionTransform {
  // Mark all functions that may indirectly call a semantic function, ignoring
  // simple wrappers, but are not themselves an optimizable semantic function.
  //
  // This could be a very large set. It is built gradually during bottom-up
  // function transforms, then deleted once all functions are processed before
  // executing the next pass pipeline.
  toolchain::SmallPtrSet<SILFunction *, 8> mayCallSemanticFunctions;

  // Mark semantic functions and wrappers around semantic calls.
  toolchain::SmallPtrSet<SILFunction *, 8> semanticFunctions;

public:
  void run() override;

protected:
  void checkSemanticFunction(SILFunction *f);
};

} // end anonymous namespace

/// This is a semantic function. Diagnose any calls to mayCallSemanticFunctions.
void NestedSemanticFunctionCheck::checkSemanticFunction(SILFunction *f) {
  for (auto &bb : *f) {
    for (auto &i : bb) {
      auto apply = FullApplySite::isa(&i);
      if (!apply) {
        continue;
      }
      auto callee = apply.getReferencedFunctionOrNull();
      if (!callee) {
        continue;
      }
      // Semantic function calling non-semantic function with improperly
      // nested semantic calls underneath.
      if (mayCallSemanticFunctions.count(callee)) {
        ASTContext &astContext = f->getASTContext();
        astContext.Diags.diagnose(apply.getLoc().getSourceLoc(),
                                  diag::semantic_function_improper_nesting);
      }
    }
  }
}

// If this is a semantic function, diagnose it immediately. Otherwise propagate
// the mayCallSemanticFunctions and semanticCallWrappers sets upward in the call
// tree.
void NestedSemanticFunctionCheck::run() {
  auto *f = getFunction();
  if (!semanticFunctions.count(f) && isOptimizableSemanticFunction(f)) {
    semanticFunctions.insert(f);
    checkSemanticFunction(f);
    return;
  }
  // Add this function to mayCallSemanticFunctions if needed.
  if (mayCallSemanticFunctions.count(f)) {
    return;
  }
  for (auto &bb : *f) {
    for (auto &i : bb) {
      auto apply = FullApplySite::isa(&i);
      if (!apply) {
        continue;
      }
      // If this is a trivial wrapper around a semantic call, don't add it
      // immediately to the mayCallSemanticFunctions set, but add it to
      // semanticCallWrappers so that its caller will be added to
      // mayCallSemanticFunctions.
      if (isNestedSemanticCall(apply)) {
        semanticFunctions.insert(f);
        continue;
      }
      auto callee = apply.getReferencedFunctionOrNull();
      if (!callee) {
        continue;
      }
      // Propagate mayCallSemanticFunctions.
      if (mayCallSemanticFunctions.count(callee)
          || semanticFunctions.count(callee)) {
        mayCallSemanticFunctions.insert(f);
      }
    }
  }
}

SILTransform *language::createNestedSemanticFunctionCheck() {
  return new NestedSemanticFunctionCheck();
}
