//===- SymbolStream.cpp - PDB Symbol Stream Access ------------------------===//
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

#include "vm/core/DebugInfo/PDB/Native/SymbolStream.h"

#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"

using namespace vm::core;
using namespace vm::core::msf;
using namespace vm::core::support;
using namespace vm::core::pdb;

SymbolStream::SymbolStream(std::unique_ptr<MappedBlockStream> Stream)
    : Stream(std::move(Stream)) {}

SymbolStream::~SymbolStream() = default;

Error SymbolStream::reload() {
  BinaryStreamReader Reader(*Stream);

  if (auto EC = Reader.readArray(SymbolRecords, Stream->getLength()))
    return EC;

  return Error::success();
}

iterator_range<codeview::CVSymbolArray::Iterator>
SymbolStream::getSymbols(bool *HadError) const {
  return toolchain::make_range(SymbolRecords.begin(HadError), SymbolRecords.end());
}

Error SymbolStream::commit() { return Error::success(); }

codeview::CVSymbol SymbolStream::readRecord(uint32_t Offset) const {
  return *SymbolRecords.at(Offset);
}
