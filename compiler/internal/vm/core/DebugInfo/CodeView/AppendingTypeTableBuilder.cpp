//===- AppendingTypeTableBuilder.cpp --------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/AppendingTypeTableBuilder.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/CodeView/ContinuationRecordBuilder.h"
#include "vm/core/DebugInfo/CodeView/TypeIndex.h"
#include "vm/core/Support/Allocator.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>
#include <cstring>

using namespace vm::core;
using namespace vm::core::codeview;

TypeIndex AppendingTypeTableBuilder::nextTypeIndex() const {
  return TypeIndex::fromArrayIndex(SeenRecords.size());
}

AppendingTypeTableBuilder::AppendingTypeTableBuilder(BumpPtrAllocator &Storage)
    : RecordStorage(Storage) {}

AppendingTypeTableBuilder::~AppendingTypeTableBuilder() = default;

std::optional<TypeIndex> AppendingTypeTableBuilder::getFirst() {
  if (empty())
    return std::nullopt;

  return TypeIndex(TypeIndex::FirstNonSimpleIndex);
}

std::optional<TypeIndex> AppendingTypeTableBuilder::getNext(TypeIndex Prev) {
  if (++Prev == nextTypeIndex())
    return std::nullopt;
  return Prev;
}

CVType AppendingTypeTableBuilder::getType(TypeIndex Index){
  return CVType(SeenRecords[Index.toArrayIndex()]);
}

StringRef AppendingTypeTableBuilder::getTypeName(TypeIndex Index) {
  llvm_unreachable("Method not implemented");
}

bool AppendingTypeTableBuilder::contains(TypeIndex Index) {
  if (Index.isSimple() || Index.isNoneType())
    return false;

  return Index.toArrayIndex() < SeenRecords.size();
}

uint32_t AppendingTypeTableBuilder::size() { return SeenRecords.size(); }

uint32_t AppendingTypeTableBuilder::capacity() { return SeenRecords.size(); }

ArrayRef<ArrayRef<uint8_t>> AppendingTypeTableBuilder::records() const {
  return SeenRecords;
}

void AppendingTypeTableBuilder::reset() { SeenRecords.clear(); }

static ArrayRef<uint8_t> stabilize(BumpPtrAllocator &RecordStorage,
                                   ArrayRef<uint8_t> Record) {
  uint8_t *Stable = RecordStorage.Allocate<uint8_t>(Record.size());
  memcpy(Stable, Record.data(), Record.size());
  return ArrayRef<uint8_t>(Stable, Record.size());
}

TypeIndex
AppendingTypeTableBuilder::insertRecordBytes(ArrayRef<uint8_t> &Record) {
  TypeIndex NewTI = nextTypeIndex();
  Record = stabilize(RecordStorage, Record);
  SeenRecords.push_back(Record);
  return NewTI;
}

TypeIndex
AppendingTypeTableBuilder::insertRecord(ContinuationRecordBuilder &Builder) {
  TypeIndex TI;
  auto Fragments = Builder.end(nextTypeIndex());
  assert(!Fragments.empty());
  for (auto C : Fragments)
    TI = insertRecordBytes(C.RecordData);
  return TI;
}

bool AppendingTypeTableBuilder::replaceType(TypeIndex &Index, CVType Data,
                                            bool Stabilize) {
  assert(Index.toArrayIndex() < SeenRecords.size() &&
         "This function cannot be used to insert records!");

  ArrayRef<uint8_t> Record = Data.data();
  if (Stabilize)
    Record = stabilize(RecordStorage, Record);
  SeenRecords[Index.toArrayIndex()] = Record;
  return true;
}
