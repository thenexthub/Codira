//===--- CodiraCompletionInfo.h --------------------------------------------===//
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

#ifndef LANGUAGE_IDE_LANGUAGECOMPLETIONINFO_H
#define LANGUAGE_IDE_LANGUAGECOMPLETIONINFO_H

#include "language/Frontend/Frontend.h"
#include "language/IDE/CodeCompletionContext.h"

namespace language {
namespace ide {

struct CodiraCompletionInfo {
  std::shared_ptr<CompilerInstance> compilerInstance = nullptr;
  CodeCompletionContext *completionContext = nullptr;
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_LANGUAGECOMPLETIONINFO_H
