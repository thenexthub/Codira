//===- Mutations.h - mutate syntax trees --------------------*- C++ ---*-=====//
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
// Defines high-level APIs for transforming syntax trees and producing the
// corresponding textual replacements.
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_TOOLING_SYNTAX_MUTATIONS_H
#define LANGUAGE_CORE_TOOLING_SYNTAX_MUTATIONS_H

#include "language/Core/Tooling/Core/Replacement.h"
#include "language/Core/Tooling/Syntax/Nodes.h"
#include "language/Core/Tooling/Syntax/TokenBufferTokenManager.h"
#include "language/Core/Tooling/Syntax/Tree.h"

namespace language::Core {
namespace syntax {

/// Computes textual replacements required to mimic the tree modifications made
/// to the syntax tree.
tooling::Replacements computeReplacements(const TokenBufferTokenManager &TBTM,
                                          const syntax::TranslationUnit &TU);

/// Removes a statement or replaces it with an empty statement where one is
/// required syntactically. E.g., in the following example:
///     if (cond) { foo(); } else bar();
/// One can remove `foo();` completely and to remove `bar();` we would need to
/// replace it with an empty statement.
/// EXPECTS: S->canModify() == true
void removeStatement(syntax::Arena &A, TokenBufferTokenManager &TBTM,
                     syntax::Statement *S);

} // namespace syntax
} // namespace language::Core

#endif
