//===-- StringExtras.cpp - Implement the StringExtras header --------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements the StringExtras.h header
//
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Support/raw_ostream.h"
#include <cctype>

using namespace vm::core;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> toolchain::getToken(StringRef Source,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::size_type Start = Source.find_first_not_of(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::size_type End = Source.find_first_of(Delimiters, Start);

  return {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
void toolchain::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  while (!S.first.empty()) {
    OutFragments.push_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

void toolchain::printEscapedString(StringRef Name, raw_ostream &Out) {
  for (unsigned char C : Name) {
    if (C == '\\')
      Out << '\\' << C;
    else if (isPrint(C) && C != '"')
      Out << C;
    else
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

void toolchain::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  for (char C : String) {
    if (C == '&')
      Out << "&amp;";
    else if (C == '<')
      Out << "&lt;";
    else if (C == '>')
      Out << "&gt;";
    else if (C == '\"')
      Out << "&quot;";
    else if (C == '\'')
      Out << "&apos;";
    else
      Out << C;
  }
}

void toolchain::printLowerCase(StringRef String, raw_ostream &Out) {
  for (const char C : String)
    Out << toLower(C);
}

std::string toolchain::convertToSnakeFromCamelCase(StringRef input) {
  if (input.empty())
    return "";

  std::string snakeCase;
  snakeCase.reserve(input.size());
  auto check = [&input](size_t j, function_ref<bool(int)> predicate) {
    return j < input.size() && predicate(input[j]);
  };
  for (size_t i = 0; i < input.size(); ++i) {
    snakeCase.push_back(tolower(input[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    if (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.push_back('_');
    if ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.push_back('_');
  }
  return snakeCase;
}

std::string toolchain::convertToCamelFromSnakeCase(StringRef input,
                                              bool capitalizeFirst) {
  if (input.empty())
    return "";

  std::string output;
  output.reserve(input.size());

  // Push the first character, capatilizing if necessary.
  if (capitalizeFirst && std::islower(input.front()))
    output.push_back(toolchain::toUpper(input.front()));
  else
    output.push_back(input.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  for (size_t pos = 1, e = input.size(); pos < e; ++pos) {
    if (input[pos] == '_' && pos != (e - 1) && std::islower(input[pos + 1]))
      output.push_back(toolchain::toUpper(input[++pos]));
    else
      output.push_back(input[pos]);
  }
  return output;
}
