//===--- Rewriters.h - Rewriter implementations     -------------*- C++ -*-===//
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
//  This header contains miscellaneous utilities for various front-end actions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_REWRITE_FRONTEND_REWRITERS_H
#define LANGUAGE_CORE_REWRITE_FRONTEND_REWRITERS_H

#include "language/Core/Basic/LLVM.h"

namespace language::Core {
class Preprocessor;
class PreprocessorOutputOptions;

/// RewriteMacrosInInput - Implement -rewrite-macros mode.
void RewriteMacrosInInput(Preprocessor &PP, raw_ostream *OS);

/// DoRewriteTest - A simple test for the TokenRewriter class.
void DoRewriteTest(Preprocessor &PP, raw_ostream *OS);

/// RewriteIncludesInInput - Implement -frewrite-includes mode.
void RewriteIncludesInInput(Preprocessor &PP, raw_ostream *OS,
                            const PreprocessorOutputOptions &Opts);

}  // end namespace language::Core

#endif
