//===- TypeHashing.cpp -------------------------------------------*- C++-*-===//
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

#include "vm/core/DebugInfo/CodeView/TypeHashing.h"

#include "vm/core/DebugInfo/CodeView/TypeIndexDiscovery.h"
#include "vm/core/Support/BLAKE3.h"

using namespace vm::core;
using namespace vm::core::codeview;

LocallyHashedType DenseMapInfo<LocallyHashedType>::Empty{0, {}};
LocallyHashedType DenseMapInfo<LocallyHashedType>::Tombstone{hash_code(-1), {}};

static std::array<uint8_t, 8> EmptyHash = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
static std::array<uint8_t, 8> TombstoneHash = {
    {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

GloballyHashedType DenseMapInfo<GloballyHashedType>::Empty{EmptyHash};
GloballyHashedType DenseMapInfo<GloballyHashedType>::Tombstone{TombstoneHash};

LocallyHashedType LocallyHashedType::hashType(ArrayRef<uint8_t> RecordData) {
  return {toolchain::hash_value(RecordData), RecordData};
}

GloballyHashedType
GloballyHashedType::hashType(ArrayRef<uint8_t> RecordData,
                             ArrayRef<GloballyHashedType> PreviousTypes,
                             ArrayRef<GloballyHashedType> PreviousIds) {
  SmallVector<TiReference, 4> Refs;
  discoverTypeIndices(RecordData, Refs);
  TruncatedBLAKE3<8> S;
  S.init();
  uint32_t Off = 0;
  S.update(RecordData.take_front(sizeof(RecordPrefix)));
  RecordData = RecordData.drop_front(sizeof(RecordPrefix));
  for (const auto &Ref : Refs) {
    // Hash any data that comes before this TiRef.
    uint32_t PreLen = Ref.Offset - Off;
    ArrayRef<uint8_t> PreData = RecordData.slice(Off, PreLen);
    S.update(PreData);
    auto Prev = (Ref.Kind == TiRefKind::IndexRef) ? PreviousIds : PreviousTypes;

    auto RefData = RecordData.slice(Ref.Offset, Ref.Count * sizeof(TypeIndex));
    // For each type index referenced, add in the previously computed hash
    // value of that type.
    ArrayRef<TypeIndex> Indices(
        reinterpret_cast<const TypeIndex *>(RefData.data()), Ref.Count);
    for (TypeIndex TI : Indices) {
      ArrayRef<uint8_t> BytesToHash;
      if (TI.isSimple() || TI.isNoneType()) {
        const uint8_t *IndexBytes = reinterpret_cast<const uint8_t *>(&TI);
        BytesToHash = ArrayRef(IndexBytes, sizeof(TypeIndex));
      } else {
        if (TI.toArrayIndex() >= Prev.size() ||
            Prev[TI.toArrayIndex()].empty()) {
          // There are references to yet-unhashed records. Suspend hashing for
          // this record until all the other records are processed.
          return {};
        }
        BytesToHash = Prev[TI.toArrayIndex()].Hash;
      }
      S.update(BytesToHash);
    }

    Off = Ref.Offset + Ref.Count * sizeof(TypeIndex);
  }

  // Don't forget to add in any trailing bytes.
  auto TrailingBytes = RecordData.drop_front(Off);
  S.update(TrailingBytes);

  return {S.final()};
}
