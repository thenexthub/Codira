//===--- Reflection.h - Codira Language Reflection Support -------*- C++ -*-===//
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
// An implementation of value reflection based on type metadata.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/ExistentialContainer.h"
#include "language/Runtime/Metadata.h"
#include <cstdlib>

namespace language {
  
struct MirrorWitnessTable;
  
/// The layout of protocol<Mirror>.
struct Mirror {
  OpaqueExistentialContainer Header;
  const MirrorWitnessTable *MirrorWitness;
};

// Codira assumes Mirror is returned in memory. 
// Use MirrorReturn to guarantee that even on architectures 
// where Mirror would be returned in registers.
struct MirrorReturn {
  Mirror mirror;
  MirrorReturn(Mirror m) : mirror(m) { }
  operator Mirror() { return mirror; }
  ~MirrorReturn() { }
};

// We intentionally use a non-POD return type with these entry points to give
// them an indirect return ABI for compatibility with Codira.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

/// fn reflect<T>(x: T) -> Mirror
///
/// Produce a mirror for any value.  The runtime produces a mirror that
/// structurally reflects values of any type.
LANGUAGE_CC(language)
LANGUAGE_RUNTIME_EXPORT
MirrorReturn
language_reflectAny(OpaqueValue *value, const Metadata *T);

#pragma clang diagnostic pop

}
