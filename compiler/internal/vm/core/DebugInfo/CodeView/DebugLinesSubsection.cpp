//===- DebugLinesSubsection.cpp -------------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/DebugLinesSubsection.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/CodeViewError.h"
#include "vm/core/DebugInfo/CodeView/DebugChecksumsSubsection.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Error.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;
using namespace vm::core::codeview;

Error LineColumnExtractor::operator()(BinaryStreamRef Stream, uint32_t &Len,
                                      LineColumnEntry &Item) {
  const LineBlockFragmentHeader *BlockHeader;
  BinaryStreamReader Reader(Stream);
  if (auto EC = Reader.readObject(BlockHeader))
    return EC;
  bool HasColumn = Header->Flags & uint16_t(LF_HaveColumns);
  uint32_t LineInfoSize =
      BlockHeader->NumLines *
      (sizeof(LineNumberEntry) + (HasColumn ? sizeof(ColumnNumberEntry) : 0));
  if (BlockHeader->BlockSize < sizeof(LineBlockFragmentHeader))
    return make_error<CodeViewError>(cv_error_code::corrupt_record,
                                     "Invalid line block record size");
  uint32_t Size = BlockHeader->BlockSize - sizeof(LineBlockFragmentHeader);
  if (LineInfoSize > Size)
    return make_error<CodeViewError>(cv_error_code::corrupt_record,
                                     "Invalid line block record size");
  // The value recorded in BlockHeader->BlockSize includes the size of
  // LineBlockFragmentHeader.
  Len = BlockHeader->BlockSize;
  Item.NameIndex = BlockHeader->NameIndex;
  if (auto EC = Reader.readArray(Item.LineNumbers, BlockHeader->NumLines))
    return EC;
  if (HasColumn) {
    if (auto EC = Reader.readArray(Item.Columns, BlockHeader->NumLines))
      return EC;
  }
  return Error::success();
}

DebugLinesSubsectionRef::DebugLinesSubsectionRef()
    : DebugSubsectionRef(DebugSubsectionKind::Lines) {}

Error DebugLinesSubsectionRef::initialize(BinaryStreamReader Reader) {
  if (auto EC = Reader.readObject(Header))
    return EC;

  LinesAndColumns.getExtractor().Header = Header;
  if (auto EC = Reader.readArray(LinesAndColumns, Reader.bytesRemaining()))
    return EC;

  return Error::success();
}

bool DebugLinesSubsectionRef::hasColumnInfo() const {
  return !!(Header->Flags & LF_HaveColumns);
}

DebugLinesSubsection::DebugLinesSubsection(DebugChecksumsSubsection &Checksums,
                                           DebugStringTableSubsection &Strings)
    : DebugSubsection(DebugSubsectionKind::Lines), Checksums(Checksums) {}

void DebugLinesSubsection::createBlock(StringRef FileName) {
  uint32_t Offset = Checksums.mapChecksumOffset(FileName);

  Blocks.emplace_back(Offset);
}

void DebugLinesSubsection::addLineInfo(uint32_t Offset, const LineInfo &Line) {
  Block &B = Blocks.back();
  LineNumberEntry LNE;
  LNE.Flags = Line.getRawData();
  LNE.Offset = Offset;
  B.Lines.push_back(LNE);
}

void DebugLinesSubsection::addLineAndColumnInfo(uint32_t Offset,
                                                const LineInfo &Line,
                                                uint32_t ColStart,
                                                uint32_t ColEnd) {
  Block &B = Blocks.back();
  assert(B.Lines.size() == B.Columns.size());

  addLineInfo(Offset, Line);
  ColumnNumberEntry CNE;
  CNE.StartColumn = ColStart;
  CNE.EndColumn = ColEnd;
  B.Columns.push_back(CNE);
}

Error DebugLinesSubsection::commit(BinaryStreamWriter &Writer) const {
  LineFragmentHeader Header;
  Header.CodeSize = CodeSize;
  Header.Flags = hasColumnInfo() ? LF_HaveColumns : 0;
  Header.RelocOffset = RelocOffset;
  Header.RelocSegment = RelocSegment;

  if (auto EC = Writer.writeObject(Header))
    return EC;

  for (const auto &B : Blocks) {
    LineBlockFragmentHeader BlockHeader;
    assert(B.Lines.size() == B.Columns.size() || B.Columns.empty());

    BlockHeader.NumLines = B.Lines.size();
    BlockHeader.BlockSize = sizeof(LineBlockFragmentHeader);
    BlockHeader.BlockSize += BlockHeader.NumLines * sizeof(LineNumberEntry);
    if (hasColumnInfo())
      BlockHeader.BlockSize += BlockHeader.NumLines * sizeof(ColumnNumberEntry);
    BlockHeader.NameIndex = B.ChecksumBufferOffset;
    if (auto EC = Writer.writeObject(BlockHeader))
      return EC;

    if (auto EC = Writer.writeArray(ArrayRef(B.Lines)))
      return EC;

    if (hasColumnInfo()) {
      if (auto EC = Writer.writeArray(ArrayRef(B.Columns)))
        return EC;
    }
  }
  return Error::success();
}

uint32_t DebugLinesSubsection::calculateSerializedSize() const {
  uint32_t Size = sizeof(LineFragmentHeader);
  for (const auto &B : Blocks) {
    Size += sizeof(LineBlockFragmentHeader);
    Size += B.Lines.size() * sizeof(LineNumberEntry);
    if (hasColumnInfo())
      Size += B.Columns.size() * sizeof(ColumnNumberEntry);
  }
  return Size;
}

void DebugLinesSubsection::setRelocationAddress(uint16_t Segment,
                                                uint32_t Offset) {
  RelocOffset = Offset;
  RelocSegment = Segment;
}

void DebugLinesSubsection::setCodeSize(uint32_t Size) { CodeSize = Size; }

void DebugLinesSubsection::setFlags(LineFlags Flags) { this->Flags = Flags; }

bool DebugLinesSubsection::hasColumnInfo() const {
  return Flags & LF_HaveColumns;
}
