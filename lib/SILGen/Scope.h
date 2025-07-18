//===--- Scope.h - Declarations for scope RAII objects ----------*- C++ -*-===//
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
// This file defines the Scope and FullExpr RAII objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILGEN_SCOPE_H
#define LANGUAGE_SILGEN_SCOPE_H

#include "SILGenFunction.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILDebugScope.h"
#include "Cleanup.h"

namespace language {
namespace Lowering {

/// A Scope is a RAII object recording that a scope (e.g. a brace
/// statement) has been entered.
class TOOLCHAIN_LIBRARY_VISIBILITY Scope {
  CleanupManager &cleanups;
  CleanupsDepth depth;
  Scope *savedInnermostScope;
  CleanupLocation loc;

  friend class CleanupManager;

public:
  explicit Scope(CleanupManager &cleanups, CleanupLocation loc)
      : cleanups(cleanups), depth(cleanups.getCleanupsDepth()),
        savedInnermostScope(cleanups.innermostScope), loc(loc) {
    assert(depth.isValid());
    cleanups.innermostScope = this;
    if (savedInnermostScope)
      cleanups.stack.checkIterator(savedInnermostScope->depth);
  }

  Scope(const Scope &other) = delete;
  Scope &operator=(const Scope &other) = delete;

  Scope(Scope &&other)
      : cleanups(other.cleanups), depth(other.depth),
        savedInnermostScope(other.savedInnermostScope), loc(other.loc) {
    // Invalidate other.
    other.depth = CleanupsDepth::invalid();
  }

  Scope &operator=(Scope &&other) {
    depth = other.depth;
    savedInnermostScope = other.savedInnermostScope;
    loc = other.loc;

    // Invalidate other.
    other.depth = CleanupsDepth::invalid();

    return *this;
  }

  explicit Scope(SILGenFunction &SGF, SILLocation loc)
      : Scope(SGF.Cleanups, CleanupLocation(loc)) {}

  void pop() {
    assert(depth.isValid() && "popping a scope twice!");
    popImpl();
    depth = CleanupsDepth::invalid();
  }
  
  ~Scope() {
    if (depth.isValid())
      popImpl();
  }

  /// Verify that the invariants of this scope still hold.
  void verify();

  bool isValid() const { return depth.isValid(); }

  /// Pop the scope pushing the +1 ManagedValue through the scope. Asserts if mv
  /// is a plus zero managed value.
  ManagedValue popPreservingValue(ManagedValue mv);

  /// Pop this scope pushing the +1 rvalue through the scope. Asserts if rv is a
  /// plus zero rvalue.
  RValue popPreservingValue(RValue &&rv);

private:
  /// Internal private implementation of popImpl so we can use it in Scope::pop
  /// and in Scope's destructor.
  void popImpl();
};

/// A scope that must be manually popped by the using code. If not
/// popped, the destructor asserts.
class TOOLCHAIN_LIBRARY_VISIBILITY AssertingManualScope {
  Scope scope;

public:
  explicit AssertingManualScope(CleanupManager &cleanups, CleanupLocation loc)
      : scope(cleanups, loc) {}

  AssertingManualScope(AssertingManualScope &&other)
      : scope(std::move(other.scope)) {}

  AssertingManualScope &operator=(AssertingManualScope &&other) {
    scope = std::move(other.scope);
    return *this;
  }

  ~AssertingManualScope() {
    assert(!scope.isValid() && "Unpopped manual scope?!");
  }

  void pop() && { scope.pop(); }
};

/// A FullExpr is a RAII object recording that a full-expression has
/// been entered.  A full-expression is essentially a very small scope
/// for the temporaries in an expression, with the added complexity
/// that (eventually, very likely) we have to deal with expressions
/// that are only conditionally evaluated.
class TOOLCHAIN_LIBRARY_VISIBILITY FullExpr : private Scope {
public:
  explicit FullExpr(CleanupManager &cleanups, CleanupLocation loc)
      : Scope(cleanups, loc) {}
  using Scope::pop;
};

/// A LexicalScope is a Scope that is also exposed to the debug info.
class TOOLCHAIN_LIBRARY_VISIBILITY LexicalScope : private Scope {
  SILGenFunction& SGF;
public:
  explicit LexicalScope(SILGenFunction &SGF, CleanupLocation loc)
      : Scope(SGF.Cleanups, loc), SGF(SGF) {
    SGF.enterDebugScope(loc);
  }
  using Scope::pop;

  ~LexicalScope() {
    SGF.leaveDebugScope();
  }
};

/// A scope that only exists in the debug info.
class TOOLCHAIN_LIBRARY_VISIBILITY DebugScope {
  SILGenFunction &SGF;

public:
  explicit DebugScope(SILGenFunction &SGF, SILLocation loc) : SGF(SGF) {
    SGF.enterDebugScope(loc);
  }

  ~DebugScope() { SGF.leaveDebugScope(); }
};

} // end namespace Lowering
} // end namespace language

#endif
