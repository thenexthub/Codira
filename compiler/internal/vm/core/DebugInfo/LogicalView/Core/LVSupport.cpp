//===-- LVSupport.cpp -----------------------------------------------------===//
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
// This implements the supporting functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/LogicalView/Core/LVSupport.h"
#include "vm/core/Support/FormatVariadic.h"

using namespace vm::core;
using namespace vm::core::logicalview;

#define DEBUG_TYPE "Support"

namespace {
// Unique string pool instance used by all logical readers.
LVStringPool StringPool;
} // namespace
LVStringPool &toolchain::logicalview::getStringPool() { return StringPool; }

// Perform the following transformations to the given 'Path':
// - all characters to lowercase.
// - '\\' into '/' (Platform independent).
// - '//' into '/'
std::string toolchain::logicalview::transformPath(StringRef Path) {
  std::string Name(Path);
  toolchain::transform(Name, Name.begin(), tolower);
  toolchain::replace(Name, '\\', '/');

  // Remove all duplicate slashes.
  size_t Pos = 0;
  while ((Pos = Name.find("//", Pos)) != std::string::npos)
    Name.erase(Pos, 1);

  return Name;
}

// Convert the given 'Path' to lowercase and change any matching character
// from 'CharSet' into '_'.
// The characters in 'CharSet' are:
//   '/', '\', '<', '>', '.', ':', '%', '*', '?', '|', '"', ' '.
std::string toolchain::logicalview::flattenedFilePath(StringRef Path) {
  std::string Name(Path);
  toolchain::transform(Name, Name.begin(), tolower);

  const char *CharSet = "/\\<>.:%*?|\" ";
  char *Input = Name.data();
  while (Input && *Input) {
    Input = strpbrk(Input, CharSet);
    if (Input)
      *Input++ = '_';
  };
  return Name;
}

using LexicalEntry = std::pair<size_t, size_t>;
using LexicalIndexes = SmallVector<LexicalEntry, 10>;

static LexicalIndexes getAllLexicalIndexes(StringRef Name) {
  if (Name.empty())
    return {};

  size_t AngleCount = 0;
  size_t ColonSeen = 0;
  size_t Current = 0;

  LexicalIndexes Indexes;

#ifndef NDEBUG
  auto PrintLexicalEntry = [&]() {
    LexicalEntry Entry = Indexes.back();
    toolchain::dbgs() << formatv(
        "'{0}:{1}', '{2}'\n", Entry.first, Entry.second,
        Name.substr(Entry.first, Entry.second - Entry.first + 1));
  };
#endif

  size_t Length = Name.size();
  for (size_t Index = 0; Index < Length; ++Index) {
    LLVM_DEBUG({
      toolchain::dbgs() << formatv("Index: '{0}', Char: '{1}'\n", Index,
                              Name[Index]);
    });
    switch (Name[Index]) {
    case '<':
      ++AngleCount;
      break;
    case '>':
      --AngleCount;
      break;
    case ':':
      ++ColonSeen;
      break;
    }
    if (ColonSeen == 2) {
      if (!AngleCount) {
        Indexes.push_back(LexicalEntry(Current, Index - 2));
        Current = Index + 1;
        LLVM_DEBUG({ PrintLexicalEntry(); });
      }
      ColonSeen = 0;
      continue;
    }
  }

  // Store last component.
  Indexes.push_back(LexicalEntry(Current, Length - 1));
  LLVM_DEBUG({ PrintLexicalEntry(); });
  return Indexes;
}

LVLexicalComponent toolchain::logicalview::getInnerComponent(StringRef Name) {
  if (Name.empty())
    return {};

  LexicalIndexes Indexes = getAllLexicalIndexes(Name);
  if (Indexes.size() == 1)
    return std::make_tuple(StringRef(), Name);

  LexicalEntry BeginEntry = Indexes.front();
  LexicalEntry EndEntry = Indexes[Indexes.size() - 2];
  StringRef Outer =
      Name.substr(BeginEntry.first, EndEntry.second - BeginEntry.first + 1);

  LexicalEntry LastEntry = Indexes.back();
  StringRef Inner =
      Name.substr(LastEntry.first, LastEntry.second - LastEntry.first + 1);

  return std::make_tuple(Outer, Inner);
}

LVStringRefs toolchain::logicalview::getAllLexicalComponents(StringRef Name) {
  if (Name.empty())
    return {};

  LexicalIndexes Indexes = getAllLexicalIndexes(Name);
  LVStringRefs Components;
  for (const LexicalEntry &Entry : Indexes)
    Components.push_back(
        Name.substr(Entry.first, Entry.second - Entry.first + 1));

  return Components;
}

std::string toolchain::logicalview::getScopedName(const LVStringRefs &Components,
                                             StringRef BaseName) {
  if (Components.empty())
    return {};
  std::string Name(BaseName);
  raw_string_ostream Stream(Name);
  if (BaseName.size())
    Stream << "::";
  Stream << Components[0];
  for (LVStringRefs::size_type Index = 1; Index < Components.size(); ++Index)
    Stream << "::" << Components[Index];
  return Name;
}
