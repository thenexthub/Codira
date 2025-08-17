//===----- CodeCompletion.h - Code Completion for ClangRepl ---===//
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
// This file defines the classes which performs code completion at the REPL.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INTERPRETER_CODE_COMPLETION_H
#define LANGUAGE_CORE_INTERPRETER_CODE_COMPLETION_H
#include <string>
#include <vector>

namespace toolchain {
class StringRef;
} // namespace toolchain

namespace language::Core {
class CodeCompletionResult;
class CompilerInstance;

struct ReplCodeCompleter {
  ReplCodeCompleter() = default;
  std::string Prefix;

  /// \param InterpCI [in] The compiler instance that is used to trigger code
  /// completion

  /// \param Content [in] The string where code completion is triggered.

  /// \param Line [in] The line number of the code completion point.

  /// \param Col [in] The column number of the code completion point.

  /// \param ParentCI [in] The running interpreter compiler instance that
  /// provides ASTContexts.

  /// \param CCResults [out] The completion results.
  void codeComplete(CompilerInstance *InterpCI, toolchain::StringRef Content,
                    unsigned Line, unsigned Col,
                    const CompilerInstance *ParentCI,
                    std::vector<std::string> &CCResults);
};
} // namespace language::Core
#endif
