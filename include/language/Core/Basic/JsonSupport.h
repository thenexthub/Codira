//===- JsonSupport.h - JSON Output Utilities --------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_BASIC_JSONSUPPORT_H
#define LANGUAGE_CORE_BASIC_JSONSUPPORT_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include <iterator>

namespace language::Core {

inline raw_ostream &Indent(raw_ostream &Out, const unsigned int Space,
                           bool IsDot) {
  for (unsigned int I = 0; I < Space * 2; ++I)
    Out << (IsDot ? "&nbsp;" : " ");
  return Out;
}

inline std::string JsonFormat(StringRef RawSR, bool AddQuotes) {
  if (RawSR.empty())
    return "null";

  // Trim special characters.
  std::string Str = RawSR.trim().str();
  size_t Pos = 0;

  // Escape backslashes.
  while (true) {
    Pos = Str.find('\\', Pos);
    if (Pos == std::string::npos)
      break;

    // Prevent bad conversions.
    size_t TempPos = (Pos != 0) ? Pos - 1 : 0;

    // See whether the current backslash is not escaped.
    if (TempPos != Str.find("\\\\", Pos)) {
      Str.insert(Pos, "\\");
      ++Pos; // As we insert the backslash move plus one.
    }

    ++Pos;
  }

  // Escape double quotes.
  Pos = 0;
  while (true) {
    Pos = Str.find('\"', Pos);
    if (Pos == std::string::npos)
      break;

    // Prevent bad conversions.
    size_t TempPos = (Pos != 0) ? Pos - 1 : 0;

    // See whether the current double quote is not escaped.
    if (TempPos != Str.find("\\\"", Pos)) {
      Str.insert(Pos, "\\");
      ++Pos; // As we insert the escape-character move plus one.
    }

    ++Pos;
  }

  // Remove new-lines.
  toolchain::erase(Str, '\n');

  if (!AddQuotes)
    return Str;

  return '\"' + Str + '\"';
}

inline void printSourceLocationAsJson(raw_ostream &Out, SourceLocation Loc,
                                      const SourceManager &SM,
                                      bool AddBraces = true) {
  // Mostly copy-pasted from SourceLocation::print.
  if (!Loc.isValid()) {
    Out << "null";
    return;
  }

  if (Loc.isFileID()) {
    PresumedLoc PLoc = SM.getPresumedLoc(Loc);

    if (PLoc.isInvalid()) {
      Out << "null";
      return;
    }
    // The macro expansion and spelling pos is identical for file locs.
    if (AddBraces)
      Out << "{ ";
    std::string filename(PLoc.getFilename());
    if (is_style_windows(toolchain::sys::path::Style::native)) {
      // Remove forbidden Windows path characters
      toolchain::erase_if(filename, [](auto Char) {
        static const char ForbiddenChars[] = "<>*?\"|";
        return toolchain::is_contained(ForbiddenChars, Char);
      });
      // Handle windows-specific path delimiters.
      toolchain::replace(filename, '\\', '/');
    }
    Out << "\"line\": " << PLoc.getLine()
        << ", \"column\": " << PLoc.getColumn()
        << ", \"file\": \"" << filename << "\"";
    if (AddBraces)
      Out << " }";
    return;
  }

  // We want 'location: { ..., spelling: { ... }}' but not
  // 'location: { ... }, spelling: { ... }', hence the dance
  // with braces.
  Out << "{ ";
  printSourceLocationAsJson(Out, SM.getExpansionLoc(Loc), SM, false);
  Out << ", \"spelling\": ";
  printSourceLocationAsJson(Out, SM.getSpellingLoc(Loc), SM, true);
  Out << " }";
}
} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_JSONSUPPORT_H
