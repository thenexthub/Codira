//===--- MatchFilePath.cpp - Match file path with pattern -------*- C++ -*-===//
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
///
/// \file
/// This file implements the functionality of matching a file path name to
/// a pattern, similar to the POSIX fnmatch() function.
///
//===----------------------------------------------------------------------===//

#include "MatchFilePath.h"

using namespace toolchain;

namespace language::Core {
namespace format {

// Check whether `FilePath` matches `Pattern` based on POSIX 2.13.1, 2.13.2, and
// Rule 1 of 2.13.3.
bool matchFilePath(StringRef Pattern, StringRef FilePath) {
  assert(!Pattern.empty());
  assert(!FilePath.empty());

  const auto FilePathBack = FilePath.back();

  // No match if `Pattern` ends with a non-meta character not equal to the last
  // character of `FilePath`.
  if (const auto C = Pattern.back(); !strchr("?*]", C) && C != FilePathBack)
    return false;

  constexpr auto Separator = '/';
  const auto EOP = Pattern.size();  // End of `Pattern`.
  const auto End = FilePath.size(); // End of `FilePath`.
  unsigned I = 0;                   // Index to `Pattern`.

  for (unsigned J = 0; J < End; ++J) {
    if (I == EOP)
      return false;

    switch (const auto F = FilePath[J]; Pattern[I]) {
    case '\\':
      if (++I == EOP || F != Pattern[I])
        return false;
      break;
    case '?':
      if (F == Separator)
        return false;
      break;
    case '*': {
      bool Globstar = I == 0 || Pattern[I - 1] == Separator;
      int StarCount = 1;
      for (; ++I < EOP && Pattern[I] == '*'; ++StarCount) {
        // Skip consecutive stars.
      }
      if (StarCount != 2)
        Globstar = false;
      const auto K = FilePath.find(Separator, J); // Index of next `Separator`.
      const bool NoMoreSeparatorsInFilePath = K == StringRef::npos;
      if (I == EOP) // `Pattern` ends with a star.
        return Globstar || NoMoreSeparatorsInFilePath;
      if (Pattern[I] != Separator) {
        // `Pattern` ends with a lone backslash.
        if (Pattern[I] == '\\' && ++I == EOP)
          return false;
        Globstar = false;
      }
      // The star is followed by a (possibly escaped) `Separator`.
      if (Pattern[I] == Separator) {
        if (!Globstar) {
          if (NoMoreSeparatorsInFilePath)
            return false;
          J = K; // Skip to next `Separator` in `FilePath`.
          break;
        }
        if (++I == EOP)
          return FilePathBack == Separator;
      }
      // Recurse.
      for (auto Pat = Pattern.substr(I);
           J < End && (Globstar || FilePath[J] != Separator); ++J) {
        if (matchFilePath(Pat, FilePath.substr(J)))
          return true;
      }
      return false;
    }
    case '[':
      // Skip e.g. `[!]`.
      if (I + 3 < EOP || (I + 3 == EOP && Pattern[I + 1] != '!')) {
        // Skip unpaired `[`, brackets containing slashes, and `[]`.
        if (const auto K = Pattern.find_first_of("]/", I + 1);
            K != StringRef::npos && Pattern[K] == ']' && K > I + 1) {
          if (F == Separator)
            return false;
          ++I; // After the `[`.
          bool Negated = false;
          if (Pattern[I] == '!') {
            Negated = true;
            ++I; // After the `!`.
          }
          bool Match = false;
          do {
            if (I + 2 < K && Pattern[I + 1] == '-') {
              Match = Pattern[I] <= F && F <= Pattern[I + 2];
              I += 3; // After the range, e.g. `A-Z`.
            } else {
              Match = F == Pattern[I++];
            }
          } while (!Match && I < K);
          if (Negated ? Match : !Match)
            return false;
          I = K + 1; // After the `]`.
          continue;
        }
      }
      [[fallthrough]]; // Match `[` literally.
    default:
      if (F != Pattern[I])
        return false;
    }

    ++I;
  }

  // Match trailing stars with null strings.
  while (I < EOP && Pattern[I] == '*')
    ++I;

  return I == EOP;
}

} // namespace format
} // namespace language::Core
