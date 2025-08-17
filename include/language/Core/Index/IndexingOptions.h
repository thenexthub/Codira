//===--- IndexingOptions.h - Options for indexing ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_INDEX_INDEXINGOPTIONS_H
#define LANGUAGE_CORE_INDEX_INDEXINGOPTIONS_H

#include "language/Core/Frontend/FrontendOptions.h"
#include <memory>
#include <string>

namespace language::Core {
class Decl;
namespace index {

struct IndexingOptions {
  enum class SystemSymbolFilterKind {
    None,
    DeclarationsOnly,
    All,
  };

  SystemSymbolFilterKind SystemSymbolFilter =
      SystemSymbolFilterKind::DeclarationsOnly;
  bool IndexFunctionLocals = false;
  bool IndexImplicitInstantiation = false;
  bool IndexMacros = true;
  // Whether to index macro definitions in the Preprocessor when preprocessor
  // callback is not available (e.g. after parsing has finished). Note that
  // macro references are not available in Preprocessor.
  bool IndexMacrosInPreprocessor = false;
  // Has no effect if IndexFunctionLocals are false.
  bool IndexParametersInDeclarations = false;
  bool IndexTemplateParameters = false;

  // If set, skip indexing inside some declarations for performance.
  // This prevents traversal, so skipping a struct means its declaration an
  // members won't be indexed, but references elsewhere to that struct will be.
  // Currently this is only checked for top-level declarations.
  std::function<bool(const Decl *)> ShouldTraverseDecl;
};

} // namespace index
} // namespace language::Core

#endif // LANGUAGE_CORE_INDEX_INDEXINGOPTIONS_H
