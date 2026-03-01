//===-- RecordSerialization.cpp -------------------------------------------===//
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
// Utilities for serializing and deserializing CodeView records.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/CodeView/RecordSerialization.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/APSInt.h"
#include "vm/core/DebugInfo/CodeView/CVRecord.h"
#include "vm/core/DebugInfo/CodeView/CodeViewError.h"
#include "vm/core/DebugInfo/CodeView/SymbolRecord.h"
#include "vm/core/Support/BinaryByteStream.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::support;

/// Reinterpret a byte array as an array of characters. Does not interpret as
/// a C string, as StringRef has several helpers (split) that make that easy.
StringRef toolchain::codeview::getBytesAsCharacters(ArrayRef<uint8_t> LeafData) {
  return StringRef(reinterpret_cast<const char *>(LeafData.data()),
                   LeafData.size());
}

StringRef toolchain::codeview::getBytesAsCString(ArrayRef<uint8_t> LeafData) {
  return getBytesAsCharacters(LeafData).split('\0').first;
}

Error toolchain::codeview::consume(BinaryStreamReader &Reader, APSInt &Num) {
  // Used to avoid overload ambiguity on APInt constructor.
  bool FalseVal = false;
  uint16_t Short;
  if (auto EC = Reader.readInteger(Short))
    return EC;

  if (Short < LF_NUMERIC) {
    Num = APSInt(APInt(/*numBits=*/16, Short, /*isSigned=*/false),
                 /*isUnsigned=*/true);
    return Error::success();
  }

  switch (Short) {
  case LF_CHAR: {
    int8_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(8, N, true), false);
    return Error::success();
  }
  case LF_SHORT: {
    int16_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(16, N, true), false);
    return Error::success();
  }
  case LF_USHORT: {
    uint16_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(16, N, false), true);
    return Error::success();
  }
  case LF_LONG: {
    int32_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(32, N, true), false);
    return Error::success();
  }
  case LF_ULONG: {
    uint32_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(32, N, FalseVal), true);
    return Error::success();
  }
  case LF_QUADWORD: {
    int64_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(64, N, true), false);
    return Error::success();
  }
  case LF_UQUADWORD: {
    uint64_t N;
    if (auto EC = Reader.readInteger(N))
      return EC;
    Num = APSInt(APInt(64, N, false), true);
    return Error::success();
  }
  }
  return make_error<CodeViewError>(cv_error_code::corrupt_record,
                                   "Buffer contains invalid APSInt type");
}

Error toolchain::codeview::consume(StringRef &Data, APSInt &Num) {
  ArrayRef<uint8_t> Bytes(Data.bytes_begin(), Data.bytes_end());
  BinaryByteStream S(Bytes, toolchain::endianness::little);
  BinaryStreamReader SR(S);
  auto EC = consume(SR, Num);
  Data = Data.take_back(SR.bytesRemaining());
  return EC;
}

/// Decode a numeric leaf value that is known to be a uint64_t.
Error toolchain::codeview::consume_numeric(BinaryStreamReader &Reader,
                                      uint64_t &Num) {
  APSInt N;
  if (auto EC = consume(Reader, N))
    return EC;
  if (N.isSigned() || !N.isIntN(64))
    return make_error<CodeViewError>(cv_error_code::corrupt_record,
                                     "Data is not a numeric value!");
  Num = N.getLimitedValue();
  return Error::success();
}

Error toolchain::codeview::consume(BinaryStreamReader &Reader, uint32_t &Item) {
  return Reader.readInteger(Item);
}

Error toolchain::codeview::consume(StringRef &Data, uint32_t &Item) {
  ArrayRef<uint8_t> Bytes(Data.bytes_begin(), Data.bytes_end());
  BinaryByteStream S(Bytes, toolchain::endianness::little);
  BinaryStreamReader SR(S);
  auto EC = consume(SR, Item);
  Data = Data.take_back(SR.bytesRemaining());
  return EC;
}

Error toolchain::codeview::consume(BinaryStreamReader &Reader, int32_t &Item) {
  return Reader.readInteger(Item);
}

Error toolchain::codeview::consume(BinaryStreamReader &Reader, StringRef &Item) {
  if (Reader.empty())
    return make_error<CodeViewError>(cv_error_code::corrupt_record,
                                     "Null terminated string buffer is empty!");

  return Reader.readCString(Item);
}

Expected<CVSymbol> toolchain::codeview::readSymbolFromStream(BinaryStreamRef Stream,
                                                        uint32_t Offset) {
  return readCVRecordFromStream<SymbolKind>(Stream, Offset);
}
