//===- TokenManager.h - Manage Tokens for syntax-tree ------------*- C++-*-===//
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
// Defines Token interfaces for the clang syntax-tree. This is the level of
// abstraction that the syntax-tree uses to operate on Token.
//
// TokenManager decouples the syntax-tree from a particular token
// implementation. For example, a TokenBuffer captured from a clang parser may
// track macro expansions and associate tokens with clang's SourceManager, while
// a clang pseudoparser would use a flat array of raw-lexed tokens in memory.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_SYNTAX_TOKEN_MANAGER_H
#define LANGUAGE_CORE_TOOLING_SYNTAX_TOKEN_MANAGER_H

#include "toolchain/ADT/StringRef.h"
#include <cstdint>

namespace language::Core {
namespace syntax {

/// Defines interfaces for operating "Token" in the clang syntax-tree.
class TokenManager {
public:
  virtual ~TokenManager() = default;

  /// Describes what the exact class kind of the TokenManager is.
  virtual toolchain::StringLiteral kind() const = 0;

  /// A key to identify a specific token. The token concept depends on the
  /// underlying implementation -- it can be a spelled token from the original
  /// source file or an expanded token.
  /// The syntax-tree Leaf node holds a Key.
  using Key = uintptr_t;
  virtual toolchain::StringRef getText(Key K) const = 0;
};

} // namespace syntax
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_SYNTAX_TOKEN_MANAGER_H
