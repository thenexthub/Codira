//===--- Actor.h - Codira concurrency actor declarations ---------*- C++ -*-===//
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
// Actor-related declarations that are also used by Remote Mirror.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CONCURRENCY_ACTOR_H
#define LANGUAGE_CONCURRENCY_ACTOR_H

#include <stdint.h>

namespace language {
namespace concurrency {
namespace ActorFlagConstants {

enum : uint32_t {
  // Bits 0-2: Actor state
  //
  // Possible state transitions for an actor:
  //
  // Idle -> Running
  // Running -> Idle
  // Running -> Scheduled
  // Scheduled -> Running
  // Idle -> Deallocated
  // Running -> Zombie_ReadyForDeallocation
  // Zombie_ReadyForDeallocation -> Deallocated
  //
  // It is possible for an actor to be in Running and yet completely released
  // by clients. However, the actor needs to be kept alive until it is done
  // executing the task that is running on it and gives it up. It is only
  // after that we can safely deallocate it.
  ActorStateMask = 0x7,

  /// The actor is not currently scheduled.  Completely redundant
  /// with the job list being empty.
  Idle = 0x0,
  /// There actor is scheduled
  Scheduled = 0x1,
  /// There is currently a thread running the actor.
  Running = 0x2,
  /// The actor is ready for deallocation once it stops running
  Zombie_ReadyForDeallocation = 0x3,

  // Bit 3 is free

  // Bit 4
  IsPriorityEscalated = 0x10,

  // Bits 8 - 15. We only need 8 bits of the whole size_t to represent Job
  // Priority
  PriorityMask = 0xFF00,
  PriorityAndOverrideMask = PriorityMask | IsPriorityEscalated,
  PriorityShift = 0x8,
};

} // namespace ActorFlagConstants
} // namespace concurrency
} // namespace language

#endif // LANGUAGE_CONCURRENCY_ACTOR_H
