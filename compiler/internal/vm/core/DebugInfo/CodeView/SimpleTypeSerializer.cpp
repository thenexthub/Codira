//===- SimpleTypeSerializer.cpp -----------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/SimpleTypeSerializer.h"
#include "vm/core/DebugInfo/CodeView/CVRecord.h"
#include "vm/core/DebugInfo/CodeView/RecordSerialization.h"
#include "vm/core/DebugInfo/CodeView/TypeRecordMapping.h"
#include "vm/core/Support/BinaryStreamWriter.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;
using namespace vm::core::codeview;

static void addPadding(BinaryStreamWriter &Writer) {
  uint32_t Align = Writer.getOffset() % 4;
  if (Align == 0)
    return;

  int PaddingBytes = 4 - Align;
  while (PaddingBytes > 0) {
    uint8_t Pad = static_cast<uint8_t>(LF_PAD0 + PaddingBytes);
    cantFail(Writer.writeInteger(Pad));
    --PaddingBytes;
  }
}

SimpleTypeSerializer::SimpleTypeSerializer() : ScratchBuffer(MaxRecordLength) {}

SimpleTypeSerializer::~SimpleTypeSerializer() = default;

template <typename T>
ArrayRef<uint8_t> SimpleTypeSerializer::serialize(T &Record) {
  BinaryStreamWriter Writer(ScratchBuffer, toolchain::endianness::little);
  TypeRecordMapping Mapping(Writer);

  // Write the record prefix first with a dummy length but real kind.
  RecordPrefix DummyPrefix(uint16_t(Record.getKind()));
  cantFail(Writer.writeObject(DummyPrefix));

  RecordPrefix *Prefix = reinterpret_cast<RecordPrefix *>(ScratchBuffer.data());
  CVType CVT(Prefix, sizeof(RecordPrefix));

  cantFail(Mapping.visitTypeBegin(CVT));
  cantFail(Mapping.visitKnownRecord(CVT, Record));
  cantFail(Mapping.visitTypeEnd(CVT));

  addPadding(Writer);

  // Update the size and kind after serialization.
  Prefix->RecordKind = CVT.kind();
  Prefix->RecordLen = Writer.getOffset() - sizeof(uint16_t);

  return {ScratchBuffer.data(), static_cast<size_t>(Writer.getOffset())};
}

// Explicitly instantiate the member function for each known type so that we can
// implement this in the cpp file.
#define TYPE_RECORD(EnumName, EnumVal, Name)                                   \
  template LLVM_ABI ArrayRef<uint8_t>                                          \
  toolchain::codeview::SimpleTypeSerializer::serialize(Name##Record &Record);
#define TYPE_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)
#define MEMBER_RECORD(EnumName, EnumVal, Name)
#define MEMBER_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)
#include "vm/core/DebugInfo/CodeView/CodeViewTypes.def"
