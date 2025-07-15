//===--- ADContext.cpp - Differentiation Context --------------*- C++ -*---===//
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
//
// Per-module contextual information for the differentiation transform.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "differentiation"

#include "language/SILOptimizer/Differentiation/ADContext.h"
#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/Assertions.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using toolchain::DenseMap;
using toolchain::SmallPtrSet;
using toolchain::SmallVector;

namespace language {
namespace autodiff {

//===----------------------------------------------------------------------===//
// Local helpers
//===----------------------------------------------------------------------===//

/// Given an operator name, such as '+', and a protocol, returns the '+'
/// operator. If the operator does not exist in the protocol, returns null.
static FuncDecl *findOperatorDeclInProtocol(DeclName operatorName,
                                            ProtocolDecl *protocol) {
  assert(operatorName.isOperator());
  // Find the operator requirement in the given protocol declaration.
  auto opLookup = protocol->lookupDirect(operatorName);
  for (auto *decl : opLookup) {
    if (!decl->isProtocolRequirement())
      continue;
    auto *fd = dyn_cast<FuncDecl>(decl);
    if (!fd || !fd->isStatic() || !fd->isOperator())
      continue;
    return fd;
  }
  // Not found.
  return nullptr;
}

//===----------------------------------------------------------------------===//
// ADContext methods
//===----------------------------------------------------------------------===//

ADContext::ADContext(SILModuleTransform &transform)
    : transform(transform), module(*transform.getModule()),
      passManager(*transform.getPassManager()) {}

/// Get the source file for the given `SILFunction`.
static SourceFile &getSourceFile(SILFunction *f) {
  if (f->hasLocation())
    if (auto *declContext = f->getLocation().getAsDeclContext())
      if (auto *parentSourceFile = declContext->getParentSourceFile())
        return *parentSourceFile;
  for (auto *file : f->getModule().getCodiraModule()->getFiles())
    if (auto *sourceFile = dyn_cast<SourceFile>(file))
      return *sourceFile;
  toolchain_unreachable("Could not resolve SourceFile from SILFunction");
}

SynthesizedFileUnit &
ADContext::getOrCreateSynthesizedFile(SILFunction *original) {
  auto &SF = getSourceFile(original);
  return SF.getOrCreateSynthesizedFile();
}

FuncDecl *ADContext::getPlusDecl() const {
  if (!cachedPlusFn) {
    cachedPlusFn = findOperatorDeclInProtocol(astCtx.getIdentifier("+"),
                                              additiveArithmeticProtocol);
    assert(cachedPlusFn && "AdditiveArithmetic.+ not found");
  }
  return cachedPlusFn;
}

FuncDecl *ADContext::getPlusEqualDecl() const {
  if (!cachedPlusEqualFn) {
    cachedPlusEqualFn = findOperatorDeclInProtocol(astCtx.getIdentifier("+="),
                                                   additiveArithmeticProtocol);
    assert(cachedPlusEqualFn && "AdditiveArithmetic.+= not found");
  }
  return cachedPlusEqualFn;
}

AccessorDecl *ADContext::getAdditiveArithmeticZeroGetter() const {
  if (cachedZeroGetter)
    return cachedZeroGetter;
  auto zeroDeclLookup = getAdditiveArithmeticProtocol()
      ->lookupDirect(getASTContext().Id_zero);
  auto *zeroDecl = cast<VarDecl>(zeroDeclLookup.front());
  assert(zeroDecl->isProtocolRequirement());
  cachedZeroGetter = zeroDecl->getOpaqueAccessor(AccessorKind::Get);
  return cachedZeroGetter;
}

void ADContext::cleanUp() {
  // Delete all references to generated functions.
  for (auto fnRef : generatedFunctionReferences) {
    if (auto *fnRefInst =
            peerThroughFunctionConversions<FunctionRefInst>(fnRef)) {
      fnRefInst->replaceAllUsesWithUndef();
      fnRefInst->eraseFromParent();
    }
  }
  // Delete all generated functions.
  for (auto *generatedFunction : generatedFunctions) {
    TOOLCHAIN_DEBUG(getADDebugStream() << "Deleting generated function "
                                  << generatedFunction->getName() << '\n');
    generatedFunction->dropAllReferences();
    transform.notifyWillDeleteFunction(generatedFunction);
    module.eraseFunction(generatedFunction);
  }
}

DifferentiableFunctionInst *ADContext::createDifferentiableFunction(
    SILBuilder &builder, SILLocation loc, IndexSubset *parameterIndices,
    IndexSubset *resultIndices, SILValue original,
    std::optional<std::pair<SILValue, SILValue>> derivativeFunctions) {
  auto *dfi = builder.createDifferentiableFunction(
      loc, parameterIndices, resultIndices, original, derivativeFunctions);
  processedDifferentiableFunctionInsts.erase(dfi);
  return dfi;
}

LinearFunctionInst *ADContext::createLinearFunction(
    SILBuilder &builder, SILLocation loc, IndexSubset *parameterIndices,
    SILValue original, std::optional<SILValue> transposeFunction) {
  auto *lfi = builder.createLinearFunction(loc, parameterIndices, original,
                                           transposeFunction);
  processedLinearFunctionInsts.erase(lfi);
  return lfi;
}

DifferentiableFunctionExpr *
ADContext::findDifferentialOperator(DifferentiableFunctionInst *inst) {
  return inst->getLoc().getAsASTNode<DifferentiableFunctionExpr>();
}

LinearFunctionExpr *
ADContext::findDifferentialOperator(LinearFunctionInst *inst) {
  return inst->getLoc().getAsASTNode<LinearFunctionExpr>();
}

} // end namespace autodiff
} // end namespace language
