//===- TpiHashing.cpp -----------------------------------------------------===//
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

#include "vm/core/DebugInfo/PDB/Native/TpiHashing.h"

#include "vm/core/DebugInfo/CodeView/TypeDeserializer.h"
#include "vm/core/DebugInfo/PDB/Native/Hash.h"
#include "vm/core/Support/CRC.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

// Corresponds to `fUDTAnon`.
static bool isAnonymous(StringRef Name) {
  return Name == "<unnamed-tag>" || Name == "__unnamed" ||
         Name.ends_with("::<unnamed-tag>") || Name.ends_with("::__unnamed");
}

// Computes the hash for a user-defined type record. This could be a struct,
// class, union, or enum.
static uint32_t getHashForUdt(const TagRecord &Rec,
                              ArrayRef<uint8_t> FullRecord) {
  ClassOptions Opts = Rec.getOptions();
  bool ForwardRef = bool(Opts & ClassOptions::ForwardReference);
  bool Scoped = bool(Opts & ClassOptions::Scoped);
  bool HasUniqueName = bool(Opts & ClassOptions::HasUniqueName);
  bool IsAnon = HasUniqueName && isAnonymous(Rec.getName());

  if (!ForwardRef && !Scoped && !IsAnon)
    return hashStringV1(Rec.getName());
  if (!ForwardRef && HasUniqueName && !IsAnon)
    return hashStringV1(Rec.getUniqueName());
  return hashBufferV8(FullRecord);
}

template <typename T>
static Expected<uint32_t> getHashForUdt(const CVType &Rec) {
  T Deserialized;
  if (auto E = TypeDeserializer::deserializeAs(const_cast<CVType &>(Rec),
                                               Deserialized))
    return std::move(E);
  return getHashForUdt(Deserialized, Rec.data());
}

template <typename T>
static Expected<TagRecordHash> getTagRecordHashForUdt(const CVType &Rec) {
  T Deserialized;
  if (auto E = TypeDeserializer::deserializeAs(const_cast<CVType &>(Rec),
                                               Deserialized))
    return std::move(E);

  ClassOptions Opts = Deserialized.getOptions();

  bool ForwardRef = bool(Opts & ClassOptions::ForwardReference);

  uint32_t ThisRecordHash = getHashForUdt(Deserialized, Rec.data());

  // If we don't have a forward ref we can't compute the hash of it from the
  // full record because it requires hashing the entire buffer.
  if (!ForwardRef)
    return TagRecordHash{std::move(Deserialized), ThisRecordHash, 0};

  bool Scoped = bool(Opts & ClassOptions::Scoped);

  StringRef NameToHash =
      Scoped ? Deserialized.getUniqueName() : Deserialized.getName();
  uint32_t FullHash = hashStringV1(NameToHash);
  return TagRecordHash{std::move(Deserialized), FullHash, ThisRecordHash};
}

template <typename T>
static Expected<uint32_t> getSourceLineHash(const CVType &Rec) {
  T Deserialized;
  if (auto E = TypeDeserializer::deserializeAs(const_cast<CVType &>(Rec),
                                               Deserialized))
    return std::move(E);
  char Buf[4];
  support::endian::write32le(Buf, Deserialized.getUDT().getIndex());
  return hashStringV1(StringRef(Buf, 4));
}

Expected<TagRecordHash> toolchain::pdb::hashTagRecord(const codeview::CVType &Type) {
  switch (Type.kind()) {
  case LF_CLASS:
  case LF_STRUCTURE:
  case LF_INTERFACE:
    return getTagRecordHashForUdt<ClassRecord>(Type);
  case LF_UNION:
    return getTagRecordHashForUdt<UnionRecord>(Type);
  case LF_ENUM:
    return getTagRecordHashForUdt<EnumRecord>(Type);
  default:
    assert(false && "Type is not a tag record!");
  }
  return make_error<StringError>("Invalid record type",
                                 inconvertibleErrorCode());
}

Expected<uint32_t> toolchain::pdb::hashTypeRecord(const CVType &Rec) {
  switch (Rec.kind()) {
  case LF_CLASS:
  case LF_STRUCTURE:
  case LF_INTERFACE:
    return getHashForUdt<ClassRecord>(Rec);
  case LF_UNION:
    return getHashForUdt<UnionRecord>(Rec);
  case LF_ENUM:
    return getHashForUdt<EnumRecord>(Rec);

  case LF_UDT_SRC_LINE:
    return getSourceLineHash<UdtSourceLineRecord>(Rec);
  case LF_UDT_MOD_SRC_LINE:
    return getSourceLineHash<UdtModSourceLineRecord>(Rec);

  default:
    break;
  }

  // Run CRC32 over the bytes. This corresponds to `hashBufv8`.
  JamCRC JC(/*Init=*/0U);
  JC.update(Rec.data());
  return JC.getCRC();
}
