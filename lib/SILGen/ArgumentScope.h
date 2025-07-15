//===--- ArgumentScope.h ----------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SILGEN_ARGUMENTSCOPE_H
#define LANGUAGE_SILGEN_ARGUMENTSCOPE_H

#include "FormalEvaluation.h"
#include "SILGenFunction.h"
#include "Scope.h"

namespace language {
namespace Lowering {

/// A scope that is created before arguments are emitted for an apply and is
/// popped manually immediately after the raw apply is emitted and before the
/// raw results are marshalled into managed resources.
///
/// Internally this creates a Scope and a FormalEvaluationScope. It ensures that
/// they are destroyed/initialized in the appropriate order.
class ArgumentScope {
  Scope normalScope;
  FormalEvaluationScope formalEvalScope;
  SILLocation loc;

public:
  ArgumentScope(SILGenFunction &SGF, SILLocation loc)
      : normalScope(SGF.Cleanups, CleanupLocation(loc)),
        formalEvalScope(SGF), loc(loc) {}

  ~ArgumentScope() {
    if (normalScope.isValid() || !formalEvalScope.isPopped()) {
      toolchain_unreachable("ArgumentScope that wasn't popped?!");
    }
  }

  ArgumentScope() = delete;
  ArgumentScope(const ArgumentScope &) = delete;
  ArgumentScope &operator=(const ArgumentScope &) = delete;

  ArgumentScope(ArgumentScope &&other) = delete;
  ArgumentScope &operator=(ArgumentScope &&other) = delete;

  void pop() { popImpl(); }

  /// Pop the formal evaluation and argument scopes preserving the value mv.
  ManagedValue popPreservingValue(ManagedValue mv);

  // Pop the formal evaluation and argument scopes, preserving rv.
  RValue popPreservingValue(RValue &&rv);

  void verify() {
    formalEvalScope.verify();
  }

  bool isValid() const { return normalScope.isValid(); }

private:
  void popImpl() {
    // We must always pop the formal eval scope before the normal scope since
    // the formal eval scope may have pointers into the normal scope.
    normalScope.verify();
    formalEvalScope.pop();
    normalScope.pop();
  }
};

} // end namespace Lowering
} // end namespace language

#endif
