//===--- SILDebugScope.h - DebugScopes for SIL code -------------*- C++ -*-===//
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
///
/// \file
///
/// This file defines a container for scope information used to
/// generate debug info.
///
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SIL_DEBUGSCOPE_H
#define SWIFT_SIL_DEBUGSCOPE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"
#include "language/SIL/SILAllocated.h"
#include "language/SIL/SILLocation.h"

namespace language {

class SILDebugLocation;
class SILDebugScope;
class SILFunction;
class SILInstruction;

/// This class stores a lexical scope as it is represented in the
/// debug info. In contrast to LLVM IR, SILDebugScope also holds all
/// the inlining information. In LLVM IR the inline info is part of
/// DILocation.
class SILDebugScope : public SILAllocated<SILDebugScope> {
public:
  /// The AST node this lexical scope represents.
  SILLocation Loc;
  /// Always points to the parent lexical scope.
  /// For top-level scopes, this is the SILFunction.
  PointerUnion<const SILDebugScope *, SILFunction *> Parent;
  /// An optional chain of inlined call sites.
  ///
  /// If this scope is inlined, this points to a special "scope" that
  /// holds the location of the call site.
  const SILDebugScope *InlinedCallSite;

  SILDebugScope(SILLocation Loc, SILFunction *SILFn,
                const SILDebugScope *ParentScope = nullptr,
                const SILDebugScope *InlinedCallSite = nullptr);

  /// Create a scope for an artificial function.
  SILDebugScope(SILLocation Loc);

  SILLocation getLoc() const { return Loc; }

  /// Return the function this scope originated from before being inlined.
  SILFunction *getInlinedFunction() const;

  /// Return the parent function of this scope. If the scope was
  /// inlined this recursively returns the function it was inlined
  /// into.
  SILFunction *getParentFunction() const;

  /// If this is a debug scope associated with an inlined call site, return the
  /// SILLocation associated with the call site resulting from the final
  /// inlining.
  ///
  /// This allows one to emit diagnostics based off of inlined code's final
  /// location in the function that was inlined into.
  SILLocation getOutermostInlineLocation() const {
    if (!InlinedCallSite)
      return SILLocation::invalid();

    auto *scope = this;
    do {
      scope = scope->InlinedCallSite;
    } while (scope->InlinedCallSite);

    SILLocation callSite = scope->Loc;
    if (callSite.isNull() || !callSite.isASTNode())
      return SILLocation::invalid();

    return callSite;
  }

#ifndef NDEBUG
  void print(SourceManager &SM, llvm::raw_ostream &OS = llvm::errs(),
             unsigned Indent = 0) const;

  void print(SILModule &Mod) const;
#endif
};

/// Determine whether an instruction may not have a SILDebugScope.
bool maybeScopeless(const SILInstruction &inst);

/// Knows how to make a deep copy of a debug scope.
class ScopeCloner {
  llvm::SmallDenseMap<const SILDebugScope *,
                      const SILDebugScope *> ClonedScopeCache;
  SILFunction &NewFn;
public:
  /// ScopeCloner expects NewFn to be a clone of the original
  /// function, with all debug scopes and locations still pointing to
  /// the original function.
  ScopeCloner(SILFunction &NewFn);
  /// Return a (cached) deep copy of a scope.
  const SILDebugScope *getOrCreateClonedScope(const SILDebugScope *OrigScope);
};

}

#endif
