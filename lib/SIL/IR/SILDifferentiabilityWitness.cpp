//===--- SILDifferentiabilityWitness.cpp - Differentiability witnesses ----===//
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

#define DEBUG_TYPE "sil-differentiability-witness"

#include "language/AST/ASTMangler.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILDifferentiabilityWitness.h"
#include "language/SIL/SILModule.h"

using namespace language;

SILDifferentiabilityWitness *SILDifferentiabilityWitness::createDeclaration(
    SILModule &module, SILLinkage linkage, SILFunction *originalFunction,
    DifferentiabilityKind kind, IndexSubset *parameterIndices,
    IndexSubset *resultIndices, GenericSignature derivativeGenSig,
    const DeclAttribute *attribute) {
  auto *diffWitness = new (module) SILDifferentiabilityWitness(
      module, linkage, originalFunction, kind, parameterIndices, resultIndices,
      derivativeGenSig, /*jvp*/ nullptr, /*vjp*/ nullptr,
      /*isDeclaration*/ true, /*isSerialized*/ false, attribute);
  // Register the differentiability witness in the module.
  Mangle::ASTMangler mangler(module.getASTContext());
  auto mangledKey = mangler.mangleSILDifferentiabilityWitness(
      diffWitness->getOriginalFunction()->getName(),
      diffWitness->getKind(), diffWitness->getConfig());
  assert(!module.DifferentiabilityWitnessMap.count(mangledKey) &&
         "Cannot create duplicate differentiability witness in a module");
  module.DifferentiabilityWitnessMap[mangledKey] = diffWitness;
  module.DifferentiabilityWitnessesByFunction[originalFunction->getName()]
      .push_back(diffWitness);
  module.getDifferentiabilityWitnessList().push_back(diffWitness);
  return diffWitness;
}

SILDifferentiabilityWitness *SILDifferentiabilityWitness::createDefinition(
    SILModule &module, SILLinkage linkage, SILFunction *originalFunction,
    DifferentiabilityKind kind, IndexSubset *parameterIndices,
    IndexSubset *resultIndices, GenericSignature derivativeGenSig,
    SILFunction *jvp, SILFunction *vjp, bool isSerialized,
    const DeclAttribute *attribute) {
  auto *diffWitness = new (module) SILDifferentiabilityWitness(
      module, linkage, originalFunction, kind, parameterIndices, resultIndices,
      derivativeGenSig, jvp, vjp, /*isDeclaration*/ false, isSerialized,
      attribute);
  // Register the differentiability witness in the module.
  Mangle::ASTMangler mangler(module.getASTContext());
  auto mangledKey = mangler.mangleSILDifferentiabilityWitness(
      diffWitness->getOriginalFunction()->getName(),
      diffWitness->getKind(), diffWitness->getConfig());
  assert(!module.DifferentiabilityWitnessMap.count(mangledKey) &&
         "Cannot create duplicate differentiability witness in a module");
  module.DifferentiabilityWitnessMap[mangledKey] = diffWitness;
  module.DifferentiabilityWitnessesByFunction[originalFunction->getName()]
      .push_back(diffWitness);
  module.getDifferentiabilityWitnessList().push_back(diffWitness);
  return diffWitness;
}

void SILDifferentiabilityWitness::convertToDefinition(SILFunction *jvp,
                                                      SILFunction *vjp,
                                                      bool isSerialized) {
  assert(IsDeclaration);
  IsDeclaration = false;
  JVP = jvp;
  VJP = vjp;
  IsSerialized = isSerialized;
}

SILDifferentiabilityWitnessKey SILDifferentiabilityWitness::getKey() const {
  return {getOriginalFunction()->getName(), getKind(), getConfig()};
}
