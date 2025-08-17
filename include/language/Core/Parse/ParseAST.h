//===--- ParseAST.h - Define the ParseAST method ----------------*- C++ -*-===//
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
//  This file defines the language::Core::ParseAST method.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_PARSE_PARSEAST_H
#define LANGUAGE_CORE_PARSE_PARSEAST_H

#include "language/Core/Basic/LangOptions.h"

namespace language::Core {
  class Preprocessor;
  class ASTConsumer;
  class ASTContext;
  class CodeCompleteConsumer;
  class Sema;

  /// Parse the entire file specified, notifying the ASTConsumer as
  /// the file is parsed.
  ///
  /// This operation inserts the parsed decls into the translation
  /// unit held by Ctx.
  ///
  /// \param PrintStats Whether to print LLVM statistics related to parsing.
  /// \param TUKind The kind of translation unit being parsed.
  /// \param CompletionConsumer If given, an object to consume code completion
  /// results.
  /// \param SkipFunctionBodies Whether to skip parsing of function bodies.
  /// This option can be used, for example, to speed up searches for
  /// declarations/definitions when indexing.
  void ParseAST(Preprocessor &pp, ASTConsumer *C,
                ASTContext &Ctx, bool PrintStats = false,
                TranslationUnitKind TUKind = TU_Complete,
                CodeCompleteConsumer *CompletionConsumer = nullptr,
                bool SkipFunctionBodies = false);

  /// Parse the main file known to the preprocessor, producing an
  /// abstract syntax tree.
  void ParseAST(Sema &S, bool PrintStats = false,
                bool SkipFunctionBodies = false);

}  // end namespace language::Core

#endif
