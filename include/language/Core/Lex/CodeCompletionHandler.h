//===--- CodeCompletionHandler.h - Preprocessor code completion -*- C++ -*-===//
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
//
//  This file defines the CodeCompletionHandler interface, which provides
//  code-completion callbacks for the preprocessor.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_LEX_CODECOMPLETIONHANDLER_H
#define LANGUAGE_CORE_LEX_CODECOMPLETIONHANDLER_H

#include "toolchain/ADT/StringRef.h"

namespace language::Core {

class IdentifierInfo;
class MacroInfo;

/// Callback handler that receives notifications when performing code
/// completion within the preprocessor.
class CodeCompletionHandler {
public:
  virtual ~CodeCompletionHandler();

  /// Callback invoked when performing code completion for a preprocessor
  /// directive.
  ///
  /// This callback will be invoked when the preprocessor processes a '#' at the
  /// start of a line, followed by the code-completion token.
  ///
  /// \param InConditional Whether we're inside a preprocessor conditional
  /// already.
  virtual void CodeCompleteDirective(bool InConditional) { }

  /// Callback invoked when performing code completion within a block of
  /// code that was excluded due to preprocessor conditionals.
  virtual void CodeCompleteInConditionalExclusion() { }

  /// Callback invoked when performing code completion in a context
  /// where the name of a macro is expected.
  ///
  /// \param IsDefinition Whether this is the definition of a macro, e.g.,
  /// in a \#define.
  virtual void CodeCompleteMacroName(bool IsDefinition) { }

  /// Callback invoked when performing code completion in a preprocessor
  /// expression, such as the condition of an \#if or \#elif directive.
  virtual void CodeCompletePreprocessorExpression() { }

  /// Callback invoked when performing code completion inside a
  /// function-like macro argument.
  ///
  /// There will be another callback invocation after the macro arguments are
  /// parsed, so this callback should generally be used to note that the next
  /// callback is invoked inside a macro argument.
  virtual void CodeCompleteMacroArgument(IdentifierInfo *Macro,
                                         MacroInfo *MacroInfo,
                                         unsigned ArgumentIndex) { }

  /// Callback invoked when performing code completion inside the filename
  /// part of an #include directive. (Also #import, #include_next, etc).
  /// \p Dir is the directory relative to the include path.
  virtual void CodeCompleteIncludedFile(toolchain::StringRef Dir, bool IsAngled) {}

  /// Callback invoked when performing code completion in a part of the
  /// file where we expect natural language, e.g., a comment, string, or
  /// \#error directive.
  virtual void CodeCompleteNaturalLanguage() { }
};

}

#endif // LANGUAGE_CORE_LEX_CODECOMPLETIONHANDLER_H
