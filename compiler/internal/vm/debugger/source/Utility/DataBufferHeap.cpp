//===-- DataBufferHeap.cpp ------------------------------------------------===//
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

#include "lldb/Utility/DataBufferHeap.h"


using namespace lldb_private;

// Default constructor
DataBufferHeap::DataBufferHeap() : m_data() {}

// Initialize this class with "n" characters and fill the buffer with "ch".
DataBufferHeap::DataBufferHeap(lldb::offset_t n, uint8_t ch) : m_data() {
  if (n < m_data.max_size())
    m_data.assign(n, ch);
}

// Initialize this class with a copy of the "n" bytes from the "bytes" buffer.
DataBufferHeap::DataBufferHeap(const void *src, lldb::offset_t src_len)
    : m_data() {
  CopyData(src, src_len);
}

DataBufferHeap::DataBufferHeap(const DataBuffer &data_buffer) : m_data() {
  CopyData(data_buffer.GetBytes(), data_buffer.GetByteSize());
}

// Virtual destructor since this class inherits from a pure virtual base class.
DataBufferHeap::~DataBufferHeap() = default;

// Return a const pointer to the bytes owned by this object, or nullptr if the
// object contains no bytes.
const uint8_t *DataBufferHeap::GetBytesImpl() const {
  return (m_data.empty() ? nullptr : m_data.data());
}

// Return the number of bytes this object currently contains.
uint64_t DataBufferHeap::GetByteSize() const { return m_data.size(); }

// Sets the number of bytes that this object should be able to contain. This
// can be used prior to copying data into the buffer.
uint64_t DataBufferHeap::SetByteSize(uint64_t new_size) {
  if (new_size < m_data.max_size())
    m_data.resize(new_size);
  return m_data.size();
}

void DataBufferHeap::CopyData(const void *src, uint64_t src_len) {
  const uint8_t *src_u8 = static_cast<const uint8_t *>(src);
  if (src && src_len > 0)
    m_data.assign(src_u8, src_u8 + src_len);
  else
    m_data.clear();
}

void DataBufferHeap::AppendData(const void *src, uint64_t src_len) {
  m_data.insert(m_data.end(), static_cast<const uint8_t *>(src),
                static_cast<const uint8_t *>(src) + src_len);
}

void DataBufferHeap::Clear() {
  buffer_t empty;
  m_data.swap(empty);
}

char DataBuffer::ID;
char WritableDataBuffer::ID;
char DataBufferUnowned::ID;
char DataBufferHeap::ID;
