//===- SymbolSerializer.cpp -----------------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/SymbolSerializer.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/Support/Error.h"
#include <cassert>
#include <cstdint>
#include <cstring>

using namespace vm::core;
using namespace vm::core::codeview;

SymbolSerializer::SymbolSerializer(BumpPtrAllocator &Allocator,
                                   CodeViewContainer Container)
    : Storage(Allocator), Stream(RecordBuffer, toolchain::endianness::little),
      Writer(Stream), Mapping(Writer, Container) {}

Error SymbolSerializer::visitSymbolBegin(CVSymbol &Record) {
  assert(!CurrentSymbol && "Already in a symbol mapping!");

  Writer.setOffset(0);

  if (auto EC = writeRecordPrefix(Record.kind()))
    return EC;

  CurrentSymbol = Record.kind();
  if (auto EC = Mapping.visitSymbolBegin(Record))
    return EC;

  return Error::success();
}

Error SymbolSerializer::visitSymbolEnd(CVSymbol &Record) {
  assert(CurrentSymbol && "Not in a symbol mapping!");

  if (auto EC = Mapping.visitSymbolEnd(Record))
    return EC;

  uint32_t RecordEnd = Writer.getOffset();
  uint16_t Length = RecordEnd - 2;
  Writer.setOffset(0);
  if (auto EC = Writer.writeInteger(Length))
    return EC;

  uint8_t *StableStorage = Storage.Allocate<uint8_t>(RecordEnd);
  ::memcpy(StableStorage, &RecordBuffer[0], RecordEnd);
  Record.RecordData = ArrayRef<uint8_t>(StableStorage, RecordEnd);
  CurrentSymbol.reset();

  return Error::success();
}
