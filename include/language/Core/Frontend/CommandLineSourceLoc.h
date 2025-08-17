
//===--- CommandLineSourceLoc.h - Parsing for source locations-*- C++ -*---===//
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
// Command line parsing for source locations.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_COMMANDLINESOURCELOC_H
#define LANGUAGE_CORE_FRONTEND_COMMANDLINESOURCELOC_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

namespace language::Core {

/// A source location that has been parsed on the command line.
struct ParsedSourceLocation {
  std::string FileName;
  // The 1-based line number
  unsigned Line;
  // The 1-based column number
  unsigned Column;

public:
  /// Construct a parsed source location from a string; the Filename is empty on
  /// error.
  static ParsedSourceLocation FromString(StringRef Str) {
    ParsedSourceLocation PSL;
    std::pair<StringRef, StringRef> ColSplit = Str.rsplit(':');
    std::pair<StringRef, StringRef> LineSplit =
      ColSplit.first.rsplit(':');

    // If both tail splits were valid integers, return success.
    if (!ColSplit.second.getAsInteger(10, PSL.Column) &&
        !LineSplit.second.getAsInteger(10, PSL.Line) &&
        !(PSL.Column == 0 || PSL.Line == 0)) {
      PSL.FileName = std::string(LineSplit.first);

      // On the command-line, stdin may be specified via "-". Inside the
      // compiler, stdin is called "<stdin>".
      if (PSL.FileName == "-")
        PSL.FileName = "<stdin>";
    }

    return PSL;
  }

  /// Serialize ParsedSourceLocation back to a string.
  std::string ToString() const {
    return (toolchain::Twine(FileName == "<stdin>" ? "-" : FileName) + ":" +
            Twine(Line) + ":" + Twine(Column))
        .str();
  }
};

/// A source range that has been parsed on the command line.
struct ParsedSourceRange {
  std::string FileName;
  /// The starting location of the range. The first element is the line and
  /// the second element is the column.
  std::pair<unsigned, unsigned> Begin;
  /// The ending location of the range. The first element is the line and the
  /// second element is the column.
  std::pair<unsigned, unsigned> End;

  /// Returns a parsed source range from a string or std::nullopt if the string
  /// is invalid.
  ///
  /// These source string has the following format:
  ///
  /// file:start_line:start_column[-end_line:end_column]
  ///
  /// If the end line and column are omitted, the starting line and columns
  /// are used as the end values.
  static std::optional<ParsedSourceRange> fromString(StringRef Str) {
    std::pair<StringRef, StringRef> RangeSplit = Str.rsplit('-');
    unsigned EndLine, EndColumn;
    bool HasEndLoc = false;
    if (!RangeSplit.second.empty()) {
      std::pair<StringRef, StringRef> Split = RangeSplit.second.rsplit(':');
      if (Split.first.getAsInteger(10, EndLine) ||
          Split.second.getAsInteger(10, EndColumn)) {
        // The string does not end in end_line:end_column, so the '-'
        // probably belongs to the filename which menas the whole
        // string should be parsed.
        RangeSplit.first = Str;
      } else {
        // Column and line numbers are 1-based.
        if (EndLine == 0 || EndColumn == 0)
          return std::nullopt;
        HasEndLoc = true;
      }
    }
    auto Begin = ParsedSourceLocation::FromString(RangeSplit.first);
    if (Begin.FileName.empty())
      return std::nullopt;
    if (!HasEndLoc) {
      EndLine = Begin.Line;
      EndColumn = Begin.Column;
    }
    return ParsedSourceRange{std::move(Begin.FileName),
                             {Begin.Line, Begin.Column},
                             {EndLine, EndColumn}};
  }
};
}

namespace toolchain {
  namespace cl {
    /// Command-line option parser that parses source locations.
    ///
    /// Source locations are of the form filename:line:column.
    template<>
    class parser<language::Core::ParsedSourceLocation> final
      : public basic_parser<language::Core::ParsedSourceLocation> {
    public:
      inline bool parse(Option &O, StringRef ArgName, StringRef ArgValue,
                 language::Core::ParsedSourceLocation &Val);
    };

    bool
    parser<language::Core::ParsedSourceLocation>::
    parse(Option &O, StringRef ArgName, StringRef ArgValue,
          language::Core::ParsedSourceLocation &Val) {
      using namespace language::Core;

      Val = ParsedSourceLocation::FromString(ArgValue);
      if (Val.FileName.empty()) {
        errs() << "error: "
               << "source location must be of the form filename:line:column\n";
        return true;
      }

      return false;
    }
  }
}

#endif
