//===--- GenericArguments.h - IR generation for metadata requests ---------===//
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
//  This file implements IR generation for accessing metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENERICARGUMENTS_H
#define LANGUAGE_IRGEN_GENERICARGUMENTS_H

#include "Explosion.h"
#include "FixedTypeInfo.h"
#include "GenArchetype.h"
#include "GenClass.h"
#include "GenMeta.h"
#include "GenProto.h"
#include "GenType.h"
#include "GenericRequirement.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenMangler.h"
#include "IRGenModule.h"
#include "MetadataRequest.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ExistentialLayout.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/SubstitutionMap.h"
#include "language/ClangImporter/ClangModule.h"
#include "language/IRGen/Linking.h"
#include "toolchain/ADT/STLExtras.h"

namespace language {
namespace irgen {

/// A structure for collecting generic arguments for emitting a
/// nominal metadata reference.  The structure produced here is
/// consumed by language_getGenericMetadata() and must correspond to
/// the fill operations that the compiler emits for the bound decl.
struct GenericArguments {
  /// The values to use to initialize the arguments structure.
  SmallVector<toolchain::Value *, 8> Values;
  SmallVector<toolchain::Type *, 8> Types;

 void collectTypes(IRGenModule &IGM, NominalTypeDecl *nominal) {
    GenericTypeRequirements requirements(IGM, nominal);
    collectTypes(IGM, requirements);
  }

  void collectTypes(IRGenModule &IGM,
                    const GenericTypeRequirements &requirements) {
    for (auto &requirement : requirements.getRequirements()) {
      Types.push_back(requirement.getType(IGM));
    }
  }

  void collect(IRGenFunction &IGF, CanType type) {
    auto subs = type->getContextSubstitutionMap();
    collect(IGF, subs);
  }

  void collect(IRGenFunction &IGF, SubstitutionMap subs) {
    GenericTypeRequirements requirements(IGF.IGM, subs.getGenericSignature());

    for (auto requirement : requirements.getRequirements()) {
      Values.push_back(emitGenericRequirementFromSubstitutions(
          IGF, requirement, MetadataState::Abstract, subs));
    }

    collectTypes(IGF.IGM, requirements);
    assert(Types.size() == Values.size());
  }
};

} // namespace irgen
} // namespace language

#endif
