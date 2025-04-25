//===--- GenPoly.cpp - Swift IR Generation for Polymorphism ---------------===//
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
//  This file implements IR generation for polymorphic operations in Swift.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/ASTVisitor.h"
#include "language/AST/Types.h"
#include "language/AST/Decl.h"
#include "language/AST/GenericEnvironment.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILType.h"
#include "llvm/IR/DerivedTypes.h"

#include "Explosion.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "LoadableTypeInfo.h"
#include "TypeVisitor.h"
#include "GenTuple.h"
#include "GenPoly.h"
#include "GenType.h"

using namespace language;
using namespace irgen;

static SILType applyPrimaryArchetypes(IRGenFunction &IGF,
                                      SILType type) {
  if (!type.hasTypeParameter()) {
    return type;
  }

  auto substType =
    IGF.IGM.getGenericEnvironment()->mapTypeIntoContext(type.getASTType())
      ->getCanonicalType();
  return SILType::getPrimitiveType(substType, type.getCategory());
}

/// Given a substituted explosion, re-emit it as an unsubstituted one.
///
/// For example, given an explosion which begins with the
/// representation of an (Int, Float), consume that and produce the
/// representation of an (Int, T).
///
/// The substitutions must carry origTy to substTy.
void irgen::reemitAsUnsubstituted(IRGenFunction &IGF,
                                  SILType expectedTy, SILType substTy,
                                  Explosion &in, Explosion &out) {
  expectedTy = applyPrimaryArchetypes(IGF, expectedTy);

  ExplosionSchema expectedSchema;
  cast<LoadableTypeInfo>(IGF.IGM.getTypeInfo(expectedTy))
    .getSchema(expectedSchema);

#ifndef NDEBUG
  auto &substTI = IGF.IGM.getTypeInfo(applyPrimaryArchetypes(IGF, substTy));
  assert(expectedSchema.size() ==
         cast<LoadableTypeInfo>(substTI).getExplosionSize());
#endif

  for (ExplosionSchema::Element &elt : expectedSchema) {
    llvm::Value *value = in.claimNext();
    assert(elt.isScalar());

    // The only type differences we expect here should be due to
    // substitution of class archetypes.
    if (value->getType() != elt.getScalarType()) {
      value = IGF.Builder.CreateBitCast(value, elt.getScalarType(),
                                        value->getName() + ".asUnsubstituted");
    }
    out.add(value);
  }
}
