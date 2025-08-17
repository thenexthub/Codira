//===--- RefactoringResultConsumer.h - Clang refactoring library ----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGRESULTCONSUMER_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGRESULTCONSUMER_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Tooling/Refactoring/AtomicChange.h"
#include "language/Core/Tooling/Refactoring/Rename/SymbolOccurrences.h"
#include "toolchain/Support/Error.h"

namespace language::Core {
namespace tooling {

/// An abstract interface that consumes the various refactoring results that can
/// be produced by refactoring actions.
///
/// A valid refactoring result must be handled by a \c handle method.
class RefactoringResultConsumer {
public:
  virtual ~RefactoringResultConsumer() {}

  /// Handles an initiation or an invication error. An initiation error typically
  /// has a \c DiagnosticError payload that describes why initiation failed.
  virtual void handleError(toolchain::Error Err) = 0;

  /// Handles the source replacements that are produced by a refactoring action.
  virtual void handle(AtomicChanges SourceReplacements) {
    defaultResultHandler();
  }

  /// Handles the symbol occurrences that are found by an interactive
  /// refactoring action.
  virtual void handle(SymbolOccurrences Occurrences) { defaultResultHandler(); }

private:
  void defaultResultHandler() {
    handleError(toolchain::make_error<toolchain::StringError>(
        "unsupported refactoring result", toolchain::inconvertibleErrorCode()));
  }
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGRESULTCONSUMER_H
