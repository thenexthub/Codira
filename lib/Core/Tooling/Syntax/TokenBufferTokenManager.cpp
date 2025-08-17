//===- TokenBufferTokenManager.cpp ----------------------------------------===//
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

#include "language/Core/Tooling/Syntax/TokenBufferTokenManager.h"

namespace language::Core {
namespace syntax {
constexpr toolchain::StringLiteral syntax::TokenBufferTokenManager::Kind;

std::pair<FileID, ArrayRef<syntax::Token>>
syntax::TokenBufferTokenManager::lexBuffer(
    std::unique_ptr<toolchain::MemoryBuffer> Input) {
  auto FID = SM.createFileID(std::move(Input));
  auto It = ExtraTokens.try_emplace(FID, tokenize(FID, SM, LangOpts));
  assert(It.second && "duplicate FileID");
  return {FID, It.first->second};
}

} // namespace syntax
} // namespace language::Core
