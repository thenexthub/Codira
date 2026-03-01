//===- RemarkStringTable.cpp ----------------------------------------------===//
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
// Implementation of the Remark string table used at remark generation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/RemarkStringTable.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Remarks/Remark.h"
#include "vm/core/Remarks/RemarkParser.h"
#include "vm/core/Support/raw_ostream.h"
#include <vector>

using namespace vm::core;
using namespace vm::core::remarks;

StringTable::StringTable(const ParsedStringTable &Other) {
  for (unsigned i = 0, e = Other.size(); i < e; ++i)
    if (Expected<StringRef> MaybeStr = Other[i])
      add(*MaybeStr);
    else
      llvm_unreachable("Unexpected error while building remarks string table.");
}

std::pair<unsigned, StringRef> StringTable::add(StringRef Str) {
  size_t NextID = StrTab.size();
  auto KV = StrTab.insert({Str, NextID});
  // If it's a new string, add it to the final size.
  if (KV.second)
    SerializedSize += KV.first->first().size() + 1; // +1 for the '\0'
  // Can be either NextID or the previous ID if the string is already there.
  return {KV.first->second, KV.first->first()};
}

void StringTable::internalize(Remark &R) {
  auto Impl = [&](StringRef &S) { S = add(S).second; };
  Impl(R.PassName);
  Impl(R.RemarkName);
  Impl(R.FunctionName);
  if (R.Loc)
    Impl(R.Loc->SourceFilePath);
  for (Argument &Arg : R.Args) {
    Impl(Arg.Key);
    Impl(Arg.Val);
    if (Arg.Loc)
      Impl(Arg.Loc->SourceFilePath);
  }
}

void StringTable::serialize(raw_ostream &OS) const {
  // Emit the sequence of strings.
  for (StringRef Str : serialize()) {
    OS << Str;
    // Explicitly emit a '\0'.
    OS.write('\0');
  }
}

std::vector<StringRef> StringTable::serialize() const {
  std::vector<StringRef> Strings{StrTab.size()};
  for (const auto &KV : StrTab)
    Strings[KV.second] = KV.first();
  return Strings;
}
