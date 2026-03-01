//===- PublicsStream.cpp - PDB Public Symbol Stream -----------------------===//
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
// The data structures defined in this file are based on the reference
// implementation which is available at
// https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/dbi/gsi.h
//
// When you are reading the reference source code, you'd find the
// information below useful.
//
//  - ppdb1->m_fMinimalDbgInfo seems to be always true.
//  - SMALLBUCKETS macro is defined.
//
// The reference doesn't compile, so I learned just by reading code.
// It's not guaranteed to be correct.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/PDB/Native/PublicsStream.h"
#include "vm/core/DebugInfo/CodeView/SymbolDeserializer.h"
#include "vm/core/DebugInfo/CodeView/SymbolRecord.h"
#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"
#include "vm/core/DebugInfo/PDB/Native/RawError.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolStream.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/Error.h"
#include <cstdint>

using namespace vm::core;
using namespace vm::core::msf;
using namespace vm::core::support;
using namespace vm::core::pdb;

PublicsStream::PublicsStream(std::unique_ptr<MappedBlockStream> Stream)
    : Stream(std::move(Stream)) {}

PublicsStream::~PublicsStream() = default;

uint32_t PublicsStream::getSymHash() const { return Header->SymHash; }
uint16_t PublicsStream::getThunkTableSection() const {
  return Header->ISectThunkTable;
}
uint32_t PublicsStream::getThunkTableOffset() const {
  return Header->OffThunkTable;
}

// Publics stream contains fixed-size headers and a serialized hash table.
// This implementation is not complete yet. It reads till the end of the
// stream so that we verify the stream is at least not corrupted. However,
// we skip over the hash table which we believe contains information about
// public symbols.
Error PublicsStream::reload() {
  BinaryStreamReader Reader(*Stream);

  // Check stream size.
  if (Reader.bytesRemaining() <
      sizeof(PublicsStreamHeader) + sizeof(GSIHashHeader))
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Publics Stream does not contain a header.");

  // Read PSGSIHDR struct.
  if (Reader.readObject(Header))
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Publics Stream does not contain a header.");

  // Read the hash table.
  if (auto E = PublicsTable.read(Reader))
    return E;

  // Something called "address map" follows.
  uint32_t NumAddressMapEntries = Header->AddrMap / sizeof(uint32_t);
  if (auto EC = Reader.readArray(AddressMap, NumAddressMapEntries))
    return joinErrors(std::move(EC),
                      make_error<RawError>(raw_error_code::corrupt_file,
                                           "Could not read an address map."));

  // Something called "thunk map" follows.
  if (auto EC = Reader.readArray(ThunkMap, Header->NumThunks))
    return joinErrors(std::move(EC),
                      make_error<RawError>(raw_error_code::corrupt_file,
                                           "Could not read a thunk map."));

  // Something called "section map" follows.
  if (Reader.bytesRemaining() > 0) {
    if (auto EC = Reader.readArray(SectionOffsets, Header->NumSections))
      return joinErrors(std::move(EC),
                        make_error<RawError>(raw_error_code::corrupt_file,
                                             "Could not read a section map."));
  }

  if (Reader.bytesRemaining() > 0)
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Corrupted publics stream.");
  return Error::success();
}

// This is a reimplementation of NearestSym:
// https://github.com/microsoft/microsoft-pdb/blob/805655a28bd8198004be2ac27e6e0290121a5e89/PDB/dbi/gsi.cpp#L1492-L1581
std::optional<std::pair<codeview::PublicSym32, size_t>>
PublicsStream::findByAddress(const SymbolStream &Symbols, uint16_t Segment,
                             uint32_t Offset) const {
  // The address map is sorted by address, so we can use lower_bound to find the
  // position. Each element is an offset into the symbols for a public symbol.
  auto It = toolchain::lower_bound(
      AddressMap, std::tuple(Segment, Offset),
      [&](support::ulittle32_t Cur, auto Addr) {
        auto Sym = Symbols.readRecord(Cur.value());
        if (Sym.kind() != codeview::S_PUB32)
          return false; // stop here, this is most likely corrupted debug info

        auto Psym =
            codeview::SymbolDeserializer::deserializeAs<codeview::PublicSym32>(
                Sym);
        if (!Psym) {
          consumeError(Psym.takeError());
          return false;
        }

        return std::tie(Psym->Segment, Psym->Offset) < Addr;
      });

  if (It == AddressMap.end())
    return std::nullopt;

  auto Sym = Symbols.readRecord(It->value());
  if (Sym.kind() != codeview::S_PUB32)
    return std::nullopt; // this is most likely corrupted debug info

  auto MaybePsym =
      codeview::SymbolDeserializer::deserializeAs<codeview::PublicSym32>(Sym);
  if (!MaybePsym) {
    consumeError(MaybePsym.takeError());
    return std::nullopt;
  }
  codeview::PublicSym32 Psym = std::move(*MaybePsym);

  if (std::tuple(Segment, Offset) != std::tuple(Psym.Segment, Psym.Offset))
    return std::nullopt;

  std::ptrdiff_t IterOffset = It - AddressMap.begin();
  return std::pair{Psym, static_cast<size_t>(IterOffset)};
}
