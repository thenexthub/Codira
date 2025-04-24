//===--- GenDiffWitness.cpp - IRGen for differentiability witnesses -------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2020 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file implements IR generation for SIL differentiability witnesses.
//
//===----------------------------------------------------------------------===//

#include "language/AST/PrettyStackTrace.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILDifferentiabilityWitness.h"

#include "ConstantBuilder.h"
#include "IRGenModule.h"

using namespace language;
using namespace irgen;

void IRGenModule::emitSILDifferentiabilityWitness(
    SILDifferentiabilityWitness *dw) {
  PrettyStackTraceDifferentiabilityWitness _st(
      "emitting differentiability witness for", dw->getKey());
  // Don't emit declarations.
  if (dw->isDeclaration())
    return;
  // Don't emit `public_external` witnesses.
  if (dw->getLinkage() == SILLinkage::PublicExternal)
    return;
  ConstantInitBuilder builder(*this);
  auto diffWitnessContents = builder.beginStruct();
  assert(dw->getJVP() &&
         "Differentiability witness definition should have JVP");
  assert(dw->getVJP() &&
         "Differentiability witness definition should have VJP");
  diffWitnessContents.add(getAddrOfSILFunction(dw->getJVP(), NotForDefinition));
  diffWitnessContents.add(getAddrOfSILFunction(dw->getVJP(), NotForDefinition));
  getAddrOfDifferentiabilityWitness(
      dw, diffWitnessContents.finishAndCreateFuture());
}
