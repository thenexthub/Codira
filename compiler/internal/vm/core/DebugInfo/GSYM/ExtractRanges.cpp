//===- ExtractRanges.cpp ----------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/GSYM/ExtractRanges.h"
#include "vm/core/DebugInfo/GSYM/FileWriter.h"
#include "vm/core/Support/DataExtractor.h"
#include <inttypes.h>

namespace vm::core {
namespace gsym {

void encodeRange(const AddressRange &Range, FileWriter &O, uint64_t BaseAddr) {
  assert(Range.start() >= BaseAddr);
  O.writeULEB(Range.start() - BaseAddr);
  O.writeULEB(Range.size());
}

AddressRange decodeRange(DataExtractor &Data, uint64_t BaseAddr,
                         uint64_t &Offset) {
  const uint64_t AddrOffset = Data.getULEB128(&Offset);
  const uint64_t Size = Data.getULEB128(&Offset);
  const uint64_t StartAddr = BaseAddr + AddrOffset;

  return {StartAddr, StartAddr + Size};
}

void encodeRanges(const AddressRanges &Ranges, FileWriter &O,
                  uint64_t BaseAddr) {
  O.writeULEB(Ranges.size());
  if (Ranges.empty())
    return;
  for (auto Range : Ranges)
    encodeRange(Range, O, BaseAddr);
}

void decodeRanges(AddressRanges &Ranges, DataExtractor &Data, uint64_t BaseAddr,
                  uint64_t &Offset) {
  Ranges.clear();
  uint64_t NumRanges = Data.getULEB128(&Offset);
  Ranges.reserve(NumRanges);
  for (uint64_t RangeIdx = 0; RangeIdx < NumRanges; RangeIdx++)
    Ranges.insert(decodeRange(Data, BaseAddr, Offset));
}

void skipRange(DataExtractor &Data, uint64_t &Offset) {
  Data.getULEB128(&Offset);
  Data.getULEB128(&Offset);
}

uint64_t skipRanges(DataExtractor &Data, uint64_t &Offset) {
  uint64_t NumRanges = Data.getULEB128(&Offset);
  for (uint64_t I = 0; I < NumRanges; ++I)
    skipRange(Data, Offset);
  return NumRanges;
}

} // namespace gsym

raw_ostream &operator<<(raw_ostream &OS, const AddressRange &R) {
  return OS << '[' << HEX64(R.start()) << " - " << HEX64(R.end()) << ")";
}

raw_ostream &operator<<(raw_ostream &OS, const AddressRanges &AR) {
  size_t Size = AR.size();
  for (size_t I = 0; I < Size; ++I) {
    if (I)
      OS << ' ';
    OS << AR[I];
  }
  return OS;
}

} // namespace vm::core
