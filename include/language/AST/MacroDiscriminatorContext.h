//===--- MacroDiscriminatorContext.h - Macro Discriminators -----*- C++ -*-===//
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

#ifndef LANGUAGE_AST_MACRO_DISCRIMINATOR_CONTEXT_H
#define LANGUAGE_AST_MACRO_DISCRIMINATOR_CONTEXT_H

#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "toolchain/ADT/PointerUnion.h"

namespace language {

/// Describes the context of a macro expansion for the purpose of
/// computing macro expansion discriminators.
struct MacroDiscriminatorContext
    : public toolchain::PointerUnion<DeclContext *, FreestandingMacroExpansion *> {
  using PointerUnion::PointerUnion;

  static MacroDiscriminatorContext getParentOf(FreestandingMacroExpansion *expansion);
  static MacroDiscriminatorContext getParentOf(
      SourceLoc loc, DeclContext *origDC
  );

  /// Return the innermost declaration context that is suitable for
  /// use in identifying a macro.
  static DeclContext *getInnermostMacroContext(DeclContext *dc);
};

}

#endif // LANGUAGE_AST_MACRO_DISCRIMINATOR_CONTEXT_H
