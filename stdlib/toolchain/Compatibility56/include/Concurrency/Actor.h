//===--- Actor.h - ABI structures for actors --------------------*- C++ -*-===//
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
// Codira ABI describing actors.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_ACTOR_BACKDEPLOY56_H
#define LANGUAGE_ABI_ACTOR_BACKDEPLOY56_H

#include "language/ABI/HeapObject.h"
#include "language/ABI/MetadataValues.h"

namespace language {

/// The default actor implementation.  This is the layout of both
/// the DefaultActor and NSDefaultActor classes.
class alignas(Alignment_DefaultActor) DefaultActor : public HeapObject {
public:
  // These constructors do not initialize the actor instance, and the
  // destructor does not destroy the actor instance; you must call
  // language_defaultActor_{initialize,destroy} yourself.
  constexpr DefaultActor(const HeapMetadata *metadata)
    : HeapObject(metadata), PrivateData{} {}

  constexpr DefaultActor(const HeapMetadata *metadata,
                         InlineRefCounts::Immortal_t immortal)
    : HeapObject(metadata, immortal), PrivateData{} {}

  void *PrivateData[NumWords_DefaultActor];
};

} // end namespace language

#endif // LANGUAGE_ABI_ACTOR_BACKDEPLOY56_H

