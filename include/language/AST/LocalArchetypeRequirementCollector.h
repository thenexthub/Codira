//===--- LocalArchetypeRequirementCollector.h -------------------*- C++ -*-===//
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
// This file has utility code for extending a generic signature with opened
// existentials and shape classes.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_LOCAL_ARCHETYPE_REQUIREMENT_COLLECTOR_H
#define SWIFT_AST_LOCAL_ARCHETYPE_REQUIREMENT_COLLECTOR_H

#include "language/AST/ASTContext.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Requirement.h"
#include "language/AST/Types.h"

namespace language {

struct LocalArchetypeRequirementCollector {
  const ASTContext &Context;
  GenericSignature OuterSig;
  unsigned Depth;

  /// The lists of new parameters and requirements to add to the signature.
  SmallVector<GenericTypeParamType *, 2> Params;
  SmallVector<Requirement, 2> Requirements;

  LocalArchetypeRequirementCollector(const ASTContext &ctx, GenericSignature sig);

  void addOpenedExistential(Type constraint);
  void addOpenedElement(CanGenericTypeParamType shapeClass);

  GenericTypeParamType *addParameter();
};

struct MapLocalArchetypesOutOfContext {
  GenericSignature baseGenericSig;
  ArrayRef<GenericEnvironment *> capturedEnvs;

  MapLocalArchetypesOutOfContext(GenericSignature baseGenericSig,
                                 ArrayRef<GenericEnvironment *> capturedEnvs)
      : baseGenericSig(baseGenericSig), capturedEnvs(capturedEnvs) {}

  Type getInterfaceType(Type interfaceTy, GenericEnvironment *genericEnv) const;
  Type operator()(SubstitutableType *type) const;
};

Type mapLocalArchetypesOutOfContext(Type type,
                                    GenericSignature baseGenericSig,
                                    ArrayRef<GenericEnvironment *> capturedEnvs);

struct MapIntoLocalArchetypeContext {
  GenericEnvironment *baseGenericEnv;
  ArrayRef<GenericEnvironment *> capturedEnvs;

  MapIntoLocalArchetypeContext(GenericEnvironment *baseGenericEnv,
                               ArrayRef<GenericEnvironment *> capturedEnvs)
      : baseGenericEnv(baseGenericEnv), capturedEnvs(capturedEnvs) {}

  Type operator()(SubstitutableType *type) const;
};

GenericSignature buildGenericSignatureWithCapturedEnvironments(
    ASTContext &ctx,
    GenericSignature sig,
    ArrayRef<GenericEnvironment *> capturedEnvs);

SubstitutionMap buildSubstitutionMapWithCapturedEnvironments(
    SubstitutionMap baseSubMap,
    GenericSignature genericSigWithCaptures,
    ArrayRef<GenericEnvironment *> capturedEnvs);

}

#endif // SWIFT_AST_LOCAL_ARCHETYPE_REQUIREMENT_COLLECTOR_H