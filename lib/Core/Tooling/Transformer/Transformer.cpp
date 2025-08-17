//===--- Transformer.cpp - Transformer library implementation ---*- C++ -*-===//
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

#include "language/Core/Tooling/Transformer/Transformer.h"
#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/ASTMatchers/ASTMatchersInternal.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Tooling/Refactoring/AtomicChange.h"
#include "toolchain/Support/Error.h"
#include <map>
#include <utility>

namespace language::Core {
namespace tooling {

using ::language::Core::ast_matchers::MatchFinder;

namespace detail {

void TransformerImpl::onMatch(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  if (Result.Context->getDiagnostics().hasErrorOccurred())
    return;

  onMatchImpl(Result);
}

toolchain::Expected<toolchain::SmallVector<AtomicChange, 1>>
TransformerImpl::convertToAtomicChanges(
    const toolchain::SmallVectorImpl<transformer::Edit> &Edits,
    const MatchFinder::MatchResult &Result) {
  // Group the transformations, by file, into AtomicChanges, each anchored by
  // the location of the first change in that file.
  std::map<FileID, AtomicChange> ChangesByFileID;
  for (const auto &T : Edits) {
    auto ID = Result.SourceManager->getFileID(T.Range.getBegin());
    auto Iter = ChangesByFileID
                    .emplace(ID, AtomicChange(*Result.SourceManager,
                                              T.Range.getBegin(), T.Metadata))
                    .first;
    auto &AC = Iter->second;
    switch (T.Kind) {
    case transformer::EditKind::Range:
      if (auto Err =
              AC.replace(*Result.SourceManager, T.Range, T.Replacement)) {
        return std::move(Err);
      }
      break;
    case transformer::EditKind::AddInclude:
      AC.addHeader(T.Replacement);
      break;
    }
  }

  toolchain::SmallVector<AtomicChange, 1> Changes;
  Changes.reserve(ChangesByFileID.size());
  for (auto &IDChangePair : ChangesByFileID)
    Changes.push_back(std::move(IDChangePair.second));

  return Changes;
}

} // namespace detail

void Transformer::registerMatchers(MatchFinder *MatchFinder) {
  for (auto &Matcher : Impl->buildMatchers())
    MatchFinder->addDynamicMatcher(Matcher, this);
}

void Transformer::run(const MatchFinder::MatchResult &Result) {
  if (Result.Context->getDiagnostics().hasErrorOccurred())
    return;

  Impl->onMatch(Result);
}

} // namespace tooling
} // namespace language::Core
