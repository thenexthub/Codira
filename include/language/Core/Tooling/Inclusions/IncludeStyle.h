//===--- IncludeStyle.h - Style of C++ #include directives -------*- C++-*-===//
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

#ifndef LANGUAGE_CORE_TOOLING_INCLUSIONS_INCLUDESTYLE_H
#define LANGUAGE_CORE_TOOLING_INCLUSIONS_INCLUDESTYLE_H

#include "toolchain/Support/YAMLTraits.h"
#include <string>
#include <vector>

namespace language::Core {
namespace tooling {

/// Style for sorting and grouping C++ #include directives.
struct IncludeStyle {
  /// Styles for sorting multiple ``#include`` blocks.
  enum IncludeBlocksStyle {
    /// Sort each ``#include`` block separately.
    /// \code
    ///    #include "b.h"               into      #include "b.h"
    ///
    ///    #include <lib/main.h>                  #include "a.h"
    ///    #include "a.h"                         #include <lib/main.h>
    /// \endcode
    IBS_Preserve,
    /// Merge multiple ``#include`` blocks together and sort as one.
    /// \code
    ///    #include "b.h"               into      #include "a.h"
    ///                                           #include "b.h"
    ///    #include <lib/main.h>                  #include <lib/main.h>
    ///    #include "a.h"
    /// \endcode
    IBS_Merge,
    /// Merge multiple ``#include`` blocks together and sort as one.
    /// Then split into groups based on category priority. See
    /// ``IncludeCategories``.
    /// \code
    ///    #include "b.h"               into      #include "a.h"
    ///                                           #include "b.h"
    ///    #include <lib/main.h>
    ///    #include "a.h"                         #include <lib/main.h>
    /// \endcode
    IBS_Regroup,
  };

  /// Dependent on the value, multiple ``#include`` blocks can be sorted
  /// as one and divided based on category.
  /// \version 6
  IncludeBlocksStyle IncludeBlocks;

  /// See documentation of ``IncludeCategories``.
  struct IncludeCategory {
    /// The regular expression that this category matches.
    std::string Regex;
    /// The priority to assign to this category.
    int Priority;
    /// The custom priority to sort before grouping.
    int SortPriority;
    /// If the regular expression is case sensitive.
    bool RegexIsCaseSensitive;
    bool operator==(const IncludeCategory &Other) const {
      return Regex == Other.Regex && Priority == Other.Priority &&
             RegexIsCaseSensitive == Other.RegexIsCaseSensitive;
    }
  };

  /// Regular expressions denoting the different ``#include`` categories
  /// used for ordering ``#includes``.
  ///
  /// `POSIX extended
  /// <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html>`_
  /// regular expressions are supported.
  ///
  /// These regular expressions are matched against the filename of an include
  /// (including the <> or "") in order. The value belonging to the first
  /// matching regular expression is assigned and ``#includes`` are sorted first
  /// according to increasing category number and then alphabetically within
  /// each category.
  ///
  /// If none of the regular expressions match, INT_MAX is assigned as
  /// category. The main header for a source file automatically gets category 0.
  /// so that it is generally kept at the beginning of the ``#includes``
  /// (https://toolchain.org/docs/CodingStandards.html#include-style). However, you
  /// can also assign negative priorities if you have certain headers that
  /// always need to be first.
  ///
  /// There is a third and optional field ``SortPriority`` which can used while
  /// ``IncludeBlocks = IBS_Regroup`` to define the priority in which
  /// ``#includes`` should be ordered. The value of ``Priority`` defines the
  /// order of ``#include blocks`` and also allows the grouping of ``#includes``
  /// of different priority. ``SortPriority`` is set to the value of
  /// ``Priority`` as default if it is not assigned.
  ///
  /// Each regular expression can be marked as case sensitive with the field
  /// ``CaseSensitive``, per default it is not.
  ///
  /// To configure this in the .clang-format file, use:
  /// \code{.yaml}
  ///   IncludeCategories:
  ///     - Regex:           '^"(toolchain|toolchain-c|clang|clang-c)/'
  ///       Priority:        2
  ///       SortPriority:    2
  ///       CaseSensitive:   true
  ///     - Regex:           '^((<|")(gtest|gmock|isl|json)/)'
  ///       Priority:        3
  ///     - Regex:           '<[[:alnum:].]+>'
  ///       Priority:        4
  ///     - Regex:           '.*'
  ///       Priority:        1
  ///       SortPriority:    0
  /// \endcode
  /// \version 3.8
  std::vector<IncludeCategory> IncludeCategories;

  /// Specify a regular expression of suffixes that are allowed in the
  /// file-to-main-include mapping.
  ///
  /// When guessing whether a #include is the "main" include (to assign
  /// category 0, see above), use this regex of allowed suffixes to the header
  /// stem. A partial match is done, so that:
  /// * ``""`` means "arbitrary suffix"
  /// * ``"$"`` means "no suffix"
  ///
  /// For example, if configured to ``"(_test)?$"``, then a header a.h would be
  /// seen as the "main" include in both a.cc and a_test.cc.
  /// \version 3.9
  std::string IncludeIsMainRegex;

  /// Specify a regular expression for files being formatted
  /// that are allowed to be considered "main" in the
  /// file-to-main-include mapping.
  ///
  /// By default, clang-format considers files as "main" only when they end
  /// with: ``.c``, ``.cc``, ``.cpp``, ``.c++``, ``.cxx``, ``.m`` or ``.mm``
  /// extensions.
  /// For these files a guessing of "main" include takes place
  /// (to assign category 0, see above). This config option allows for
  /// additional suffixes and extensions for files to be considered as "main".
  ///
  /// For example, if this option is configured to ``(Impl\.hpp)$``,
  /// then a file ``ClassImpl.hpp`` is considered "main" (in addition to
  /// ``Class.c``, ``Class.cc``, ``Class.cpp`` and so on) and "main
  /// include file" logic will be executed (with *IncludeIsMainRegex* setting
  /// also being respected in later phase). Without this option set,
  /// ``ClassImpl.hpp`` would not have the main include file put on top
  /// before any other include.
  /// \version 10
  std::string IncludeIsMainSourceRegex;

  /// Character to consider in the include directives for the main header.
  enum MainIncludeCharDiscriminator : int8_t {
    /// Main include uses quotes: ``#include "foo.hpp"`` (the default).
    MICD_Quote,
    /// Main include uses angle brackets: ``#include <foo.hpp>``.
    MICD_AngleBracket,
    /// Main include uses either quotes or angle brackets.
    MICD_Any
  };

  /// When guessing whether a #include is the "main" include, only the include
  /// directives that use the specified character are considered.
  /// \version 19
  MainIncludeCharDiscriminator MainIncludeChar;
};

} // namespace tooling
} // namespace language::Core

LLVM_YAML_IS_SEQUENCE_VECTOR(language::Core::tooling::IncludeStyle::IncludeCategory)

namespace toolchain {
namespace yaml {

template <>
struct MappingTraits<language::Core::tooling::IncludeStyle::IncludeCategory> {
  static void mapping(IO &IO,
                      language::Core::tooling::IncludeStyle::IncludeCategory &Category);
};

template <>
struct ScalarEnumerationTraits<
    language::Core::tooling::IncludeStyle::IncludeBlocksStyle> {
  static void
  enumeration(IO &IO, language::Core::tooling::IncludeStyle::IncludeBlocksStyle &Value);
};

template <>
struct ScalarEnumerationTraits<
    language::Core::tooling::IncludeStyle::MainIncludeCharDiscriminator> {
  static void enumeration(
      IO &IO,
      language::Core::tooling::IncludeStyle::MainIncludeCharDiscriminator &Value);
};

} // namespace yaml
} // namespace toolchain

#endif // LANGUAGE_CORE_TOOLING_INCLUSIONS_INCLUDESTYLE_H
