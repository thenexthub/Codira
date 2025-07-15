//===--- ExponentialGrowthAppendingBinaryByteStream.h -----------*- C++ -*-===//
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
//
/// Defines an \c toolchain::WritableBinaryStream whose internal buffer grows
/// exponentially in size as data is written to it. It is thus more efficient
/// than toolchain::AppendingBinaryByteStream if a lot of small data gets appended to
/// it.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_EXPONENTIALGROWTHAPPENDINGBINARYBYTESTREAM_H
#define LANGUAGE_BASIC_EXPONENTIALGROWTHAPPENDINGBINARYBYTESTREAM_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/Support/BinaryByteStream.h"
#include "toolchain/Support/Endian.h"

namespace language {

/// An implementation of WritableBinaryStream which can write at its end
/// causing the underlying data to grow.  This class owns the underlying data.
class ExponentialGrowthAppendingBinaryByteStream
    : public toolchain::WritableBinaryStream {
  /// The buffer holding the data.
  SmallVector<uint8_t, 0> Data;

  /// Data in the stream is always encoded in little-endian byte order.
  const toolchain::endianness Endian = toolchain::endianness::little;

public:
  ExponentialGrowthAppendingBinaryByteStream() = default;

  void reserve(size_t Size);

  toolchain::endianness getEndian() const override { return Endian; }

  toolchain::Error readBytes(uint64_t Offset, uint64_t Size,
                        ArrayRef<uint8_t> &Buffer) override;

  toolchain::Error readLongestContiguousChunk(uint64_t Offset,
                                         ArrayRef<uint8_t> &Buffer) override;

  MutableArrayRef<uint8_t> data() { return Data; }

  uint64_t getLength() override { return Data.size(); }

  toolchain::Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Buffer) override;

  /// This is an optimized version of \c writeBytes specifically for integers.
  /// Integers are written in little-endian byte order.
  template<typename T>
  toolchain::Error writeInteger(uint64_t Offset, T Value) {
    static_assert(std::is_integral<T>::value, "Integer required.");
    if (auto Error = checkOffsetForWrite(Offset, sizeof(T))) {
      return Error;
    }

    // Resize the internal buffer if needed.
    uint64_t RequiredSize = Offset + sizeof(T);
    if (RequiredSize > Data.size()) {
      Data.resize(RequiredSize);
    }

    toolchain::support::endian::write<T, toolchain::support::unaligned>(
      Data.data() + Offset, Value, Endian);

    return toolchain::Error::success();
  }

  toolchain::Error commit() override { return toolchain::Error::success(); }

  virtual toolchain::BinaryStreamFlags getFlags() const override {
    return toolchain::BSF_Write | toolchain::BSF_Append;
  }
};

} // end namespace language

#endif // LANGUAGE_BASIC_EXPONENTIALGROWTHAPPENDINGBINARYBYTESTREAM_H
