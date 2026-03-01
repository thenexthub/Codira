//===- MergingTypeTableBuilder.cpp ----------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/MergingTypeTableBuilder.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/CodeView/ContinuationRecordBuilder.h"
#include "vm/core/DebugInfo/CodeView/TypeHashing.h"
#include "vm/core/DebugInfo/CodeView/TypeIndex.h"
#include "vm/core/Support/Allocator.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>
#include <cstring>

using namespace vm::core;
using namespace vm::core::codeview;

TypeIndex MergingTypeTableBuilder::nextTypeIndex() const {
  return TypeIndex::fromArrayIndex(SeenRecords.size());
}

MergingTypeTableBuilder::MergingTypeTableBuilder(BumpPtrAllocator &Storage)
    : RecordStorage(Storage) {
  SeenRecords.reserve(4096);
}

MergingTypeTableBuilder::~MergingTypeTableBuilder() = default;

std::optional<TypeIndex> MergingTypeTableBuilder::getFirst() {
  if (empty())
    return std::nullopt;

  return TypeIndex(TypeIndex::FirstNonSimpleIndex);
}

std::optional<TypeIndex> MergingTypeTableBuilder::getNext(TypeIndex Prev) {
  if (++Prev == nextTypeIndex())
    return std::nullopt;
  return Prev;
}

CVType MergingTypeTableBuilder::getType(TypeIndex Index) {
  CVType Type(SeenRecords[Index.toArrayIndex()]);
  return Type;
}

StringRef MergingTypeTableBuilder::getTypeName(TypeIndex Index) {
  llvm_unreachable("Method not implemented");
}

bool MergingTypeTableBuilder::contains(TypeIndex Index) {
  if (Index.isSimple() || Index.isNoneType())
    return false;

  return Index.toArrayIndex() < SeenRecords.size();
}

uint32_t MergingTypeTableBuilder::size() { return SeenRecords.size(); }

uint32_t MergingTypeTableBuilder::capacity() { return SeenRecords.size(); }

ArrayRef<ArrayRef<uint8_t>> MergingTypeTableBuilder::records() const {
  return SeenRecords;
}

void MergingTypeTableBuilder::reset() {
  HashedRecords.clear();
  SeenRecords.clear();
}

static inline ArrayRef<uint8_t> stabilize(BumpPtrAllocator &Alloc,
                                          ArrayRef<uint8_t> Data) {
  uint8_t *Stable = Alloc.Allocate<uint8_t>(Data.size());
  memcpy(Stable, Data.data(), Data.size());
  return ArrayRef(Stable, Data.size());
}

TypeIndex MergingTypeTableBuilder::insertRecordAs(hash_code Hash,
                                                  ArrayRef<uint8_t> &Record) {
  assert(Record.size() < UINT32_MAX && "Record too big");
  assert(Record.size() % 4 == 0 &&
         "The type record size is not a multiple of 4 bytes which will cause "
         "misalignment in the output TPI stream!");

  LocallyHashedType WeakHash{Hash, Record};
  auto Result = HashedRecords.try_emplace(WeakHash, nextTypeIndex());

  if (Result.second) {
    ArrayRef<uint8_t> RecordData = stabilize(RecordStorage, Record);
    Result.first->first.RecordData = RecordData;
    SeenRecords.push_back(RecordData);
  }

  // Update the caller's copy of Record to point a stable copy.
  TypeIndex ActualTI = Result.first->second;
  Record = SeenRecords[ActualTI.toArrayIndex()];
  return ActualTI;
}

TypeIndex
MergingTypeTableBuilder::insertRecordBytes(ArrayRef<uint8_t> &Record) {
  return insertRecordAs(hash_value(Record), Record);
}

TypeIndex
MergingTypeTableBuilder::insertRecord(ContinuationRecordBuilder &Builder) {
  TypeIndex TI;
  auto Fragments = Builder.end(nextTypeIndex());
  assert(!Fragments.empty());
  for (auto C : Fragments)
    TI = insertRecordBytes(C.RecordData);
  return TI;
}

bool MergingTypeTableBuilder::replaceType(TypeIndex &Index, CVType Data,
                                          bool Stabilize) {
  assert(Index.toArrayIndex() < SeenRecords.size() &&
         "This function cannot be used to insert records!");

  ArrayRef<uint8_t> Record = Data.data();
  assert(Record.size() < UINT32_MAX && "Record too big");
  assert(Record.size() % 4 == 0 &&
         "The type record size is not a multiple of 4 bytes which will cause "
         "misalignment in the output TPI stream!");

  LocallyHashedType WeakHash{hash_value(Record), Record};
  auto Result = HashedRecords.try_emplace(WeakHash, Index.toArrayIndex());
  if (!Result.second) {
    Index = Result.first->second;
    return false; // The record is already there, at a different location
  }

  if (Stabilize) {
    Record = stabilize(RecordStorage, Record);
    Result.first->first.RecordData = Record;
  }

  SeenRecords[Index.toArrayIndex()] = Record;
  return true;
}
