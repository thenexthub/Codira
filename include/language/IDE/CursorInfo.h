//===--- CursorInfo.h --- ---------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_IDE_CURSORINFO_H
#define LANGUAGE_IDE_CURSORINFO_H

#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"
#include "language/IDE/Utils.h"

namespace language {
class IDEInspectionCallbacksFactory;

namespace ide {

/// An abstract base class for consumers of context info results.
class CursorInfoConsumer {
public:
  virtual ~CursorInfoConsumer() {}
  virtual void handleResults(std::vector<ResolvedCursorInfoPtr>) = 0;
};

/// Create a factory for code completion callbacks.
IDEInspectionCallbacksFactory *
makeCursorInfoCallbacksFactory(CursorInfoConsumer &Consumer,
                               SourceLoc RequestedLoc);

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_CURSORINFO_H
