//===- BinaryStreamRef.cpp - ----------------------------------------------===//
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

#include "vm/core/Support/BinaryStreamRef.h"
#include "vm/core/Support/BinaryByteStream.h"

using namespace vm::core;

namespace {

class ArrayRefImpl : public BinaryStream {
public:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  toolchain::endianness getEndian() const override { return BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    return BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    return BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { return BBS.getLength(); }

private:
  BinaryByteStream BBS;
};

class MutableArrayRefImpl : public WritableBinaryStream {
public:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  toolchain::endianness getEndian() const override { return BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    return BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    return BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { return BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    return BBS.writeBytes(Offset, Data);
  }
  Error commit() override { return BBS.commit(); }

private:
  MutableBinaryByteStream BBS;
};
} // namespace

BinaryStreamRef::BinaryStreamRef(BinaryStream &Stream)
    : BinaryStreamRefBase(Stream) {}
BinaryStreamRef::BinaryStreamRef(BinaryStream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : BinaryStreamRefBase(Stream, Offset, Length) {}
BinaryStreamRef::BinaryStreamRef(ArrayRef<uint8_t> Data, endianness Endian)
    : BinaryStreamRefBase(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
BinaryStreamRef::BinaryStreamRef(StringRef Data, endianness Endian)
    : BinaryStreamRef(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error BinaryStreamRef::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) const {
  if (auto EC = checkOffsetForRead(Offset, Size))
    return EC;
  return BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error BinaryStreamRef::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) const {
  if (auto EC = checkOffsetForRead(Offset, 1))
    return EC;

  if (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    return EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  if (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  return Error::success();
}

WritableBinaryStreamRef::WritableBinaryStreamRef(WritableBinaryStream &Stream)
    : BinaryStreamRefBase(Stream) {}

WritableBinaryStreamRef::WritableBinaryStreamRef(WritableBinaryStream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : BinaryStreamRefBase(Stream, Offset, Length) {}

WritableBinaryStreamRef::WritableBinaryStreamRef(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : BinaryStreamRefBase(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error WritableBinaryStreamRef::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) const {
  if (auto EC = checkOffsetForWrite(Offset, Data.size()))
    return EC;

  return BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

WritableBinaryStreamRef::operator BinaryStreamRef() const {
  return BinaryStreamRef(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error WritableBinaryStreamRef::commit() { return BorrowedImpl->commit(); }
