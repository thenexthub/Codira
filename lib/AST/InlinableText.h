//===------ InlinableText.h - Extract inlinable source text -----*- C++ -*-===//
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

#ifndef SWIFT_AST_INLINABLETEXT_H
#define SWIFT_AST_INLINABLETEXT_H

#include "language/AST/ASTNode.h"
#include "language/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"

namespace language {
class ASTContext;

/// Extracts the text of this ASTNode from the source buffer, ignoring
/// all #if declarations and clauses except the elements that are active.
StringRef extractInlinableText(ASTContext &ctx, ASTNode node,
                               SmallVectorImpl<char> &scratch);

} // end namespace language

#endif // defined(SWIFT_AST_INLINABLETEXT_H)
