//===- GlobalTypeTableBuilder.cpp -----------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/GlobalTypeTableBuilder.h"
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

TypeIndex GlobalTypeTableBuilder::nextTypeIndex() const {
  return TypeIndex::fromArrayIndex(SeenRecords.size());
}

GlobalTypeTableBuilder::GlobalTypeTableBuilder(BumpPtrAllocator &Storage)
    : RecordStorage(Storage) {
  SeenRecords.reserve(4096);
}

GlobalTypeTableBuilder::~GlobalTypeTableBuilder() = default;

std::optional<TypeIndex> GlobalTypeTableBuilder::getFirst() {
  if (empty())
    return std::nullopt;

  return TypeIndex(TypeIndex::FirstNonSimpleIndex);
}

std::optional<TypeIndex> GlobalTypeTableBuilder::getNext(TypeIndex Prev) {
  if (++Prev == nextTypeIndex())
    return std::nullopt;
  return Prev;
}

CVType GlobalTypeTableBuilder::getType(TypeIndex Index) {
  CVType Type(SeenRecords[Index.toArrayIndex()]);
  return Type;
}

StringRef GlobalTypeTableBuilder::getTypeName(TypeIndex Index) {
  llvm_unreachable("Method not implemented");
}

bool GlobalTypeTableBuilder::contains(TypeIndex Index) {
  if (Index.isSimple() || Index.isNoneType())
    return false;

  return Index.toArrayIndex() < SeenRecords.size();
}

uint32_t GlobalTypeTableBuilder::size() { return SeenRecords.size(); }

uint32_t GlobalTypeTableBuilder::capacity() { return SeenRecords.size(); }

ArrayRef<ArrayRef<uint8_t>> GlobalTypeTableBuilder::records() const {
  return SeenRecords;
}

ArrayRef<GloballyHashedType> GlobalTypeTableBuilder::hashes() const {
  return SeenHashes;
}

void GlobalTypeTableBuilder::reset() {
  HashedRecords.clear();
  SeenRecords.clear();
}

static inline ArrayRef<uint8_t> stabilize(BumpPtrAllocator &Alloc,
                                          ArrayRef<uint8_t> Data) {
  uint8_t *Stable = Alloc.Allocate<uint8_t>(Data.size());
  memcpy(Stable, Data.data(), Data.size());
  return ArrayRef(Stable, Data.size());
}

TypeIndex GlobalTypeTableBuilder::insertRecordBytes(ArrayRef<uint8_t> Record) {
  GloballyHashedType GHT =
      GloballyHashedType::hashType(Record, SeenHashes, SeenHashes);
  return insertRecordAs(GHT, Record.size(),
                        [Record](MutableArrayRef<uint8_t> Data) {
                          assert(Data.size() == Record.size());
                          ::memcpy(Data.data(), Record.data(), Record.size());
                          return Data;
                        });
}

TypeIndex
GlobalTypeTableBuilder::insertRecord(ContinuationRecordBuilder &Builder) {
  TypeIndex TI;
  auto Fragments = Builder.end(nextTypeIndex());
  assert(!Fragments.empty());
  for (auto C : Fragments)
    TI = insertRecordBytes(C.RecordData);
  return TI;
}

bool GlobalTypeTableBuilder::replaceType(TypeIndex &Index, CVType Data,
                                         bool Stabilize) {
  assert(Index.toArrayIndex() < SeenRecords.size() &&
         "This function cannot be used to insert records!");

  ArrayRef<uint8_t> Record = Data.data();
  assert(Record.size() < UINT32_MAX && "Record too big");
  assert(Record.size() % 4 == 0 &&
         "The type record size is not a multiple of 4 bytes which will cause "
         "misalignment in the output TPI stream!");

  GloballyHashedType Hash =
      GloballyHashedType::hashType(Record, SeenHashes, SeenHashes);
  auto Result = HashedRecords.try_emplace(Hash, Index.toArrayIndex());
  if (!Result.second) {
    Index = Result.first->second;
    return false; // The record is already there, at a different location
  }

  if (Stabilize)
    Record = stabilize(RecordStorage, Record);

  SeenRecords[Index.toArrayIndex()] = Record;
  SeenHashes[Index.toArrayIndex()] = Hash;
  return true;
}
