//===- TpiStreamBuilder.cpp -   -------------------------------------------===//
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

#include "vm/core/DebugInfo/PDB/Native/TpiStreamBuilder.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/DebugInfo/CodeView/RecordSerialization.h"
#include "vm/core/DebugInfo/CodeView/TypeIndex.h"
#include "vm/core/DebugInfo/MSF/MSFBuilder.h"
#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/Support/Allocator.h"
#include "vm/core/Support/BinaryByteStream.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Endian.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/TimeProfiler.h"
#include <cstdint>
#include <numeric>

using namespace vm::core;
using namespace vm::core::msf;
using namespace vm::core::pdb;
using namespace vm::core::support;

TpiStreamBuilder::TpiStreamBuilder(MSFBuilder &Msf, uint32_t StreamIdx)
    : Msf(Msf), Allocator(Msf.getAllocator()), Header(nullptr), Idx(StreamIdx) {
}

TpiStreamBuilder::~TpiStreamBuilder() = default;

void TpiStreamBuilder::setVersionHeader(PdbRaw_TpiVer Version) {
  VerHeader = Version;
}

void TpiStreamBuilder::updateTypeIndexOffsets(ArrayRef<uint16_t> Sizes) {
  // If we just crossed an 8KB threshold, add a type index offset.
  for (uint16_t Size : Sizes) {
    size_t NewSize = TypeRecordBytes + Size;
    constexpr size_t EightKB = 8 * 1024;
    if (NewSize / EightKB > TypeRecordBytes / EightKB || TypeRecordCount == 0) {
      TypeIndexOffsets.push_back(
          {codeview::TypeIndex(codeview::TypeIndex::FirstNonSimpleIndex +
                               TypeRecordCount),
           ulittle32_t(TypeRecordBytes)});
    }
    ++TypeRecordCount;
    TypeRecordBytes = NewSize;
  }
}

void TpiStreamBuilder::addTypeRecord(ArrayRef<uint8_t> Record,
                                     std::optional<uint32_t> Hash) {
  assert(((Record.size() & 3) == 0) &&
         "The type record's size is not a multiple of 4 bytes which will "
         "cause misalignment in the output TPI stream!");
  assert(Record.size() <= codeview::MaxRecordLength);
  uint16_t OneSize = (uint16_t)Record.size();
  updateTypeIndexOffsets(ArrayRef(&OneSize, 1));

  TypeRecBuffers.push_back(Record);
  // FIXME: Require it.
  if (Hash)
    TypeHashes.push_back(*Hash);
}

void TpiStreamBuilder::addTypeRecords(ArrayRef<uint8_t> Types,
                                      ArrayRef<uint16_t> Sizes,
                                      ArrayRef<uint32_t> Hashes) {
  // Ignore empty type buffers. There should be no hashes or sizes in this case.
  if (Types.empty()) {
    assert(Sizes.empty() && Hashes.empty());
    return;
  }

  assert(((Types.size() & 3) == 0) &&
         "The type record's size is not a multiple of 4 bytes which will "
         "cause misalignment in the output TPI stream!");
  assert(Sizes.size() == Hashes.size() && "sizes and hashes should be in sync");
  assert(std::accumulate(Sizes.begin(), Sizes.end(), 0U) == Types.size() &&
         "sizes of type records should sum to the size of the types");
  updateTypeIndexOffsets(Sizes);

  TypeRecBuffers.push_back(Types);
  toolchain::append_range(TypeHashes, Hashes);
}

Error TpiStreamBuilder::finalize() {
  if (Header)
    return Error::success();

  TpiStreamHeader *H = Allocator.Allocate<TpiStreamHeader>();

  H->Version = VerHeader;
  H->HeaderSize = sizeof(TpiStreamHeader);
  H->TypeIndexBegin = codeview::TypeIndex::FirstNonSimpleIndex;
  H->TypeIndexEnd = H->TypeIndexBegin + TypeRecordCount;
  H->TypeRecordBytes = TypeRecordBytes;

  H->HashStreamIndex = HashStreamIndex;
  H->HashAuxStreamIndex = kInvalidStreamIndex;
  H->HashKeySize = sizeof(ulittle32_t);
  H->NumHashBuckets = MaxTpiHashBuckets - 1;

  // Recall that hash values go into a completely different stream identified by
  // the `HashStreamIndex` field of the `TpiStreamHeader`.  Therefore, the data
  // begins at offset 0 of this independent stream.
  H->HashValueBuffer.Off = 0;
  H->HashValueBuffer.Length = calculateHashBufferSize();

  // We never write any adjustments into our PDBs, so this is usually some
  // offset with zero length.
  H->HashAdjBuffer.Off = H->HashValueBuffer.Off + H->HashValueBuffer.Length;
  H->HashAdjBuffer.Length = 0;

  H->IndexOffsetBuffer.Off = H->HashAdjBuffer.Off + H->HashAdjBuffer.Length;
  H->IndexOffsetBuffer.Length = calculateIndexOffsetSize();

  Header = H;
  return Error::success();
}

uint32_t TpiStreamBuilder::calculateSerializedLength() {
  return sizeof(TpiStreamHeader) + TypeRecordBytes;
}

uint32_t TpiStreamBuilder::calculateHashBufferSize() const {
  assert((TypeRecordCount == TypeHashes.size() || TypeHashes.empty()) &&
         "either all or no type records should have hashes");
  return TypeHashes.size() * sizeof(ulittle32_t);
}

uint32_t TpiStreamBuilder::calculateIndexOffsetSize() const {
  return TypeIndexOffsets.size() * sizeof(codeview::TypeIndexOffset);
}

Error TpiStreamBuilder::finalizeMsfLayout() {
  uint32_t Length = calculateSerializedLength();
  if (auto EC = Msf.setStreamSize(Idx, Length))
    return EC;

  uint32_t HashStreamSize =
      calculateHashBufferSize() + calculateIndexOffsetSize();

  if (HashStreamSize == 0)
    return Error::success();

  auto ExpectedIndex = Msf.addStream(HashStreamSize);
  if (!ExpectedIndex)
    return ExpectedIndex.takeError();
  HashStreamIndex = *ExpectedIndex;
  if (!TypeHashes.empty()) {
    ulittle32_t *H = Allocator.Allocate<ulittle32_t>(TypeHashes.size());
    MutableArrayRef<ulittle32_t> HashBuffer(H, TypeHashes.size());
    for (uint32_t I = 0; I < TypeHashes.size(); ++I) {
      HashBuffer[I] = TypeHashes[I] % (MaxTpiHashBuckets - 1);
    }
    ArrayRef<uint8_t> Bytes(
        reinterpret_cast<const uint8_t *>(HashBuffer.data()),
        calculateHashBufferSize());
    HashValueStream =
        std::make_unique<BinaryByteStream>(Bytes, toolchain::endianness::little);
  }
  return Error::success();
}

Error TpiStreamBuilder::commit(const msf::MSFLayout &Layout,
                               WritableBinaryStreamRef Buffer) {
  toolchain::TimeTraceScope timeScope("Commit TPI stream");
  if (auto EC = finalize())
    return EC;

  auto InfoS = WritableMappedBlockStream::createIndexedStream(Layout, Buffer,
                                                              Idx, Allocator);

  BinaryStreamWriter Writer(*InfoS);
  if (auto EC = Writer.writeObject(*Header))
    return EC;

  for (auto Rec : TypeRecBuffers) {
    assert(!Rec.empty() && "Attempting to write an empty type record shifts "
                           "all offsets in the TPI stream!");
    assert(((Rec.size() & 3) == 0) &&
           "The type record's size is not a multiple of 4 bytes which will "
           "cause misalignment in the output TPI stream!");
    if (auto EC = Writer.writeBytes(Rec))
      return EC;
  }

  if (HashStreamIndex != kInvalidStreamIndex) {
    auto HVS = WritableMappedBlockStream::createIndexedStream(
        Layout, Buffer, HashStreamIndex, Allocator);
    BinaryStreamWriter HW(*HVS);
    if (HashValueStream) {
      if (auto EC = HW.writeStreamRef(*HashValueStream))
        return EC;
    }

    for (auto &IndexOffset : TypeIndexOffsets) {
      if (auto EC = HW.writeObject(IndexOffset))
        return EC;
    }
  }

  return Error::success();
}
