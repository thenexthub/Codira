//===- BinaryStream.h - Base interface for a stream of data -----*- C++ -*-===//
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

//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_BINARYSTREAM_H
#define TOOLCHAIN_SUPPORT_BINARYSTREAM_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/BitmaskEnum.h"
#include "toolchain/Support/BinaryStreamError.h"
#include "toolchain/Support/Endian.h"
#include "toolchain/Support/Error.h"
#include <cstdint>

namespace toolchain {

enum BinaryStreamFlags {
  BSF_None = 0,
  BSF_Write = 1,  // Stream supports writing.
  BSF_Append = 2, // Writing can occur at offset == length.
  TOOLCHAIN_MARK_AS_BITMASK_ENUM(/* LargestValue = */ BSF_Append)
};

/// An interface for accessing data in a stream-like format, but which
/// discourages copying.  Instead of specifying a buffer in which to copy
/// data on a read, the API returns an ArrayRef to data owned by the stream's
/// implementation.  Since implementations may not necessarily store data in a
/// single contiguous buffer (or even in memory at all), in such cases a it may
/// be necessary for an implementation to cache such a buffer so that it can
/// return it.
class BinaryStream {
public:
  virtual ~BinaryStream() = default;

  virtual toolchain::support::endianness getEndian() const = 0;

  /// Given an offset into the stream and a number of bytes, attempt to
  /// read the bytes and set the output ArrayRef to point to data owned by the
  /// stream.
  virtual Error readBytes(uint64_t Offset, uint64_t Size,
                          ArrayRef<uint8_t> &Buffer) = 0;

  /// Given an offset into the stream, read as much as possible without
  /// copying any data.
  virtual Error readLongestContiguousChunk(uint64_t Offset,
                                           ArrayRef<uint8_t> &Buffer) = 0;

  /// Return the number of bytes of data in this stream.
  virtual uint64_t getLength() = 0;

  /// Return the properties of this stream.
  virtual BinaryStreamFlags getFlags() const { return BSF_None; }

protected:
  Error checkOffsetForRead(uint64_t Offset, uint64_t DataSize) {
    if (Offset > getLength())
      return make_error<BinaryStreamError>(stream_error_code::invalid_offset);
    if (getLength() < DataSize + Offset)
      return make_error<BinaryStreamError>(stream_error_code::stream_too_short);
    return Error::success();
  }
};

/// A BinaryStream which can be read from as well as written to.  Note
/// that writing to a BinaryStream always necessitates copying from the input
/// buffer to the stream's backing store.  Streams are assumed to be buffered
/// so that to be portable it is necessary to call commit() on the stream when
/// all data has been written.
class WritableBinaryStream : public BinaryStream {
public:
  ~WritableBinaryStream() override = default;

  /// Attempt to write the given bytes into the stream at the desired
  /// offset. This will always necessitate a copy.  Cannot shrink or grow the
  /// stream, only writes into existing allocated space.
  virtual Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) = 0;

  /// For buffered streams, commits changes to the backing store.
  virtual Error commit() = 0;

  /// Return the properties of this stream.
  BinaryStreamFlags getFlags() const override { return BSF_Write; }

protected:
  Error checkOffsetForWrite(uint64_t Offset, uint64_t DataSize) {
    if (!(getFlags() & BSF_Append))
      return checkOffsetForRead(Offset, DataSize);

    if (Offset > getLength())
      return make_error<BinaryStreamError>(stream_error_code::invalid_offset);
    return Error::success();
  }
};

} // end namespace toolchain

#endif // TOOLCHAIN_SUPPORT_BINARYSTREAM_H
