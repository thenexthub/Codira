//===--- RuntimeEffect.h - Defines the RuntimeEffect enum -------*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_RUNTIMEIMPACT_H
#define LANGUAGE_SIL_RUNTIMEIMPACT_H

namespace language {

/// Defines the effect of runtime functions.
enum class RuntimeEffect : unsigned {
  /// The runtime function has no observable effect.
  NoEffect            = 0,
  
  /// The runtime function can lock.
  Locking             = 0x01,

  /// The runtime function can allocate memory (implies Locking).
  Allocating          = 0x02,

  /// The runtime function can deallocate memory (implies Locking).
  Deallocating        = 0x04,
  
  /// The runtime function performs ARC operations (implies Locking).
  RefCounting         = 0x08,
  
  /// The runtime function performs metadata operations (which implies Locking
  /// and Allocating).
  /// TODO: distinguish between metadata runtime functions which only lock and
  ///       which also can allocate.
  MetaData            = 0x10,

  /// The runtime function performs dynamic casting (which implies Locking
  /// and Allocating).
  /// TODO: distinguish between casting runtime functions which only lock and
  ///       which also can allocate.
  Casting             = 0x20,

  /// The runtime function performs exclusivity checking.
  /// This does not have any observable runtime effect like locking or
  /// allocation, but it's modelled separately.
  ExclusivityChecking = 0x100,

  /// The runtime function calls ObjectiveC methods.
  ObjectiveC          = 0x40,
  
  /// Witness methods, boxing, unboxing, initializing, etc.
  Existential         = 0x80,

  /// Class-bound only existential
  ExistentialClassBound = 0x200,
  
  /// Not modelled currently.
  Concurrency         = 0x0,

  /// Not modelled currently.
  AutoDiff            = 0x0,
  
  Releasing = RefCounting | Deallocating,
};

inline RuntimeEffect operator|(RuntimeEffect lhs, RuntimeEffect rhs) {
  return (RuntimeEffect)((unsigned)lhs | (unsigned)rhs);
}

inline RuntimeEffect &operator|=(RuntimeEffect &lhs, RuntimeEffect rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline bool operator&(RuntimeEffect lhs, RuntimeEffect rhs) {
  return ((unsigned)lhs & (unsigned)rhs) != 0;
}

} // end language namespace

namespace RuntimeConstants {
  const auto NoEffect = language::RuntimeEffect::NoEffect;
  const auto Locking = language::RuntimeEffect::Locking;
  const auto Allocating = language::RuntimeEffect::Allocating;
  const auto Deallocating = language::RuntimeEffect::Deallocating;
  const auto RefCounting = language::RuntimeEffect::RefCounting;
  const auto ObjectiveC = language::RuntimeEffect::ObjectiveC;
  const auto Concurrency = language::RuntimeEffect::Concurrency;
  const auto AutoDiff = language::RuntimeEffect::AutoDiff;
  const auto MetaData = language::RuntimeEffect::MetaData;
  const auto Casting = language::RuntimeEffect::Casting;
  const auto ExclusivityChecking = language::RuntimeEffect::ExclusivityChecking;
}

// Enable the following macro to perform validation check on the runtime effects
// of instructions in IRGen.
// #define CHECK_RUNTIME_EFFECT_ANALYSIS

#endif
