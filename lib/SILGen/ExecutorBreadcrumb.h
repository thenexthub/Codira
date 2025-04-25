//===--- ExecutorBreadcrumb.h - executor hop tracking for SILGen ----------===//
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

#ifndef SWIFT_SILGEN_EXECUTORBREADCRUMB_H
#define SWIFT_SILGEN_EXECUTORBREADCRUMB_H

#include "language/SIL/SILValue.h"

namespace language {

class SILLocation;

namespace Lowering {

class SILGenFunction;

/// Represents the information necessary to return to a caller's own
/// active executor after making a hop to an actor for actor-isolated calls.
class ExecutorBreadcrumb {
  bool mustReturnToExecutor;
  
public:
  // An empty breadcrumb, indicating no hop back is necessary.
  ExecutorBreadcrumb() : mustReturnToExecutor(false) {}
  
  // A breadcrumb representing the need to hop back to the expected
  // executor of the current function.
  explicit ExecutorBreadcrumb(bool mustReturnToExecutor)
    : mustReturnToExecutor(mustReturnToExecutor) {}
  
  // Emits the hop back sequence, if any, necessary to get back to
  // the executor represented by this breadcrumb.
  void emit(SILGenFunction &SGF, SILLocation loc);

#ifndef NDEBUG
  // FOR ASSERTS ONLY: returns true if calling `emit` will emit a hop-back.
  bool needsEmit() const { return mustReturnToExecutor; }
#endif
};

} // namespace Lowering
} // namespace language

#endif
