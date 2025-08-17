//===---- CodeCompleteOptions.h - Code Completion Options -------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_SEMA_CODECOMPLETEOPTIONS_H
#define LANGUAGE_CORE_SEMA_CODECOMPLETEOPTIONS_H

#include "toolchain/Support/Compiler.h"

namespace language::Core {

/// Options controlling the behavior of code completion.
class CodeCompleteOptions {
public:
  /// Show macros in code completion results.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeMacros : 1;

  /// Show code patterns in code completion results.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeCodePatterns : 1;

  /// Show top-level decls in code completion results.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeGlobals : 1;

  /// Show decls in namespace (including the global namespace) in code
  /// completion results. If this is 0, `IncludeGlobals` will be ignored.
  ///
  /// Currently, this only works when completing qualified IDs (i.e.
  /// `Sema::CodeCompleteQualifiedId`).
  /// FIXME: consider supporting more completion cases with this option.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeNamespaceLevelDecls : 1;

  /// Show brief documentation comments in code completion results.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeBriefComments : 1;

  /// Hint whether to load data from the external AST to provide full results.
  /// If false, namespace-level declarations and macros from the preamble may be
  /// omitted.
  LLVM_PREFERRED_TYPE(bool)
  unsigned LoadExternal : 1;

  /// Include results after corrections (small fix-its), e.g. change '.' to '->'
  /// on member access, etc.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeFixIts : 1;

  CodeCompleteOptions()
      : IncludeMacros(0), IncludeCodePatterns(0), IncludeGlobals(1),
        IncludeNamespaceLevelDecls(1), IncludeBriefComments(0),
        LoadExternal(1), IncludeFixIts(0) {}
};

} // namespace language::Core

#endif

