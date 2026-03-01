//===- DebugStringTableSubsection.cpp - CodeView String Table -------------===//
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

#include "vm/core/DebugInfo/CodeView/DebugStringTableSubsection.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Error.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;
using namespace vm::core::codeview;

DebugStringTableSubsectionRef::DebugStringTableSubsectionRef()
    : DebugSubsectionRef(DebugSubsectionKind::StringTable) {}

Error DebugStringTableSubsectionRef::initialize(BinaryStreamRef Contents) {
  Stream = Contents;
  return Error::success();
}

Error DebugStringTableSubsectionRef::initialize(BinaryStreamReader &Reader) {
  return Reader.readStreamRef(Stream);
}

Expected<StringRef>
DebugStringTableSubsectionRef::getString(uint32_t Offset) const {
  BinaryStreamReader Reader(Stream);
  Reader.setOffset(Offset);
  StringRef Result;
  if (auto EC = Reader.readCString(Result))
    return std::move(EC);
  return Result;
}

DebugStringTableSubsection::DebugStringTableSubsection()
    : DebugSubsection(DebugSubsectionKind::StringTable) {}

uint32_t DebugStringTableSubsection::insert(StringRef S) {
  auto P = StringToId.insert({S, StringSize});

  // If a given string didn't exist in the string table, we want to increment
  // the string table size and insert it into the reverse lookup.
  if (P.second) {
    IdToString.insert({P.first->getValue(), P.first->getKey()});
    StringSize += S.size() + 1; // +1 for '\0'
  }

  return P.first->second;
}

uint32_t DebugStringTableSubsection::calculateSerializedSize() const {
  return StringSize;
}

Error DebugStringTableSubsection::commit(BinaryStreamWriter &Writer) const {
  uint32_t Begin = Writer.getOffset();
  uint32_t End = Begin + StringSize;

  // Write a null string at the beginning.
  if (auto EC = Writer.writeCString(StringRef()))
    return EC;

  for (auto &Pair : StringToId) {
    StringRef S = Pair.getKey();
    uint32_t Offset = Begin + Pair.getValue();
    Writer.setOffset(Offset);
    if (auto EC = Writer.writeCString(S))
      return EC;
    assert(Writer.getOffset() <= End);
  }

  Writer.setOffset(End);
  assert((End - Begin) == StringSize);
  return Error::success();
}

uint32_t DebugStringTableSubsection::size() const { return StringToId.size(); }

std::vector<uint32_t> DebugStringTableSubsection::sortedIds() const {
  std::vector<uint32_t> Result;
  Result.reserve(IdToString.size());
  for (const auto &Entry : IdToString)
    Result.push_back(Entry.first);
  toolchain::sort(Result);
  return Result;
}

uint32_t DebugStringTableSubsection::getIdForString(StringRef S) const {
  auto Iter = StringToId.find(S);
  assert(Iter != StringToId.end());
  return Iter->second;
}

StringRef DebugStringTableSubsection::getStringForId(uint32_t Id) const {
  auto Iter = IdToString.find(Id);
  assert(Iter != IdToString.end());
  return Iter->second;
}
