//===- PDBStringTable.cpp - PDB String Table ---------------------*- C++-*-===//
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

#include "vm/core/DebugInfo/PDB/Native/PDBStringTable.h"

#include "vm/core/DebugInfo/PDB/Native/Hash.h"
#include "vm/core/DebugInfo/PDB/Native/RawError.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/Endian.h"

using namespace vm::core;
using namespace vm::core::support;
using namespace vm::core::pdb;

uint32_t PDBStringTable::getByteSize() const { return Header->ByteSize; }
uint32_t PDBStringTable::getNameCount() const { return NameCount; }
uint32_t PDBStringTable::getHashVersion() const { return Header->HashVersion; }
uint32_t PDBStringTable::getSignature() const { return Header->Signature; }

Error PDBStringTable::readHeader(BinaryStreamReader &Reader) {
  if (auto EC = Reader.readObject(Header))
    return EC;

  if (Header->Signature != PDBStringTableSignature)
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Invalid hash table signature");
  if (Header->HashVersion != 1 && Header->HashVersion != 2)
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Unsupported hash version");

  assert(Reader.bytesRemaining() == 0);
  return Error::success();
}

Error PDBStringTable::readStrings(BinaryStreamReader &Reader) {
  BinaryStreamRef Stream;
  if (auto EC = Reader.readStreamRef(Stream))
    return EC;

  if (auto EC = Strings.initialize(Stream)) {
    return joinErrors(std::move(EC),
                      make_error<RawError>(raw_error_code::corrupt_file,
                                           "Invalid hash table byte length"));
  }

  assert(Reader.bytesRemaining() == 0);
  return Error::success();
}

const codeview::DebugStringTableSubsectionRef &
PDBStringTable::getStringTable() const {
  return Strings;
}

Error PDBStringTable::readHashTable(BinaryStreamReader &Reader) {
  const support::ulittle32_t *HashCount;
  if (auto EC = Reader.readObject(HashCount))
    return EC;

  if (auto EC = Reader.readArray(IDs, *HashCount)) {
    return joinErrors(std::move(EC),
                      make_error<RawError>(raw_error_code::corrupt_file,
                                           "Could not read bucket array"));
  }

  return Error::success();
}

Error PDBStringTable::readEpilogue(BinaryStreamReader &Reader) {
  if (auto EC = Reader.readInteger(NameCount))
    return EC;

  assert(Reader.bytesRemaining() == 0);
  return Error::success();
}

Error PDBStringTable::reload(BinaryStreamReader &Reader) {

  BinaryStreamReader SectionReader;

  std::tie(SectionReader, Reader) = Reader.split(sizeof(PDBStringTableHeader));
  if (auto EC = readHeader(SectionReader))
    return EC;

  std::tie(SectionReader, Reader) = Reader.split(Header->ByteSize);
  if (auto EC = readStrings(SectionReader))
    return EC;

  // We don't know how long the hash table is until we parse it, so let the
  // function responsible for doing that figure it out.
  if (auto EC = readHashTable(Reader))
    return EC;

  std::tie(SectionReader, Reader) = Reader.split(sizeof(uint32_t));
  if (auto EC = readEpilogue(SectionReader))
    return EC;

  assert(Reader.bytesRemaining() == 0);
  return Error::success();
}

Expected<StringRef> PDBStringTable::getStringForID(uint32_t ID) const {
  return Strings.getString(ID);
}

Expected<uint32_t> PDBStringTable::getIDForString(StringRef Str) const {
  uint32_t Hash =
      (Header->HashVersion == 1) ? hashStringV1(Str) : hashStringV2(Str);
  size_t Count = IDs.size();
  uint32_t Start = Hash % Count;
  for (size_t I = 0; I < Count; ++I) {
    // The hash is just a starting point for the search, but if it
    // doesn't work we should find the string no matter what, because
    // we iterate the entire array.
    uint32_t Index = (Start + I) % Count;

    // If we find 0, it means the item isn't in the hash table.
    uint32_t ID = IDs[Index];
    if (ID == 0)
      return make_error<RawError>(raw_error_code::no_entry);
    auto ExpectedStr = getStringForID(ID);
    if (!ExpectedStr)
      return ExpectedStr.takeError();

    if (*ExpectedStr == Str)
      return ID;
  }
  return make_error<RawError>(raw_error_code::no_entry);
}

FixedStreamArray<support::ulittle32_t> PDBStringTable::name_ids() const {
  return IDs;
}
