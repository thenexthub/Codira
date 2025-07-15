//===--- CodeCompletionConsumer.h -----------------------------------------===//
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

#ifndef LANGUAGE_IDE_CODECOMPLETIONCONSUMER
#define LANGUAGE_IDE_CODECOMPLETIONCONSUMER

#include "language/IDE/CodeCompletionContext.h"
#include "language/Parse/IDEInspectionCallbacks.h"

namespace language {
namespace ide {

struct RequestedCachedModule;

/// An abstract base class for consumers of code completion results.
class CodeCompletionConsumer {
public:
  virtual ~CodeCompletionConsumer() {}
  /// Clients should override this method to receive \p Results.
  virtual void handleResults(CodeCompletionContext &context) = 0;
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_CODECOMPLETIONCONSUMER
