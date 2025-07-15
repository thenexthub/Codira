//===--- IRGenSILPasses.cpp - The IRGen Prepare SIL Passes ----------------===//
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
//  This file implements the registration of SILOptimizer passes necessary for
//  IRGen.
//
//===----------------------------------------------------------------------===//

#include "language/IRGen/IRGenSILPasses.h"
#include "language/AST/ASTContext.h"
#include "language/Subsystems.h"

namespace language {
using SILTransformCtor = SILTransform *(void);
static SILTransformCtor *irgenSILPassFns[] = {
#define PASS(Name, Tag, Desc)
#define IRGEN_PASS(Name, Tag, Desc) &irgen::create##Name,
#include "language/SILOptimizer/PassManager/Passes.def"
};
} // end namespace language

void language::registerIRGenSILTransforms(ASTContext &ctx) {
  ctx.registerIRGenSILTransforms(irgenSILPassFns);
}
