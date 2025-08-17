//===--- IndexingAction.h - Frontend index action ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_INDEX_INDEXINGACTION_H
#define LANGUAGE_CORE_INDEX_INDEXINGACTION_H

#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Index/IndexingOptions.h"
#include "language/Core/Lex/PPCallbacks.h"
#include "language/Core/Lex/Preprocessor.h"
#include "toolchain/ADT/ArrayRef.h"
#include <memory>

namespace language::Core {
  class ASTContext;
  class ASTConsumer;
  class ASTReader;
  class ASTUnit;
  class Decl;
  class FrontendAction;

namespace serialization {
  class ModuleFile;
}

namespace index {
class IndexDataConsumer;

/// Creates an ASTConsumer that indexes all symbols (macros and AST decls).
std::unique_ptr<ASTConsumer>
createIndexingASTConsumer(std::shared_ptr<IndexDataConsumer> DataConsumer,
                          const IndexingOptions &Opts,
                          std::shared_ptr<Preprocessor> PP);

std::unique_ptr<ASTConsumer> createIndexingASTConsumer(
    std::shared_ptr<IndexDataConsumer> DataConsumer,
    const IndexingOptions &Opts, std::shared_ptr<Preprocessor> PP,
    // Prefer to set Opts.ShouldTraverseDecl and use the above overload.
    // This version is only needed if used to *track* function body parsing.
    std::function<bool(const Decl *)> ShouldSkipFunctionBody);

/// Creates a frontend action that indexes all symbols (macros and AST decls).
std::unique_ptr<FrontendAction>
createIndexingAction(std::shared_ptr<IndexDataConsumer> DataConsumer,
                     const IndexingOptions &Opts);

/// Recursively indexes all decls in the AST.
void indexASTUnit(ASTUnit &Unit, IndexDataConsumer &DataConsumer,
                  IndexingOptions Opts);

/// Recursively indexes \p Decls.
void indexTopLevelDecls(ASTContext &Ctx, Preprocessor &PP,
                        ArrayRef<const Decl *> Decls,
                        IndexDataConsumer &DataConsumer, IndexingOptions Opts);

/// Creates a PPCallbacks that indexes macros and feeds macros to \p Consumer.
/// The caller is responsible for calling `Consumer.setPreprocessor()`.
std::unique_ptr<PPCallbacks> indexMacrosCallback(IndexDataConsumer &Consumer,
                                                 IndexingOptions Opts);

/// Recursively indexes all top-level decls in the module.
void indexModuleFile(serialization::ModuleFile &Mod, ASTReader &Reader,
                     IndexDataConsumer &DataConsumer, IndexingOptions Opts);

} // namespace index
} // namespace language::Core

#endif
