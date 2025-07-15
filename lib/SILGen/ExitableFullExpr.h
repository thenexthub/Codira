//===--- ExitableFullExpr.h - An exitable full-expression -------*- C++ -*-===//
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
// This file defines ExitableFullExpr, a cleanup scope RAII object
// that conveniently creates a continuation block.
//
//===----------------------------------------------------------------------===//

#ifndef EXITABLE_FULL_EXPR_H
#define EXITABLE_FULL_EXPR_H

#include "JumpDest.h"
#include "Scope.h"
#include "language/Basic/Assertions.h"

namespace language {
namespace Lowering {

/// A cleanup scope RAII object, like FullExpr, that comes with a
/// JumpDest for a continuation block.
///
/// You *must* call exit() at some point.
///
/// This scope is also exposed to the debug info.
class TOOLCHAIN_LIBRARY_VISIBILITY ExitableFullExpr {
  SILGenFunction &SGF;
  FullExpr Scope;
  JumpDest ExitDest;
public:
  explicit ExitableFullExpr(SILGenFunction &SGF, CleanupLocation loc)
    : SGF(SGF), Scope(SGF.Cleanups, loc),
      ExitDest(SGF.B.splitBlockForFallthrough(),
               SGF.Cleanups.getCleanupsDepth(), loc) {
    SGF.enterDebugScope(loc);
  }
  ~ExitableFullExpr() {
    SGF.leaveDebugScope();
  }


  JumpDest getExitDest() const { return ExitDest; }

  SILBasicBlock *exit() {
    assert(!SGF.B.hasValidInsertionPoint());
    Scope.pop();
    SGF.B.setInsertionPoint(ExitDest.getBlock());
    return ExitDest.getBlock();
  }
};

} // end namespace Lowering
} // end namespace language

#endif
