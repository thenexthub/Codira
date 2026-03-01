//===- IndexedMemProfData.h - MemProf format support ------------*- C++ -*-===//
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
// MemProf data is serialized in writeMemProf provided in this file.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ProfileData/DataAccessProf.h"
#include "vm/core/ProfileData/InstrProf.h"
#include "vm/core/ProfileData/InstrProfReader.h"
#include "vm/core/ProfileData/MemProf.h"
#include "vm/core/ProfileData/MemProfRadixTree.h"
#include "vm/core/ProfileData/MemProfSummary.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/OnDiskHashTable.h"

namespace vm::core {

// Serialize Schema.
static void writeMemProfSchema(ProfOStream &OS,
                               const memprof::MemProfSchema &Schema) {
  OS.write(static_cast<uint64_t>(Schema.size()));
  for (const auto Id : Schema)
    OS.write(static_cast<uint64_t>(Id));
}

// Serialize MemProfRecordData.  Return RecordTableOffset.
static uint64_t writeMemProfRecords(
    ProfOStream &OS,
    toolchain::MapVector<GlobalValue::GUID, memprof::IndexedMemProfRecord>
        &MemProfRecordData,
    memprof::MemProfSchema *Schema, memprof::IndexedVersion Version,
    toolchain::DenseMap<memprof::CallStackId, memprof::LinearCallStackId>
        *MemProfCallStackIndexes = nullptr) {
  memprof::RecordWriterTrait RecordWriter(Schema, Version,
                                          MemProfCallStackIndexes);
  OnDiskChainedHashTableGenerator<memprof::RecordWriterTrait>
      RecordTableGenerator;
  for (auto &[GUID, Record] : MemProfRecordData) {
    // Insert the key (func hash) and value (memprof record).
    RecordTableGenerator.insert(GUID, Record, RecordWriter);
  }
  // Release the memory of this MapVector as it is no longer needed.
  MemProfRecordData.clear();

  // The call to Emit invokes RecordWriterTrait::EmitData which destructs
  // the memprof record copies owned by the RecordTableGenerator. This works
  // because the RecordTableGenerator is not used after this point.
  return RecordTableGenerator.Emit(OS.OS, RecordWriter);
}

// Serialize MemProfFrameData.  Return FrameTableOffset.
static uint64_t writeMemProfFrames(
    ProfOStream &OS,
    toolchain::MapVector<memprof::FrameId, memprof::Frame> &MemProfFrameData) {
  OnDiskChainedHashTableGenerator<memprof::FrameWriterTrait>
      FrameTableGenerator;
  for (auto &[FrameId, Frame] : MemProfFrameData) {
    // Insert the key (frame id) and value (frame contents).
    FrameTableGenerator.insert(FrameId, Frame);
  }
  // Release the memory of this MapVector as it is no longer needed.
  MemProfFrameData.clear();

  return FrameTableGenerator.Emit(OS.OS);
}

// Serialize MemProfFrameData.  Return the mapping from FrameIds to their
// indexes within the frame array.
static toolchain::DenseMap<memprof::FrameId, memprof::LinearFrameId>
writeMemProfFrameArray(
    ProfOStream &OS,
    toolchain::MapVector<memprof::FrameId, memprof::Frame> &MemProfFrameData,
    toolchain::DenseMap<memprof::FrameId, memprof::FrameStat> &FrameHistogram) {
  // Mappings from FrameIds to array indexes.
  toolchain::DenseMap<memprof::FrameId, memprof::LinearFrameId> MemProfFrameIndexes;

  // Compute the order in which we serialize Frames.  The order does not matter
  // in terms of correctness, but we still compute it for deserialization
  // performance.  Specifically, if we serialize frequently used Frames one
  // after another, we have better cache utilization.  For two Frames that
  // appear equally frequently, we break a tie by serializing the one that tends
  // to appear earlier in call stacks.  We implement the tie-breaking mechanism
  // by computing the sum of indexes within call stacks for each Frame.  If we
  // still have a tie, then we just resort to compare two FrameIds, which is
  // just for stability of output.
  std::vector<std::pair<memprof::FrameId, const memprof::Frame *>> FrameIdOrder;
  FrameIdOrder.reserve(MemProfFrameData.size());
  for (const auto &[Id, Frame] : MemProfFrameData)
    FrameIdOrder.emplace_back(Id, &Frame);
  assert(MemProfFrameData.size() == FrameIdOrder.size());
  toolchain::sort(FrameIdOrder,
             [&](const std::pair<memprof::FrameId, const memprof::Frame *> &L,
                 const std::pair<memprof::FrameId, const memprof::Frame *> &R) {
               const auto &SL = FrameHistogram[L.first];
               const auto &SR = FrameHistogram[R.first];
               // Popular FrameIds should come first.
               if (SL.Count != SR.Count)
                 return SL.Count > SR.Count;
               // If they are equally popular, then the one that tends to appear
               // earlier in call stacks should come first.
               if (SL.PositionSum != SR.PositionSum)
                 return SL.PositionSum < SR.PositionSum;
               // Compare their FrameIds for sort stability.
               return L.first < R.first;
             });

  // Serialize all frames while creating mappings from linear IDs to FrameIds.
  uint64_t Index = 0;
  MemProfFrameIndexes.reserve(FrameIdOrder.size());
  for (const auto &[Id, F] : FrameIdOrder) {
    F->serialize(OS.OS);
    MemProfFrameIndexes.insert({Id, Index});
    ++Index;
  }
  assert(MemProfFrameData.size() == Index);
  assert(MemProfFrameData.size() == MemProfFrameIndexes.size());

  // Release the memory of this MapVector as it is no longer needed.
  MemProfFrameData.clear();

  return MemProfFrameIndexes;
}

static uint64_t writeMemProfCallStacks(
    ProfOStream &OS,
    toolchain::MapVector<memprof::CallStackId, toolchain::SmallVector<memprof::FrameId>>
        &MemProfCallStackData) {
  OnDiskChainedHashTableGenerator<memprof::CallStackWriterTrait>
      CallStackTableGenerator;
  for (auto &[CSId, CallStack] : MemProfCallStackData)
    CallStackTableGenerator.insert(CSId, CallStack);
  // Release the memory of this vector as it is no longer needed.
  MemProfCallStackData.clear();

  return CallStackTableGenerator.Emit(OS.OS);
}

static toolchain::DenseMap<memprof::CallStackId, memprof::LinearCallStackId>
writeMemProfCallStackArray(
    ProfOStream &OS,
    toolchain::MapVector<memprof::CallStackId, toolchain::SmallVector<memprof::FrameId>>
        &MemProfCallStackData,
    toolchain::DenseMap<memprof::FrameId, memprof::LinearFrameId>
        &MemProfFrameIndexes,
    toolchain::DenseMap<memprof::FrameId, memprof::FrameStat> &FrameHistogram,
    unsigned &NumElements) {
  toolchain::DenseMap<memprof::CallStackId, memprof::LinearCallStackId>
      MemProfCallStackIndexes;

  memprof::CallStackRadixTreeBuilder<memprof::FrameId> Builder;
  Builder.build(std::move(MemProfCallStackData), &MemProfFrameIndexes,
                FrameHistogram);
  for (auto I : Builder.getRadixArray())
    OS.write32(I);
  NumElements = Builder.getRadixArray().size();
  MemProfCallStackIndexes = Builder.takeCallStackPos();

  // Release the memory of this vector as it is no longer needed.
  MemProfCallStackData.clear();

  return MemProfCallStackIndexes;
}

// Write out MemProf Version2 as follows:
// uint64_t Version
// uint64_t RecordTableOffset = RecordTableGenerator.Emit
// uint64_t FramePayloadOffset = Offset for the frame payload
// uint64_t FrameTableOffset = FrameTableGenerator.Emit
// uint64_t CallStackPayloadOffset = Offset for the call stack payload (NEW V2)
// uint64_t CallStackTableOffset = CallStackTableGenerator.Emit (NEW in V2)
// uint64_t Num schema entries
// uint64_t Schema entry 0
// uint64_t Schema entry 1
// ....
// uint64_t Schema entry N - 1
// OnDiskChainedHashTable MemProfRecordData
// OnDiskChainedHashTable MemProfFrameData
// OnDiskChainedHashTable MemProfCallStackData (NEW in V2)
static Error writeMemProfV2(ProfOStream &OS,
                            memprof::IndexedMemProfData &MemProfData,
                            bool MemProfFullSchema) {
  OS.write(memprof::Version2);
  uint64_t HeaderUpdatePos = OS.tell();
  OS.write(0ULL); // Reserve space for the memprof record table offset.
  OS.write(0ULL); // Reserve space for the memprof frame payload offset.
  OS.write(0ULL); // Reserve space for the memprof frame table offset.
  OS.write(0ULL); // Reserve space for the memprof call stack payload offset.
  OS.write(0ULL); // Reserve space for the memprof call stack table offset.

  auto Schema = memprof::getHotColdSchema();
  if (MemProfFullSchema)
    Schema = memprof::getFullSchema();
  writeMemProfSchema(OS, Schema);

  uint64_t RecordTableOffset =
      writeMemProfRecords(OS, MemProfData.Records, &Schema, memprof::Version2);

  uint64_t FramePayloadOffset = OS.tell();
  uint64_t FrameTableOffset = writeMemProfFrames(OS, MemProfData.Frames);

  uint64_t CallStackPayloadOffset = OS.tell();
  uint64_t CallStackTableOffset =
      writeMemProfCallStacks(OS, MemProfData.CallStacks);

  uint64_t Header[] = {
      RecordTableOffset,      FramePayloadOffset,   FrameTableOffset,
      CallStackPayloadOffset, CallStackTableOffset,
  };
  OS.patch({{HeaderUpdatePos, Header}});

  return Error::success();
}

static Error writeMemProfRadixTreeBased(
    ProfOStream &OS, memprof::IndexedMemProfData &MemProfData,
    memprof::IndexedVersion Version, bool MemProfFullSchema,
    std::unique_ptr<memprof::DataAccessProfData> DataAccessProfileData =
        nullptr,
    std::unique_ptr<memprof::MemProfSummary> MemProfSum = nullptr) {
  assert((Version == memprof::Version3 || Version == memprof::Version4) &&
         "Unsupported version for radix tree format");

  OS.write(Version); // Write the specific version (V3 or V4)
  uint64_t HeaderUpdatePos = OS.tell();
  OS.write(0ULL); // Reserve space for the memprof call stack payload offset.
  OS.write(0ULL); // Reserve space for the memprof record payload offset.
  OS.write(0ULL); // Reserve space for the memprof record table offset.
  if (Version >= memprof::Version4) {
    OS.write(0ULL); // Reserve space for the data access profile offset.

    MemProfSum->write(OS);
  }

  auto Schema = memprof::getHotColdSchema();
  if (MemProfFullSchema)
    Schema = memprof::getFullSchema();
  writeMemProfSchema(OS, Schema);

  toolchain::DenseMap<memprof::FrameId, memprof::FrameStat> FrameHistogram =
      memprof::computeFrameHistogram(MemProfData.CallStacks);
  assert(MemProfData.Frames.size() == FrameHistogram.size());

  toolchain::DenseMap<memprof::FrameId, memprof::LinearFrameId> MemProfFrameIndexes =
      writeMemProfFrameArray(OS, MemProfData.Frames, FrameHistogram);

  uint64_t CallStackPayloadOffset = OS.tell();
  // The number of elements in the call stack array.
  unsigned NumElements = 0;
  toolchain::DenseMap<memprof::CallStackId, memprof::LinearCallStackId>
      MemProfCallStackIndexes =
          writeMemProfCallStackArray(OS, MemProfData.CallStacks,
                                     MemProfFrameIndexes, FrameHistogram,
                                     NumElements);

  uint64_t RecordPayloadOffset = OS.tell();
  uint64_t RecordTableOffset = writeMemProfRecords(
      OS, MemProfData.Records, &Schema, Version, &MemProfCallStackIndexes);

  uint64_t DataAccessProfOffset = 0;
  if (DataAccessProfileData != nullptr) {
    assert(Version >= memprof::Version4 &&
           "Data access profiles are added starting from v4");
    DataAccessProfOffset = OS.tell();
    if (Error E = DataAccessProfileData->serialize(OS))
      return E;
  }

  // Verify that the computation for the number of elements in the call stack
  // array works.
  assert(CallStackPayloadOffset +
             NumElements * sizeof(memprof::LinearFrameId) ==
         RecordPayloadOffset);

  SmallVector<uint64_t, 4> Header = {
      CallStackPayloadOffset,
      RecordPayloadOffset,
      RecordTableOffset,
  };
  if (Version >= memprof::Version4)
    Header.push_back(DataAccessProfOffset);

  OS.patch({{HeaderUpdatePos, Header}});

  return Error::success();
}

// Write out MemProf Version3
static Error writeMemProfV3(ProfOStream &OS,
                            memprof::IndexedMemProfData &MemProfData,
                            bool MemProfFullSchema) {
  return writeMemProfRadixTreeBased(OS, MemProfData, memprof::Version3,
                                    MemProfFullSchema);
}

// Write out MemProf Version4
static Error writeMemProfV4(
    ProfOStream &OS, memprof::IndexedMemProfData &MemProfData,
    bool MemProfFullSchema,
    std::unique_ptr<memprof::DataAccessProfData> DataAccessProfileData,
    std::unique_ptr<memprof::MemProfSummary> MemProfSum) {
  return writeMemProfRadixTreeBased(
      OS, MemProfData, memprof::Version4, MemProfFullSchema,
      std::move(DataAccessProfileData), std::move(MemProfSum));
}

// Write out the MemProf data in a requested version.
Error writeMemProf(
    ProfOStream &OS, memprof::IndexedMemProfData &MemProfData,
    memprof::IndexedVersion MemProfVersionRequested, bool MemProfFullSchema,
    std::unique_ptr<memprof::DataAccessProfData> DataAccessProfileData,
    std::unique_ptr<memprof::MemProfSummary> MemProfSum) {
  switch (MemProfVersionRequested) {
  case memprof::Version2:
    return writeMemProfV2(OS, MemProfData, MemProfFullSchema);
  case memprof::Version3:
    return writeMemProfV3(OS, MemProfData, MemProfFullSchema);
  case memprof::Version4:
    return writeMemProfV4(OS, MemProfData, MemProfFullSchema,
                          std::move(DataAccessProfileData),
                          std::move(MemProfSum));
  }

  return make_error<InstrProfError>(
      instrprof_error::unsupported_version,
      formatv("MemProf version {} not supported; "
              "requires version between {} and {}, inclusive",
              MemProfVersionRequested, memprof::MinimumSupportedVersion,
              memprof::MaximumSupportedVersion));
}

Error IndexedMemProfReader::deserializeV2(const unsigned char *Start,
                                          const unsigned char *Ptr) {
  // The value returned from RecordTableGenerator.Emit.
  const uint64_t RecordTableOffset =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
  // The offset in the stream right before invoking
  // FrameTableGenerator.Emit.
  const uint64_t FramePayloadOffset =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
  // The value returned from FrameTableGenerator.Emit.
  const uint64_t FrameTableOffset =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);

  // The offset in the stream right before invoking
  // CallStackTableGenerator.Emit.
  uint64_t CallStackPayloadOffset = 0;
  // The value returned from CallStackTableGenerator.Emit.
  uint64_t CallStackTableOffset = 0;
  if (Version >= memprof::Version2) {
    CallStackPayloadOffset =
        support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
    CallStackTableOffset =
        support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
  }

  // Read the schema.
  auto SchemaOr = memprof::readMemProfSchema(Ptr);
  if (!SchemaOr)
    return SchemaOr.takeError();
  Schema = SchemaOr.get();

  // Now initialize the table reader with a pointer into data buffer.
  MemProfRecordTable.reset(MemProfRecordHashTable::Create(
      /*Buckets=*/Start + RecordTableOffset,
      /*Payload=*/Ptr,
      /*Base=*/Start, memprof::RecordLookupTrait(Version, Schema)));

  // Initialize the frame table reader with the payload and bucket offsets.
  MemProfFrameTable.reset(MemProfFrameHashTable::Create(
      /*Buckets=*/Start + FrameTableOffset,
      /*Payload=*/Start + FramePayloadOffset,
      /*Base=*/Start));

  if (Version >= memprof::Version2)
    MemProfCallStackTable.reset(MemProfCallStackHashTable::Create(
        /*Buckets=*/Start + CallStackTableOffset,
        /*Payload=*/Start + CallStackPayloadOffset,
        /*Base=*/Start));

  return Error::success();
}

Error IndexedMemProfReader::deserializeRadixTreeBased(
    const unsigned char *Start, const unsigned char *Ptr,
    memprof::IndexedVersion Version) {
  assert((Version == memprof::Version3 || Version == memprof::Version4) &&
         "Unsupported version for radix tree format");
  // The offset in the stream right before invoking
  // CallStackTableGenerator.Emit.
  const uint64_t CallStackPayloadOffset =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
  // The offset in the stream right before invoking RecordTableGenerator.Emit.
  const uint64_t RecordPayloadOffset =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
  // The value returned from RecordTableGenerator.Emit.
  const uint64_t RecordTableOffset =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);

  uint64_t DataAccessProfOffset = 0;
  if (Version >= memprof::Version4) {
    DataAccessProfOffset =
        support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);
    MemProfSum = memprof::MemProfSummary::deserialize(Ptr);

    if (DataAccessProfOffset > RecordTableOffset) {
      DataAccessProfileData = std::make_unique<memprof::DataAccessProfData>();
      const unsigned char *DAPPtr = Start + DataAccessProfOffset;
      if (Error E = DataAccessProfileData->deserialize(DAPPtr))
        return E;
      MemProfSum->buildDataAccessSummary(*DataAccessProfileData);
    }
  }

  assert((!DataAccessProfOffset || DataAccessProfOffset > RecordTableOffset) &&
         "Data access profile is either empty or after the record table");

  // Read the schema.
  auto SchemaOr = memprof::readMemProfSchema(Ptr);
  if (!SchemaOr)
    return SchemaOr.takeError();
  Schema = SchemaOr.get();

  FrameBase = Ptr;
  CallStackBase = Start + CallStackPayloadOffset;

  // Compute the number of elements in the radix tree array.  Since we use this
  // to reserve enough bits in a BitVector, it's totally OK if we overestimate
  // this number a little bit because of padding just before the next section.
  RadixTreeSize = (RecordPayloadOffset - CallStackPayloadOffset) /
                  sizeof(memprof::LinearFrameId);

  // Now initialize the table reader with a pointer into data buffer.
  MemProfRecordTable.reset(MemProfRecordHashTable::Create(
      /*Buckets=*/Start + RecordTableOffset,
      /*Payload=*/Start + RecordPayloadOffset,
      /*Base=*/Start, memprof::RecordLookupTrait(Version, Schema)));

  return Error::success();
}

Error IndexedMemProfReader::deserialize(const unsigned char *Start,
                                        uint64_t MemProfOffset) {
  const unsigned char *Ptr = Start + MemProfOffset;

  // Read the MemProf version number.
  const uint64_t FirstWord =
      support::endian::readNext<uint64_t, toolchain::endianness::little>(Ptr);

  // Check if the version is supported
  if (FirstWord >= memprof::MinimumSupportedVersion &&
      FirstWord <= memprof::MaximumSupportedVersion) {
    // Everything is good. We can proceed to deserialize the rest.
    Version = static_cast<memprof::IndexedVersion>(FirstWord);
  } else {
    return make_error<InstrProfError>(
        instrprof_error::unsupported_version,
        formatv("MemProf version {} not supported; "
                "requires version between {} and {}, inclusive",
                FirstWord, memprof::MinimumSupportedVersion,
                memprof::MaximumSupportedVersion));
  }

  switch (Version) {
  case memprof::Version2:
    if (Error E = deserializeV2(Start, Ptr))
      return E;
    break;
  case memprof::Version3:
  case memprof::Version4:
    // V3 and V4 share the same high-level structure (radix tree, linear IDs).
    if (Error E = deserializeRadixTreeBased(Start, Ptr, Version))
      return E;
    break;
  }

  return Error::success();
}
} // namespace vm::core
