//===- InjectedSourceStream.cpp - PDB Headerblock Stream Access -----------===//
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

#include "vm/core/DebugInfo/PDB/Native/InjectedSourceStream.h"

#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"
#include "vm/core/DebugInfo/PDB/Native/HashTable.h"
#include "vm/core/DebugInfo/PDB/Native/PDBStringTable.h"
#include "vm/core/DebugInfo/PDB/Native/RawConstants.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/Support/BinaryStreamReader.h"

using namespace vm::core;
using namespace vm::core::msf;
using namespace vm::core::support;
using namespace vm::core::pdb;

InjectedSourceStream::InjectedSourceStream(
    std::unique_ptr<MappedBlockStream> Stream)
    : Stream(std::move(Stream)) {}

Error InjectedSourceStream::reload(const PDBStringTable &Strings) {
  BinaryStreamReader Reader(*Stream);

  if (auto EC = Reader.readObject(Header))
    return EC;

  if (Header->Version !=
      static_cast<uint32_t>(PdbRaw_SrcHeaderBlockVer::SrcVerOne))
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Invalid headerblock header version");

  if (auto EC = InjectedSourceTable.load(Reader))
    return EC;

  for (const auto& Entry : *this) {
    if (Entry.second.Size != sizeof(SrcHeaderBlockEntry))
      return make_error<RawError>(raw_error_code::corrupt_file,
                                  "Invalid headerbock entry size");
    if (Entry.second.Version !=
        static_cast<uint32_t>(PdbRaw_SrcHeaderBlockVer::SrcVerOne))
      return make_error<RawError>(raw_error_code::corrupt_file,
                                  "Invalid headerbock entry version");

    // Check that all name references are valid.
    auto Name = Strings.getStringForID(Entry.second.FileNI);
    if (!Name)
      return Name.takeError();
    auto ObjName = Strings.getStringForID(Entry.second.ObjNI);
    if (!ObjName)
      return ObjName.takeError();
    auto VName = Strings.getStringForID(Entry.second.VFileNI);
    if (!VName)
      return VName.takeError();
  }

  assert(Reader.bytesRemaining() == 0);
  return Error::success();
}
