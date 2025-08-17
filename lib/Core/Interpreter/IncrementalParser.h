//===--- IncrementalParser.h - Incremental Compilation ----------*- C++ -*-===//
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
// This file implements the class which performs incremental code compilation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_INTERPRETER_INCREMENTALPARSER_H
#define LANGUAGE_CORE_LIB_INTERPRETER_INCREMENTALPARSER_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Error.h"

#include <list>
#include <memory>

namespace language::Core {
class ASTConsumer;
class CodeGenerator;
class CompilerInstance;
class Parser;
class Sema;
class TranslationUnitDecl;

/// Provides support for incremental compilation. Keeps track of the state
/// changes between the subsequent incremental input.
///
class IncrementalParser {
protected:
  /// The Sema performing the incremental compilation.
  Sema &S;

  /// Parser.
  std::unique_ptr<Parser> P;

  /// Consumer to process the produced top level decls. Owned by Act.
  ASTConsumer *Consumer = nullptr;

  /// Counts the number of direct user input lines that have been parsed.
  unsigned InputCount = 0;

  // IncrementalParser();

public:
  IncrementalParser(CompilerInstance &Instance, toolchain::Error &Err);
  virtual ~IncrementalParser();

  /// Parses incremental input by creating an in-memory file.
  ///\returns a \c PartialTranslationUnit which holds information about the
  /// \c TranslationUnitDecl.
  virtual toolchain::Expected<TranslationUnitDecl *> Parse(toolchain::StringRef Input);

  void CleanUpPTU(TranslationUnitDecl *MostRecentTU);

private:
  toolchain::Expected<TranslationUnitDecl *> ParseOrWrapTopLevelDecl();
};
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_INTERPRETER_INCREMENTALPARSER_H
