//===--- PreprocessorOutputOptions.h ----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_PREPROCESSOROUTPUTOPTIONS_H
#define LANGUAGE_CORE_FRONTEND_PREPROCESSOROUTPUTOPTIONS_H

#include <toolchain/Support/Compiler.h>

namespace language::Core {

/// PreprocessorOutputOptions - Options for controlling the C preprocessor
/// output (e.g., -E).
class PreprocessorOutputOptions {
public:
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowCPP : 1;            ///< Print normal preprocessed output.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowComments : 1;       ///< Show comments.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowLineMarkers : 1;    ///< Show \#line markers.
  LLVM_PREFERRED_TYPE(bool)
  unsigned UseLineDirectives : 1;   ///< Use \#line instead of GCC-style \# N.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowMacroComments : 1;  ///< Show comments, even in macros.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowMacros : 1;         ///< Print macro definitions.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowIncludeDirectives : 1;  ///< Print includes, imports etc. within preprocessed output.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowEmbedDirectives : 1; ///< Print embeds, etc. within preprocessed
  LLVM_PREFERRED_TYPE(bool)
  unsigned RewriteIncludes : 1;    ///< Preprocess include directives only.
  LLVM_PREFERRED_TYPE(bool)
  unsigned RewriteImports  : 1;    ///< Include contents of transitively-imported modules.
  LLVM_PREFERRED_TYPE(bool)
  unsigned MinimizeWhitespace : 1; ///< Ignore whitespace from input.
  LLVM_PREFERRED_TYPE(bool)
  unsigned DirectivesOnly : 1; ///< Process directives but do not expand macros.
  LLVM_PREFERRED_TYPE(bool)
  unsigned KeepSystemIncludes : 1; ///< Do not expand system headers.

public:
  PreprocessorOutputOptions() {
    ShowCPP = 0;
    ShowComments = 0;
    ShowLineMarkers = 1;
    UseLineDirectives = 0;
    ShowMacroComments = 0;
    ShowMacros = 0;
    ShowIncludeDirectives = 0;
    ShowEmbedDirectives = 0;
    RewriteIncludes = 0;
    RewriteImports = 0;
    MinimizeWhitespace = 0;
    DirectivesOnly = 0;
    KeepSystemIncludes = 0;
  }
};

}  // end namespace language::Core

#endif
