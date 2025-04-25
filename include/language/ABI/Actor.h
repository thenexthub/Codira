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
// Swift ABI describing actors.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_ABI_ACTOR_H
#define SWIFT_ABI_ACTOR_H

#include "language/ABI/HeapObject.h"
#include "language/ABI/MetadataValues.h"

// lldb knows about some of these internals. If you change things that lldb
// knows about (or might know about in the future, as a future lldb might be
// inspecting a process running an older Swift runtime), increment
// _swift_concurrency_debug_internal_layout_version and add a comment describing
// the new version.

namespace language {

/// The default actor implementation.  This is the layout of both
/// the DefaultActor and NSDefaultActor classes.
class alignas(Alignment_DefaultActor) DefaultActor : public HeapObject {
public:
  // These constructors do not initialize the actor instance, and the
  // destructor does not destroy the actor instance; you must call
  // swift_defaultActor_{initialize,destroy} yourself.
  constexpr DefaultActor(const HeapMetadata *metadata)
    : HeapObject(metadata), PrivateData{} {}

  constexpr DefaultActor(const HeapMetadata *metadata,
                         InlineRefCounts::Immortal_t immortal)
    : HeapObject(metadata, immortal), PrivateData{} {}

  void *PrivateData[NumWords_DefaultActor];
};

/// The non-default distributed actor implementation.
class alignas(Alignment_NonDefaultDistributedActor) NonDefaultDistributedActor : public HeapObject {
public:
  // These constructors do not initialize the actor instance, and the
  // destructor does not destroy the actor instance; you must call
  // swift_nonDefaultDistributedActor_initialize yourself.
  constexpr NonDefaultDistributedActor(const HeapMetadata *metadata)
    : HeapObject(metadata), PrivateData{} {}

  constexpr NonDefaultDistributedActor(const HeapMetadata *metadata,
                                       InlineRefCounts::Immortal_t immortal)
    : HeapObject(metadata, immortal), PrivateData{} {}

  void *PrivateData[NumWords_NonDefaultDistributedActor];
};

} // end namespace language

#endif
