//===--- CodeCompletionOrganizer.h - ----------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_CODECOMPLETIONORGANIZER_H
#define TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_CODECOMPLETIONORGANIZER_H

#include "CodeCompletion.h"
#include "SourceKit/Core/LangSupport.h"
//#include "language/IDE/CodeCompletionContext.h"
#include "language/IDE/CodiraCompletionInfo.h"
#include "toolchain/ADT/StringMap.h"

namespace language {
class CompilerInvocation;
}

namespace SourceKit {

  using TypeContextKind = language::ide::CodeCompletionContext::TypeContextKind;

namespace CodeCompletion {

struct Options {
  bool sortByName = false;
  bool useImportDepth = true;
  bool groupOverloads = false;
  bool groupStems = false;
  bool includeExactMatch = true;
  bool addInnerResults = false;
  bool addInnerOperators = true;
  bool addInitsToTopLevel = false;
  bool hideUnderscores = true;
  bool reallyHideAllUnderscores = false;
  bool hideLowPriority = true;
  bool hideByNameStyle = true;
  bool fuzzyMatching = true;
  bool annotatedDescription = false;
  bool includeObjectLiterals = true;
  bool addCallWithNoDefaultArgs = true;
  unsigned minFuzzyLength = 2;
  unsigned showTopNonLiteralResults = 3;

  // Options for combining priorities.
  unsigned semanticContextWeight = 7;
  unsigned fuzzyMatchWeight = 13;
  unsigned popularityBonus = 2;
};

typedef toolchain::StringMap<PopularityFactor> NameToPopularityMap;

std::vector<Completion *>
extendCompletions(ArrayRef<CodiraResult *> languageResults, CompletionSink &sink,
                  language::ide::CodiraCompletionInfo &info,
                  const NameToPopularityMap *nameToPopularity,
                  const Options &options, Completion *prefix = nullptr,
                  bool clearFlair = false);

bool addCustomCompletions(CompletionSink &sink,
                          std::vector<Completion *> &completions,
                          ArrayRef<CustomCompletionInfo> customCompletions,
                          CompletionKind completionKind);

class CodeCompletionOrganizer {
  class Impl;
  Impl &impl;
  const Options &options;
public:
  CodeCompletionOrganizer(const Options &options, CompletionKind kind,
                          TypeContextKind typeContextKind);
  ~CodeCompletionOrganizer();

  static void
  preSortCompletions(toolchain::MutableArrayRef<Completion *> completions);

  /// Add \p completions to the organizer, removing any results that don't match
  /// \p filterText and returning \p exactMatch if there is an exact match.
  ///
  /// Precondition: \p completions should be sorted with preSortCompletions().
  void addCompletionsWithFilter(ArrayRef<Completion *> completions,
                                StringRef filterText, const FilterRules &rules,
                                Completion *&exactMatch);

  void groupAndSort(const Options &options);

  /// Finishes the results and returns them.
  /// For convenience, this returns a shared_ptr, but it is uniquely referenced.
  CodeCompletionViewRef takeResultsView();
};

} // end namespace CodeCompletion
} // end namespace SourceKit

#endif // TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_CODECOMPLETIONORGANIZER_H
