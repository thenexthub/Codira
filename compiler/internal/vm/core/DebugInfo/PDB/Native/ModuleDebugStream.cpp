//===- ModuleDebugStream.cpp - PDB Module Info Stream Access --------------===//
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

#include "vm/core/DebugInfo/PDB/Native/ModuleDebugStream.h"
#include "vm/core/ADT/iterator_range.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/DebugChecksumsSubsection.h"
#include "vm/core/DebugInfo/CodeView/SymbolRecordHelpers.h"
#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"
#include "vm/core/DebugInfo/PDB/Native/DbiModuleDescriptor.h"
#include "vm/core/DebugInfo/PDB/Native/RawConstants.h"
#include "vm/core/DebugInfo/PDB/Native/RawError.h"
#include "vm/core/Support/BinaryStreamArray.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamRef.h"
#include "vm/core/Support/Error.h"
#include <cstdint>

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::msf;
using namespace vm::core::pdb;

ModuleDebugStreamRef::ModuleDebugStreamRef(
    const DbiModuleDescriptor &Module,
    std::unique_ptr<MappedBlockStream> Stream)
    : Mod(Module), Stream(std::move(Stream)) {}

ModuleDebugStreamRef::~ModuleDebugStreamRef() = default;

Error ModuleDebugStreamRef::reload() {
  BinaryStreamReader Reader(*Stream);

  if (Mod.getModuleStreamIndex() != toolchain::pdb::kInvalidStreamIndex) {
    if (Error E = reloadSerialize(Reader))
      return E;
  }
  if (Reader.bytesRemaining() > 0)
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Unexpected bytes in module stream.");
  return Error::success();
}

Error ModuleDebugStreamRef::reloadSerialize(BinaryStreamReader &Reader) {
  uint32_t SymbolSize = Mod.getSymbolDebugInfoByteSize();
  uint32_t C11Size = Mod.getC11LineInfoByteSize();
  uint32_t C13Size = Mod.getC13LineInfoByteSize();

  if (C11Size > 0 && C13Size > 0)
    return make_error<RawError>(raw_error_code::corrupt_file,
                                "Module has both C11 and C13 line info");

  BinaryStreamRef S;

  if (auto EC = Reader.readInteger(Signature))
    return EC;
  Reader.setOffset(0);
  if (auto EC = Reader.readSubstream(SymbolsSubstream, SymbolSize))
    return EC;
  if (auto EC = Reader.readSubstream(C11LinesSubstream, C11Size))
    return EC;
  if (auto EC = Reader.readSubstream(C13LinesSubstream, C13Size))
    return EC;

  BinaryStreamReader SymbolReader(SymbolsSubstream.StreamData);
  if (auto EC = SymbolReader.readArray(
          SymbolArray, SymbolReader.bytesRemaining(), sizeof(uint32_t)))
    return EC;

  BinaryStreamReader SubsectionsReader(C13LinesSubstream.StreamData);
  if (auto EC = SubsectionsReader.readArray(Subsections,
                                            SubsectionsReader.bytesRemaining()))
    return EC;

  uint32_t GlobalRefsSize;
  if (auto EC = Reader.readInteger(GlobalRefsSize))
    return EC;
  if (auto EC = Reader.readSubstream(GlobalRefsSubstream, GlobalRefsSize))
    return EC;
  return Error::success();
}

const codeview::CVSymbolArray
ModuleDebugStreamRef::getSymbolArrayForScope(uint32_t ScopeBegin) const {
  return limitSymbolArrayToScope(SymbolArray, ScopeBegin);
}

BinarySubstreamRef ModuleDebugStreamRef::getSymbolsSubstream() const {
  return SymbolsSubstream;
}

BinarySubstreamRef ModuleDebugStreamRef::getC11LinesSubstream() const {
  return C11LinesSubstream;
}

BinarySubstreamRef ModuleDebugStreamRef::getC13LinesSubstream() const {
  return C13LinesSubstream;
}

BinarySubstreamRef ModuleDebugStreamRef::getGlobalRefsSubstream() const {
  return GlobalRefsSubstream;
}

iterator_range<codeview::CVSymbolArray::Iterator>
ModuleDebugStreamRef::symbols(bool *HadError) const {
  return make_range(SymbolArray.begin(HadError), SymbolArray.end());
}

CVSymbol ModuleDebugStreamRef::readSymbolAtOffset(uint32_t Offset) const {
  auto Iter = SymbolArray.at(Offset);
  assert(Iter != SymbolArray.end());
  return *Iter;
}

iterator_range<ModuleDebugStreamRef::DebugSubsectionIterator>
ModuleDebugStreamRef::subsections() const {
  return make_range(Subsections.begin(), Subsections.end());
}

bool ModuleDebugStreamRef::hasDebugSubsections() const {
  return !C13LinesSubstream.empty();
}

Error ModuleDebugStreamRef::commit() { return Error::success(); }

Expected<codeview::DebugChecksumsSubsectionRef>
ModuleDebugStreamRef::findChecksumsSubsection() const {
  codeview::DebugChecksumsSubsectionRef Result;
  for (const auto &SS : subsections()) {
    if (SS.kind() != DebugSubsectionKind::FileChecksums)
      continue;

    if (auto EC = Result.initialize(SS.getRecordData()))
      return std::move(EC);
    return Result;
  }
  return Result;
}
