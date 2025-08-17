//===--- SymbolOccurrences.h - Clang refactoring library ------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_SYMBOLOCCURRENCES_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_SYMBOLOCCURRENCES_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include <vector>

namespace language::Core {
namespace tooling {

class SymbolName;

/// An occurrence of a symbol in the source.
///
/// Occurrences can have difference kinds, that describe whether this occurrence
/// is an exact semantic match, or whether this is a weaker textual match that's
/// not guaranteed to represent the exact declaration.
///
/// A single occurrence of a symbol can span more than one source range. For
/// example, Objective-C selectors can contain multiple argument labels:
///
/// \code
/// [object selectorPiece1: ... selectorPiece2: ...];
/// //      ^~~ range 0 ~~      ^~~ range 1 ~~
/// \endcode
///
/// We have to replace the text in both range 0 and range 1 when renaming the
/// Objective-C method 'selectorPiece1:selectorPiece2'.
class SymbolOccurrence {
public:
  enum OccurrenceKind {
    /// This occurrence is an exact match and can be renamed automatically.
    ///
    /// Note:
    /// Symbol occurrences in macro arguments that expand to different
    /// declarations get marked as exact matches, and thus the renaming engine
    /// will rename them e.g.:
    ///
    /// \code
    ///   #define MACRO(x) x + ns::x
    ///   int foo(int var) {
    ///     return MACRO(var); // var is renamed automatically here when
    ///                        // either var or ns::var is renamed.
    ///   };
    /// \endcode
    ///
    /// The user will have to fix their code manually after performing such a
    /// rename.
    /// FIXME: The rename verifier should notify user about this issue.
    MatchingSymbol
  };

  SymbolOccurrence(const SymbolName &Name, OccurrenceKind Kind,
                   ArrayRef<SourceLocation> Locations);

  SymbolOccurrence(SymbolOccurrence &&) = default;
  SymbolOccurrence &operator=(SymbolOccurrence &&) = default;

  OccurrenceKind getKind() const { return Kind; }

  ArrayRef<SourceRange> getNameRanges() const {
    if (MultipleRanges)
      return toolchain::ArrayRef(MultipleRanges.get(), NumRanges);
    return SingleRange;
  }

private:
  OccurrenceKind Kind;
  std::unique_ptr<SourceRange[]> MultipleRanges;
  union {
    SourceRange SingleRange;
    unsigned NumRanges;
  };
};

using SymbolOccurrences = std::vector<SymbolOccurrence>;

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_SYMBOLOCCURRENCES_H
