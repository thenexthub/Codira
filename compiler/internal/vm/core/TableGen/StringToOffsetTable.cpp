//===- StringToOffsetTable.cpp - Emit a big concatenated string -*- C++ -*-===//
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

#include "vm/core/TableGen/StringToOffsetTable.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TableGen/Error.h"
#include "vm/core/TableGen/Main.h"

using namespace vm::core;

unsigned StringToOffsetTable::GetOrAddStringOffset(StringRef Str) {
  auto [II, Inserted] = StringOffset.insert({Str, size()});
  if (Inserted) {
    // Add the string to the aggregate if this is the first time found.
    AggregateString.append(Str.begin(), Str.end());
    if (AppendZero)
      AggregateString += '\0';
  }

  return II->second;
}

void StringToOffsetTable::EmitStringTableDef(raw_ostream &OS,
                                             const Twine &Name) const {
  // This generates a `toolchain::StringTable` which expects that entries are null
  // terminated. So fail with an error if `AppendZero` is false.
  if (!AppendZero)
    PrintFatalError("toolchain::StringTable requires null terminated strings");

  OS << formatv(R"(
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif
{} constexpr char {}{}Storage[] =)",
                ClassPrefix.empty() ? "static" : "",
                UsePrefixForStorageMember ? ClassPrefix : "", Name);

  // MSVC silently miscompiles string literals longer than 64k in some
  // circumstances. The build system sets EmitLongStrLiterals to false when it
  // detects that it is targetting MSVC. When that option is false and the
  // string table is longer than 64k, emit it as an array of character
  // literals.
  bool UseChars = !EmitLongStrLiterals && AggregateString.size() > (64 * 1024);
  OS << (UseChars ? "{\n" : "\n");

  ListSeparator LineSep(UseChars ? ",\n" : "\n");
  SmallVector<StringRef> Strings(split(AggregateString, '\0'));
  // We should always have an empty string at the start, and because these are
  // null terminators rather than separators, we'll have one at the end as
  // well. Skip the end one.
  assert(Strings.front().empty() && "Expected empty initial string!");
  assert(Strings.back().empty() &&
         "Expected empty string at the end due to terminators!");
  Strings.pop_back();
  for (StringRef Str : Strings) {
    OS << LineSep << "  ";
    // If we can, just emit this as a string literal to be concatenated.
    if (!UseChars) {
      OS << "\"";
      OS.write_escaped(Str);
      OS << "\\0\"";
      continue;
    }

    ListSeparator CharSep(", ");
    for (char C : Str) {
      OS << CharSep << "'";
      OS.write_escaped(StringRef(&C, 1));
      OS << "'";
    }
    OS << CharSep << "'\\0'";
  }
  OS << LineSep << (UseChars ? "};" : "  ;");

  OS << formatv(R"(
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

{1} toolchain::StringTable
{2}{0} = {0}Storage;
)",
                Name, ClassPrefix.empty() ? "static constexpr" : "const",
                ClassPrefix);
}

void StringToOffsetTable::EmitString(raw_ostream &O) const {
  // Escape the string.
  SmallString<256> EscapedStr;
  raw_svector_ostream(EscapedStr).write_escaped(AggregateString);

  O << "    \"";
  unsigned CharsPrinted = 0;
  for (unsigned i = 0, e = EscapedStr.size(); i != e; ++i) {
    if (CharsPrinted > 70) {
      O << "\"\n    \"";
      CharsPrinted = 0;
    }
    O << EscapedStr[i];
    ++CharsPrinted;

    // Print escape sequences all together.
    if (EscapedStr[i] != '\\')
      continue;

    assert(i + 1 < EscapedStr.size() && "Incomplete escape sequence!");
    if (isDigit(EscapedStr[i + 1])) {
      assert(isDigit(EscapedStr[i + 2]) && isDigit(EscapedStr[i + 3]) &&
             "Expected 3 digit octal escape!");
      O << EscapedStr[++i];
      O << EscapedStr[++i];
      O << EscapedStr[++i];
      CharsPrinted += 3;
    } else {
      O << EscapedStr[++i];
      ++CharsPrinted;
    }
  }
  O << "\"";
}
