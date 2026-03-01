//===-- StreamBuffer.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_STREAMBUFFER_H
#define LLDB_CORE_STREAMBUFFER_H

#include "lldb/Utility/Stream.h"
#include "llvm/ADT/SmallVector.h"
#include <cstdio>
#include <string>

namespace lldb_private {

template <unsigned N> class StreamBuffer : public Stream {
public:
  StreamBuffer() : Stream(0, 4, lldb::eByteOrderBig), m_packet() {}

  StreamBuffer(uint32_t flags, uint32_t addr_size, lldb::ByteOrder byte_order)
      : Stream(flags, addr_size, byte_order), m_packet() {}

  ~StreamBuffer() override = default;

  void Flush() override {
    // Nothing to do when flushing a buffer based stream...
  }

  void Clear() { m_packet.clear(); }

  // Beware, this might not be NULL terminated as you can expect from
  // StringString as there may be random bits in the llvm::SmallVector. If you
  // are using this class to create a C string, be sure the call PutChar ('\0')
  // after you have created your string, or use StreamString.
  const char *GetData() const { return m_packet.data(); }

  size_t GetSize() const { return m_packet.size(); }

protected:
  llvm::SmallVector<char, N> m_packet;

  size_t WriteImpl(const void *s, size_t length) override {
    if (s && length)
      m_packet.append((const char *)s, ((const char *)s) + length);
    return length;
  }
};

} // namespace lldb_private

#endif // LLDB_CORE_STREAMBUFFER_H
