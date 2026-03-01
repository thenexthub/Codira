//===--- DataBufferLLVM.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_DATABUFFERLLVM_H
#define LLDB_UTILITY_DATABUFFERLLVM_H

#include "lldb/Utility/DataBuffer.h"
#include "lldb/lldb-types.h"

#include <cstdint>
#include <memory>

namespace llvm {
class WritableMemoryBuffer;
class MemoryBuffer;
class Twine;
} // namespace llvm

namespace lldb_private {
class FileSystem;

class DataBufferLLVM : public DataBuffer {
public:
  ~DataBufferLLVM() override;

  const uint8_t *GetBytesImpl() const override;
  lldb::offset_t GetByteSize() const override;

  /// LLVM RTTI support.
  /// {
  static char ID;
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || DataBuffer::isA(ClassID);
  }
  static bool classof(const DataBuffer *data_buffer) {
    return data_buffer->isA(&ID);
  }
  /// }

  /// Construct a DataBufferLLVM from \p Buffer.  \p Buffer must be a valid
  /// pointer.
  explicit DataBufferLLVM(std::unique_ptr<llvm::MemoryBuffer> Buffer);

protected:
  std::unique_ptr<llvm::MemoryBuffer> Buffer;
};

class WritableDataBufferLLVM : public WritableDataBuffer {
public:
  ~WritableDataBufferLLVM() override;

  const uint8_t *GetBytesImpl() const override;
  lldb::offset_t GetByteSize() const override;

  /// LLVM RTTI support.
  /// {
  static char ID;
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || WritableDataBuffer::isA(ClassID);
  }
  static bool classof(const DataBuffer *data_buffer) {
    return data_buffer->isA(&ID);
  }
  /// }

  /// Construct a DataBufferLLVM from \p Buffer.  \p Buffer must be a valid
  /// pointer.
  explicit WritableDataBufferLLVM(
      std::unique_ptr<llvm::WritableMemoryBuffer> Buffer);

protected:
  std::unique_ptr<llvm::WritableMemoryBuffer> Buffer;
};
} // namespace lldb_private

#endif
