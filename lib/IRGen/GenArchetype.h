//===--- GenArchetype.h - Swift IR generation for archetypes ----*- C++ -*-===//
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
//  This file provides the private interface to the archetype emission code.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_GENARCHETYPE_H
#define SWIFT_IRGEN_GENARCHETYPE_H

#include "language/AST/Types.h"
#include "llvm/ADT/STLExtras.h"

namespace llvm {
  class Value;
}

namespace language {
  class AssociatedTypeDecl;
  class ProtocolDecl;
  class SILType;

namespace irgen {
  class Address;
  class IRGenFunction;
  class DynamicMetadataRequest;
  class MetadataResponse;

  using GetTypeParameterInContextFn =
    llvm::function_ref<CanType(CanType type)>;

  /// Emit a type metadata reference for an archetype.
  MetadataResponse emitArchetypeTypeMetadataRef(IRGenFunction &IGF,
                                                CanArchetypeType archetype,
                                                DynamicMetadataRequest request);

  /// Emit a witness table reference.
  llvm::Value *emitArchetypeWitnessTableRef(IRGenFunction &IGF,
                                            CanArchetypeType archetype,
                                            ProtocolDecl *protocol);

  /// Emit a metadata reference for an associated type of an archetype.
  MetadataResponse emitAssociatedTypeMetadataRef(IRGenFunction &IGF,
                                                 CanArchetypeType origin,
                                                 AssociatedTypeDecl *assocType,
                                                 DynamicMetadataRequest request);

  /// Emit a dynamic metatype lookup for the given archetype.
  llvm::Value *emitDynamicTypeOfOpaqueArchetype(IRGenFunction &IGF,
                                                Address archetypeAddr,
                                                SILType archetypeType);
  
  /// Emit a lookup for an opaque result type's metadata.
  MetadataResponse emitOpaqueTypeMetadataRef(IRGenFunction &IGF,
                                             CanOpaqueTypeArchetypeType archetype,
                                             DynamicMetadataRequest request);
  /// Emit a lookup for an opaque result type's protocol conformance.
  llvm::Value *emitOpaqueTypeWitnessTableRef(IRGenFunction &IGF,
                                             CanOpaqueTypeArchetypeType archetype,
                                             ProtocolDecl *protocol);
} // end namespace irgen
} // end namespace language

#endif
