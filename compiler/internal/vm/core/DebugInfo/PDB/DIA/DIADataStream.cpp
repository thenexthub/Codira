//===- DIADataStream.cpp - DIA implementation of IPDBDataStream -*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIADataStream.h"
#include "vm/core/DebugInfo/PDB/DIA/DIAUtils.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIADataStream::DIADataStream(CComPtr<IDiaEnumDebugStreamData> DiaStreamData)
    : StreamData(DiaStreamData) {}

uint32_t DIADataStream::getRecordCount() const {
  LONG Count = 0;
  return (S_OK == StreamData->get_Count(&Count)) ? Count : 0;
}

std::string DIADataStream::getName() const {
  return invokeBstrMethod(*StreamData, &IDiaEnumDebugStreamData::get_name);
}

std::optional<DIADataStream::RecordType>
DIADataStream::getItemAtIndex(uint32_t Index) const {
  RecordType Record;
  DWORD RecordSize = 0;
  StreamData->Item(Index, 0, &RecordSize, nullptr);
  if (RecordSize == 0)
    return std::nullopt;

  Record.resize(RecordSize);
  if (S_OK != StreamData->Item(Index, RecordSize, &RecordSize, &Record[0]))
    return std::nullopt;
  return Record;
}

bool DIADataStream::getNext(RecordType &Record) {
  Record.clear();
  DWORD RecordSize = 0;
  ULONG CountFetched = 0;
  StreamData->Next(1, 0, &RecordSize, nullptr, &CountFetched);
  if (RecordSize == 0)
    return false;

  Record.resize(RecordSize);
  if (S_OK ==
      StreamData->Next(1, RecordSize, &RecordSize, &Record[0], &CountFetched))
    return false;
  return true;
}

void DIADataStream::reset() { StreamData->Reset(); }
