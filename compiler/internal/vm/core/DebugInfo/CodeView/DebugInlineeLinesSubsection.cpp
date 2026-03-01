//===- DebugInlineeLinesSubsection.cpp ------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/DebugInlineeLinesSubsection.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/DebugChecksumsSubsection.h"
#include "vm/core/DebugInfo/CodeView/RecordSerialization.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Endian.h"
#include "vm/core/Support/Error.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;
using namespace vm::core::codeview;

Error VarStreamArrayExtractor<InlineeSourceLine>::
operator()(BinaryStreamRef Stream, uint32_t &Len, InlineeSourceLine &Item) {
  BinaryStreamReader Reader(Stream);

  if (auto EC = Reader.readObject(Item.Header))
    return EC;

  if (HasExtraFiles) {
    uint32_t ExtraFileCount;
    if (auto EC = Reader.readInteger(ExtraFileCount))
      return EC;
    if (auto EC = Reader.readArray(Item.ExtraFiles, ExtraFileCount))
      return EC;
  }

  Len = Reader.getOffset();
  return Error::success();
}

DebugInlineeLinesSubsectionRef::DebugInlineeLinesSubsectionRef()
    : DebugSubsectionRef(DebugSubsectionKind::InlineeLines) {}

Error DebugInlineeLinesSubsectionRef::initialize(BinaryStreamReader Reader) {
  if (auto EC = Reader.readEnum(Signature))
    return EC;

  Lines.getExtractor().HasExtraFiles = hasExtraFiles();
  if (auto EC = Reader.readArray(Lines, Reader.bytesRemaining()))
    return EC;

  assert(Reader.bytesRemaining() == 0);
  return Error::success();
}

bool DebugInlineeLinesSubsectionRef::hasExtraFiles() const {
  return Signature == InlineeLinesSignature::ExtraFiles;
}

DebugInlineeLinesSubsection::DebugInlineeLinesSubsection(
    DebugChecksumsSubsection &Checksums, bool HasExtraFiles)
    : DebugSubsection(DebugSubsectionKind::InlineeLines), Checksums(Checksums),
      HasExtraFiles(HasExtraFiles) {}

uint32_t DebugInlineeLinesSubsection::calculateSerializedSize() const {
  // 4 bytes for the signature
  uint32_t Size = sizeof(InlineeLinesSignature);

  // one header for each entry.
  Size += Entries.size() * sizeof(InlineeSourceLineHeader);
  if (HasExtraFiles) {
    // If extra files are enabled, one count for each entry.
    Size += Entries.size() * sizeof(uint32_t);

    // And one file id for each file.
    Size += ExtraFileCount * sizeof(uint32_t);
  }
  assert(Size % 4 == 0);
  return Size;
}

Error DebugInlineeLinesSubsection::commit(BinaryStreamWriter &Writer) const {
  InlineeLinesSignature Sig = InlineeLinesSignature::Normal;
  if (HasExtraFiles)
    Sig = InlineeLinesSignature::ExtraFiles;

  if (auto EC = Writer.writeEnum(Sig))
    return EC;

  for (const auto &E : Entries) {
    if (auto EC = Writer.writeObject(E.Header))
      return EC;

    if (!HasExtraFiles)
      continue;

    if (auto EC = Writer.writeInteger<uint32_t>(E.ExtraFiles.size()))
      return EC;
    if (auto EC = Writer.writeArray(ArrayRef(E.ExtraFiles)))
      return EC;
  }

  return Error::success();
}

void DebugInlineeLinesSubsection::addExtraFile(StringRef FileName) {
  uint32_t Offset = Checksums.mapChecksumOffset(FileName);

  auto &Entry = Entries.back();
  Entry.ExtraFiles.push_back(ulittle32_t(Offset));
  ++ExtraFileCount;
}

void DebugInlineeLinesSubsection::addInlineSite(TypeIndex FuncId,
                                                StringRef FileName,
                                                uint32_t SourceLine) {
  uint32_t Offset = Checksums.mapChecksumOffset(FileName);

  Entries.emplace_back();
  auto &Entry = Entries.back();
  Entry.Header.FileID = Offset;
  Entry.Header.SourceLineNum = SourceLine;
  Entry.Header.Inlinee = FuncId;
}
