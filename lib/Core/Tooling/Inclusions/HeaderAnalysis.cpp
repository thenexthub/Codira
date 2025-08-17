//===--- HeaderAnalysis.cpp -------------------------------------*- C++ -*-===//
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

#include "language/Core/Tooling/Inclusions/HeaderAnalysis.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Lex/HeaderSearch.h"

namespace language::Core::tooling {
namespace {

// Is Line an #if or #ifdef directive?
// FIXME: This makes headers with #ifdef LINUX/WINDOWS/MACOS marked as non
// self-contained and is probably not what we want.
bool isIf(toolchain::StringRef Line) {
  Line = Line.ltrim();
  if (!Line.consume_front("#"))
    return false;
  Line = Line.ltrim();
  return Line.starts_with("if");
}

// Is Line an #error directive mentioning includes?
bool isErrorAboutInclude(toolchain::StringRef Line) {
  Line = Line.ltrim();
  if (!Line.consume_front("#"))
    return false;
  Line = Line.ltrim();
  if (!Line.starts_with("error"))
    return false;
  return Line.contains_insensitive(
      "includ"); // Matches "include" or "including".
}

// Heuristically headers that only want to be included via an umbrella.
bool isDontIncludeMeHeader(StringRef Content) {
  toolchain::StringRef Line;
  // Only sniff up to 100 lines or 10KB.
  Content = Content.take_front(100 * 100);
  for (unsigned I = 0; I < 100 && !Content.empty(); ++I) {
    std::tie(Line, Content) = Content.split('\n');
    if (isIf(Line) && isErrorAboutInclude(Content.split('\n').first))
      return true;
  }
  return false;
}

bool isImportLine(toolchain::StringRef Line) {
  Line = Line.ltrim();
  if (!Line.consume_front("#"))
    return false;
  Line = Line.ltrim();
  return Line.starts_with("import");
}

toolchain::StringRef getFileContents(FileEntryRef FE, const SourceManager &SM) {
  return const_cast<SourceManager &>(SM)
      .getMemoryBufferForFileOrNone(FE)
      .value_or(toolchain::MemoryBufferRef())
      .getBuffer();
}

} // namespace

bool isSelfContainedHeader(FileEntryRef FE, const SourceManager &SM,
                           const HeaderSearch &HeaderInfo) {
  if (!HeaderInfo.isFileMultipleIncludeGuarded(FE) &&
      !HeaderInfo.hasFileBeenImported(FE) &&
      // Any header that contains #imports is supposed to be #import'd so no
      // need to check for anything but the main-file.
      (SM.getFileEntryForID(SM.getMainFileID()) != FE ||
       !codeContainsImports(getFileContents(FE, SM))))
    return false;
  // This pattern indicates that a header can't be used without
  // particular preprocessor state, usually set up by another header.
  return !isDontIncludeMeHeader(getFileContents(FE, SM));
}

bool codeContainsImports(toolchain::StringRef Code) {
  // Only sniff up to 100 lines or 10KB.
  Code = Code.take_front(100 * 100);
  toolchain::StringRef Line;
  for (unsigned I = 0; I < 100 && !Code.empty(); ++I) {
    std::tie(Line, Code) = Code.split('\n');
    if (isImportLine(Line))
      return true;
  }
  return false;
}

std::optional<StringRef> parseIWYUPragma(const char *Text) {
  // Skip the comment start, // or /*.
  if (Text[0] != '/' || (Text[1] != '/' && Text[1] != '*'))
    return std::nullopt;
  bool BlockComment = Text[1] == '*';
  Text += 2;

  // Per spec, direcitves are whitespace- and case-sensitive.
  constexpr toolchain::StringLiteral IWYUPragma = " IWYU pragma: ";
  if (strncmp(Text, IWYUPragma.data(), IWYUPragma.size()))
    return std::nullopt;
  Text += IWYUPragma.size();
  const char *End = Text;
  while (*End != 0 && *End != '\n')
    ++End;
  StringRef Rest(Text, End - Text);
  // Strip off whitespace and comment markers to avoid confusion. This isn't
  // fully-compatible with IWYU, which splits into whitespace-delimited tokens.
  if (BlockComment)
    Rest.consume_back("*/");
  return Rest.trim();
}

} // namespace language::Core::tooling
