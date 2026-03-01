//===- DebugSubsectionRecord.cpp ------------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/DebugSubsectionRecord.h"
#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/DebugSubsection.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/MathExtras.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;
using namespace vm::core::codeview;

DebugSubsectionRecord::DebugSubsectionRecord() = default;

DebugSubsectionRecord::DebugSubsectionRecord(DebugSubsectionKind Kind,
                                             BinaryStreamRef Data)
    : Kind(Kind), Data(Data) {}

Error DebugSubsectionRecord::initialize(BinaryStreamRef Stream,
                                        DebugSubsectionRecord &Info) {
  const DebugSubsectionHeader *Header;
  BinaryStreamReader Reader(Stream);
  if (auto EC = Reader.readObject(Header))
    return EC;

  DebugSubsectionKind Kind =
      static_cast<DebugSubsectionKind>(uint32_t(Header->Kind));
  if (auto EC = Reader.readStreamRef(Info.Data, Header->Length))
    return EC;
  Info.Kind = Kind;
  return Error::success();
}

uint32_t DebugSubsectionRecord::getRecordLength() const {
  return sizeof(DebugSubsectionHeader) + Data.getLength();
}

DebugSubsectionKind DebugSubsectionRecord::kind() const { return Kind; }

BinaryStreamRef DebugSubsectionRecord::getRecordData() const { return Data; }

DebugSubsectionRecordBuilder::DebugSubsectionRecordBuilder(
    std::shared_ptr<DebugSubsection> Subsection)
    : Subsection(std::move(Subsection)) {}

DebugSubsectionRecordBuilder::DebugSubsectionRecordBuilder(
    const DebugSubsectionRecord &Contents)
    : Contents(Contents) {}

uint32_t DebugSubsectionRecordBuilder::calculateSerializedLength() const {
  uint32_t DataSize = Subsection ? Subsection->calculateSerializedSize()
                                 : Contents.getRecordData().getLength();
  // The length of the entire subsection is always padded to 4 bytes,
  // regardless of the container kind.
  return sizeof(DebugSubsectionHeader) + alignTo(DataSize, 4);
}

Error DebugSubsectionRecordBuilder::commit(BinaryStreamWriter &Writer,
                                           CodeViewContainer Container) const {
  assert(Writer.getOffset() % alignOf(Container) == 0 &&
         "Debug Subsection not properly aligned");

  DebugSubsectionHeader Header;
  Header.Kind = uint32_t(Subsection ? Subsection->kind() : Contents.kind());
  // The value written into the Header's Length field is only padded to the
  // container's alignment
  uint32_t DataSize = Subsection ? Subsection->calculateSerializedSize()
                                 : Contents.getRecordData().getLength();
  Header.Length = alignTo(DataSize, alignOf(Container));

  if (auto EC = Writer.writeObject(Header))
    return EC;
  if (Subsection) {
    if (auto EC = Subsection->commit(Writer))
      return EC;
  } else {
    if (auto EC = Writer.writeStreamRef(Contents.getRecordData()))
      return EC;
  }
  if (auto EC = Writer.padToAlignment(4))
    return EC;

  return Error::success();
}
