//===- BinaryByteStream.h ---------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
===----------------------------------------------------------------------===//
// A BinaryStream which stores data in a single continguous memory buffer.
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_BINARYBYTESTREAM_H
#define TOOLCHAIN_SUPPORT_BINARYBYTESTREAM_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/BinaryStream.h"
#include "toolchain/Support/BinaryStreamError.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/FileOutputBuffer.h"
#include "toolchain/Support/MemoryBuffer.h"
#include <cstdint>
#include <cstring>
#include <memory>

namespace toolchain {

/// An implementation of BinaryStream which holds its entire data set
/// in a single contiguous buffer.  BinaryByteStream guarantees that no read
/// operation will ever incur a copy.  Note that BinaryByteStream does not
/// own the underlying buffer.
class BinaryByteStream : public BinaryStream {
public:
  BinaryByteStream() = default;
  BinaryByteStream(ArrayRef<uint8_t> Data, toolchain::support::endianness Endian)
      : Endian(Endian), Data(Data) {}
  BinaryByteStream(StringRef Data, toolchain::support::endianness Endian)
      : Endian(Endian), Data(Data.bytes_begin(), Data.bytes_end()) {}

  toolchain::support::endianness getEndian() const override { return Endian; }

  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    if (auto EC = checkOffsetForRead(Offset, Size))
      return EC;
    Buffer = Data.slice(Offset, Size);
    return Error::success();
  }

  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    if (auto EC = checkOffsetForRead(Offset, 1))
      return EC;
    Buffer = Data.slice(Offset);
    return Error::success();
  }

  uint64_t getLength() override { return Data.size(); }

  ArrayRef<uint8_t> data() const { return Data; }

  StringRef str() const {
    const char *CharData = reinterpret_cast<const char *>(Data.data());
    return StringRef(CharData, Data.size());
  }

protected:
  toolchain::support::endianness Endian;
  ArrayRef<uint8_t> Data;
};

/// An implementation of BinaryStream whose data is backed by an toolchain
/// MemoryBuffer object.  MemoryBufferByteStream owns the MemoryBuffer in
/// question.  As with BinaryByteStream, reading from a MemoryBufferByteStream
/// will never cause a copy.
class MemoryBufferByteStream : public BinaryByteStream {
public:
  MemoryBufferByteStream(std::unique_ptr<MemoryBuffer> Buffer,
                         toolchain::support::endianness Endian)
      : BinaryByteStream(Buffer->getBuffer(), Endian),
        MemBuffer(std::move(Buffer)) {}

  std::unique_ptr<MemoryBuffer> MemBuffer;
};

/// An implementation of BinaryStream which holds its entire data set
/// in a single contiguous buffer.  As with BinaryByteStream, the mutable
/// version also guarantees that no read operation will ever incur a copy,
/// and similarly it does not own the underlying buffer.
class MutableBinaryByteStream : public WritableBinaryStream {
public:
  MutableBinaryByteStream() = default;
  MutableBinaryByteStream(MutableArrayRef<uint8_t> Data,
                          toolchain::support::endianness Endian)
      : Data(Data), ImmutableStream(Data, Endian) {}

  toolchain::support::endianness getEndian() const override {
    return ImmutableStream.getEndian();
  }

  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    return ImmutableStream.readBytes(Offset, Size, Buffer);
  }

  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    return ImmutableStream.readLongestContiguousChunk(Offset, Buffer);
  }

  uint64_t getLength() override { return ImmutableStream.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Buffer) override {
    if (Buffer.empty())
      return Error::success();

    if (auto EC = checkOffsetForWrite(Offset, Buffer.size()))
      return EC;

    uint8_t *DataPtr = const_cast<uint8_t *>(Data.data());
    ::memcpy(DataPtr + Offset, Buffer.data(), Buffer.size());
    return Error::success();
  }

  Error commit() override { return Error::success(); }

  MutableArrayRef<uint8_t> data() const { return Data; }

private:
  MutableArrayRef<uint8_t> Data;
  BinaryByteStream ImmutableStream;
};

/// An implementation of WritableBinaryStream which can write at its end
/// causing the underlying data to grow.  This class owns the underlying data.
class AppendingBinaryByteStream : public WritableBinaryStream {
  std::vector<uint8_t> Data;
  toolchain::support::endianness Endian = toolchain::support::little;

public:
  AppendingBinaryByteStream() = default;
  AppendingBinaryByteStream(toolchain::support::endianness Endian)
      : Endian(Endian) {}

  void clear() { Data.clear(); }

  toolchain::support::endianness getEndian() const override { return Endian; }

  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    if (auto EC = checkOffsetForWrite(Offset, Buffer.size()))
      return EC;

    Buffer = ArrayRef(Data).slice(Offset, Size);
    return Error::success();
  }

  void insert(uint64_t Offset, ArrayRef<uint8_t> Bytes) {
    Data.insert(Data.begin() + Offset, Bytes.begin(), Bytes.end());
  }

  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    if (auto EC = checkOffsetForWrite(Offset, 1))
      return EC;

    Buffer = ArrayRef(Data).slice(Offset);
    return Error::success();
  }

  uint64_t getLength() override { return Data.size(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Buffer) override {
    if (Buffer.empty())
      return Error::success();

    // This is well-defined for any case except where offset is strictly
    // greater than the current length.  If offset is equal to the current
    // length, we can still grow.  If offset is beyond the current length, we
    // would have to decide how to deal with the intermediate uninitialized
    // bytes.  So we punt on that case for simplicity and just say it's an
    // error.
    if (Offset > getLength())
      return make_error<BinaryStreamError>(stream_error_code::invalid_offset);

    uint64_t RequiredSize = Offset + Buffer.size();
    if (RequiredSize > Data.size())
      Data.resize(RequiredSize);

    ::memcpy(Data.data() + Offset, Buffer.data(), Buffer.size());
    return Error::success();
  }

  Error commit() override { return Error::success(); }

  /// Return the properties of this stream.
  BinaryStreamFlags getFlags() const override { return BSF_Write | BSF_Append; }

  MutableArrayRef<uint8_t> data() { return Data; }
};

/// An implementation of WritableBinaryStream backed by an toolchain
/// FileOutputBuffer.
class FileBufferByteStream : public WritableBinaryStream {
private:
  class StreamImpl : public MutableBinaryByteStream {
  public:
    StreamImpl(std::unique_ptr<FileOutputBuffer> Buffer,
               toolchain::support::endianness Endian)
        : MutableBinaryByteStream(
              MutableArrayRef<uint8_t>(Buffer->getBufferStart(),
                                       Buffer->getBufferEnd()),
              Endian),
          FileBuffer(std::move(Buffer)) {}

    Error commit() override {
      if (FileBuffer->commit())
        return make_error<BinaryStreamError>(
            stream_error_code::filesystem_error);
      return Error::success();
    }

    /// Returns a pointer to the start of the buffer.
    uint8_t *getBufferStart() const { return FileBuffer->getBufferStart(); }

    /// Returns a pointer to the end of the buffer.
    uint8_t *getBufferEnd() const { return FileBuffer->getBufferEnd(); }

  private:
    std::unique_ptr<FileOutputBuffer> FileBuffer;
  };

public:
  FileBufferByteStream(std::unique_ptr<FileOutputBuffer> Buffer,
                       toolchain::support::endianness Endian)
      : Impl(std::move(Buffer), Endian) {}

  toolchain::support::endianness getEndian() const override {
    return Impl.getEndian();
  }

  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    return Impl.readBytes(Offset, Size, Buffer);
  }

  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    return Impl.readLongestContiguousChunk(Offset, Buffer);
  }

  uint64_t getLength() override { return Impl.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    return Impl.writeBytes(Offset, Data);
  }

  Error commit() override { return Impl.commit(); }

  /// Returns a pointer to the start of the buffer.
  uint8_t *getBufferStart() const { return Impl.getBufferStart(); }

  /// Returns a pointer to the end of the buffer.
  uint8_t *getBufferEnd() const { return Impl.getBufferEnd(); }

private:
  StreamImpl Impl;
};

} // end namespace toolchain

#endif // TOOLCHAIN_SUPPORT_BINARYBYTESTREAM_H
