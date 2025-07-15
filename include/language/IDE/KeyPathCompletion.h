//===--- KeyPathCompletion.h ----------------------------------------------===//
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

#ifndef LANGUAGE_IDE_KEYPATHCOMPLETION_H
#define LANGUAGE_IDE_KEYPATHCOMPLETION_H

#include "language/IDE/CodeCompletionConsumer.h"
#include "language/IDE/CodeCompletionContext.h"
#include "language/IDE/TypeCheckCompletionCallback.h"

namespace language {
namespace ide {

class KeyPathTypeCheckCompletionCallback : public TypeCheckCompletionCallback {
  struct Result {
    /// The type on which completion should occur, i.e. a result type of the
    /// previous component.
    Type BaseType;
    /// Whether code completion happens on the key path's root.
    bool OnRoot;
  };

  KeyPathExpr *KeyPath;
  SmallVector<Result, 4> Results;

  void sawSolutionImpl(const constraints::Solution &solution) override;

public:
  KeyPathTypeCheckCompletionCallback(KeyPathExpr *KeyPath) : KeyPath(KeyPath) {}

  void collectResults(DeclContext *DC, SourceLoc DotLoc,
                      ide::CodeCompletionContext &CompletionCtx);
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_KEYPATHCOMPLETION_H
