//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
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

#include "vm/core/Support/BinaryStreamWriter.h"

#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/BinaryStreamReader.h"
#include "vm/core/Support/BinaryStreamRef.h"
#include "vm/core/Support/LEB128.h"

using namespace vm::core;

BinaryStreamWriter::BinaryStreamWriter(WritableBinaryStreamRef Ref)
    : Stream(Ref) {}

BinaryStreamWriter::BinaryStreamWriter(WritableBinaryStream &Stream)
    : Stream(Stream) {}

BinaryStreamWriter::BinaryStreamWriter(MutableArrayRef<uint8_t> Data,
                                       toolchain::endianness Endian)
    : Stream(Data, Endian) {}

Error BinaryStreamWriter::writeBytes(ArrayRef<uint8_t> Buffer) {
  if (auto EC = Stream.writeBytes(Offset, Buffer))
    return EC;
  Offset += Buffer.size();
  return Error::success();
}

Error BinaryStreamWriter::writeULEB128(uint64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(Value, &EncodedBytes[0]);
  return writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeSLEB128(int64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(Value, &EncodedBytes[0]);
  return writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeCString(StringRef Str) {
  if (auto EC = writeFixedString(Str))
    return EC;
  if (auto EC = writeObject('\0'))
    return EC;

  return Error::success();
}

Error BinaryStreamWriter::writeFixedString(StringRef Str) {

  return writeBytes(arrayRefFromStringRef(Str));
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref) {
  return writeStreamRef(Ref, Ref.getLength());
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref, uint64_t Length) {
  BinaryStreamReader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  while (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    if (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      return EC;
    if (auto EC = writeBytes(Chunk))
      return EC;
  }
  return Error::success();
}

std::pair<BinaryStreamWriter, BinaryStreamWriter>
BinaryStreamWriter::split(uint64_t Off) const {
  assert(getLength() >= Off);

  WritableBinaryStreamRef First = Stream.drop_front(Offset);

  WritableBinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamWriter W1{First};
  BinaryStreamWriter W2{Second};
  return {W1, W2};
}

Error BinaryStreamWriter::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  const uint64_t ZerosSize = 64;
  static constexpr char Zeros[ZerosSize] = {};
  while (Offset < NewOffset)
    if (auto E = writeArray(
            ArrayRef<char>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      return E;
  return Error::success();
}
