//===--- HeaderIncludes.h - Insert/Delete #includes for C++ code--*- C++-*-===//
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

#ifndef LANGUAGE_CORE_TOOLING_INCLUSIONS_HEADERINCLUDES_H
#define LANGUAGE_CORE_TOOLING_INCLUSIONS_HEADERINCLUDES_H

#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Tooling/Core/Replacement.h"
#include "language/Core/Tooling/Inclusions/IncludeStyle.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Regex.h"
#include <list>
#include <optional>
#include <unordered_map>

namespace language::Core {
namespace tooling {

/// This class manages priorities of C++ #include categories and calculates
/// priorities for headers.
/// FIXME(ioeric): move this class into implementation file when clang-format's
/// include sorting functions are also moved here.
class IncludeCategoryManager {
public:
  IncludeCategoryManager(const IncludeStyle &Style, StringRef FileName);

  /// Returns the priority of the category which \p IncludeName belongs to.
  /// If \p CheckMainHeader is true and \p IncludeName is a main header, returns
  /// 0. Otherwise, returns the priority of the matching category or INT_MAX.
  /// NOTE: this API is not thread-safe!
  int getIncludePriority(StringRef IncludeName, bool CheckMainHeader) const;
  int getSortIncludePriority(StringRef IncludeName, bool CheckMainHeader) const;

private:
  bool isMainHeader(StringRef IncludeName) const;

  const IncludeStyle Style;
  bool IsMainFile;
  std::string FileName;
  SmallVector<toolchain::Regex, 4> CategoryRegexs;
};

enum class IncludeDirective { Include, Import };

/// Generates replacements for inserting or deleting #include directives in a
/// file.
class HeaderIncludes {
public:
  HeaderIncludes(toolchain::StringRef FileName, toolchain::StringRef Code,
                 const IncludeStyle &Style);

  /// Inserts an #include or #import directive of \p Header into the code.
  /// If \p IsAngled is true, \p Header will be quoted with <> in the directive;
  /// otherwise, it will be quoted with "".
  ///
  /// When searching for points to insert new header, this ignores #include's
  /// after the #include block(s) in the beginning of a file to avoid inserting
  /// headers into code sections where new #include's should not be added by
  /// default. These code sections include:
  ///   - raw string literals (containing #include).
  ///   - #if blocks.
  ///   - Special #include's among declarations (e.g. functions).
  ///
  /// Returns a replacement that inserts the new header into a suitable #include
  /// block of the same category. This respects the order of the existing
  /// #includes in the block; if the existing #includes are not already sorted,
  /// this will simply insert the #include in front of the first #include of the
  /// same category in the code that should be sorted after \p IncludeName. If
  /// \p IncludeName already exists (with exactly the same spelling), this
  /// returns std::nullopt.
  std::optional<tooling::Replacement> insert(toolchain::StringRef Header,
                                             bool IsAngled,
                                             IncludeDirective Directive) const;

  /// Removes all existing #includes and #imports of \p Header quoted with <> if
  /// \p IsAngled is true or "" if \p IsAngled is false.
  /// This doesn't resolve the header file path; it only deletes #includes and
  /// #imports with exactly the same spelling.
  tooling::Replacements remove(toolchain::StringRef Header, bool IsAngled) const;

  // Matches a whole #include directive.
  static const toolchain::Regex IncludeRegex;

private:
  struct Include {
    Include(StringRef Name, tooling::Range R, IncludeDirective D)
        : Name(Name), R(R), Directive(D) {}

    // An include header quoted with either <> or "".
    std::string Name;
    // The range of the whole line of include directive including any leading
    // whitespaces and trailing comment.
    tooling::Range R;
    // Either #include or #import.
    IncludeDirective Directive;
  };

  void addExistingInclude(Include IncludeToAdd, unsigned NextLineOffset);

  std::string FileName;
  std::string Code;

  // Map from include name (quotation trimmed) to a list of existing includes
  // (in case there are more than one) with the name in the current file. <x>
  // and "x" will be treated as the same header when deleting #includes.
  // std::list is used for pointers stability (see IncludesByPriority)
  toolchain::StringMap<std::list<Include>> ExistingIncludes;

  /// Map from priorities of #include categories to all #includes in the same
  /// category. This is used to find #includes of the same category when
  /// inserting new #includes. #includes in the same categories are sorted in
  /// in the order they appear in the source file.
  /// See comment for "FormatStyle::IncludeCategories" for details about include
  /// priorities.
  std::unordered_map<int, toolchain::SmallVector<const Include *, 8>>
      IncludesByPriority;

  int FirstIncludeOffset;
  // All new headers should be inserted after this offset (e.g. after header
  // guards, file comment).
  unsigned MinInsertOffset;
  // Max insertion offset in the original code. For example, we want to avoid
  // inserting new #includes into the actual code section (e.g. after a
  // declaration).
  unsigned MaxInsertOffset;
  // True if we find the main-file header in the Code.
  bool MainIncludeFound;
  IncludeCategoryManager Categories;
  // Record the offset of the end of the last include in each category.
  std::unordered_map<int, int> CategoryEndOffsets;

  // All possible priorities.
  std::set<int> Priorities;
};

} // namespace tooling
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_INCLUSIONS_HEADERINCLUDES_H
