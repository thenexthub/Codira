//===--- ExistentialContainer.h -------------------------------------------===//
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

#ifndef LANGUAGE_RUNTIME_EXISTENTIALCONTAINER_H
#define LANGUAGE_RUNTIME_EXISTENTIALCONTAINER_H

#include "language/Runtime/Metadata.h"

namespace language {

/// The basic layout of an opaque (non-class-bounded) existential type.
template <typename Runtime>
struct TargetOpaqueExistentialContainer {
  TargetValueBuffer<Runtime> Buffer;
  ConstTargetMetadataPointer<Runtime, TargetMetadata> Type;

  const TargetWitnessTable<Runtime> **getWitnessTables() {
    return reinterpret_cast<const TargetWitnessTable<Runtime> **>(this + 1);
  }

  const TargetWitnessTable<Runtime> *const *getWitnessTables() const {
    return reinterpret_cast<const TargetWitnessTable<Runtime> *const *>(this +
                                                                        1);
  }

  void copyTypeInto(language::TargetOpaqueExistentialContainer<Runtime> *dest,
                    unsigned numTables) const {
    dest->Type = Type;
    for (unsigned i = 0; i != numTables; ++i)
      dest->getWitnessTables()[i] = getWitnessTables()[i];
  }

  /// Return true if this opaque existential container contains a value that is
  /// stored inline in the container. Returns false if the value is stored out
  /// of line.
  bool isValueInline() const;

  /// Project out a pointer to the value stored in the container.
  ///
  /// *NOTE* If the container contains the value inline, then this will return a
  /// pointer inside the container itself. Otherwise, it will return a pointer
  /// to out of line memory.
  const OpaqueValue *projectValue() const;

  /// Cleans up an existential container instance whose value is uninitialized.
  void deinit();

#ifndef NDEBUG
  /// Verify invariants of the container.
  ///
  /// We verify that:
  ///
  /// 1. The container itself is in live memory.
  /// 2. If we have an out of line value, that the value is in live memory.
  ///
  /// The intention is that this is used in combination with ASAN or Guard
  /// Malloc to catch use-after-frees.
  void verify() const;

  /// Dump information about this specific box and its contents. Only intended
  /// for use in the debugger.
  [[deprecated("Only meant for use in the debugger")]] void dump() const;
#endif
};
using OpaqueExistentialContainer = TargetOpaqueExistentialContainer<InProcess>;

/// The basic layout of a class-bounded existential type.
template <typename ContainedValue>
struct ClassExistentialContainerImpl {
  ContainedValue Value;

  const WitnessTable **getWitnessTables() {
    return reinterpret_cast<const WitnessTable **>(this + 1);
  }
  const WitnessTable *const *getWitnessTables() const {
    return reinterpret_cast<const WitnessTable *const *>(this + 1);
  }

  void copyTypeInto(ClassExistentialContainerImpl *dest,
                    unsigned numTables) const {
    for (unsigned i = 0; i != numTables; ++i)
      dest->getWitnessTables()[i] = getWitnessTables()[i];
  }
};
using ClassExistentialContainer = ClassExistentialContainerImpl<void *>;
using WeakClassExistentialContainer =
    ClassExistentialContainerImpl<WeakReference>;
using UnownedClassExistentialContainer =
    ClassExistentialContainerImpl<UnownedReference>;

} // end language namespace

#endif
