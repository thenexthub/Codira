//===-- PipePosix.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_POSIX_PIPEPOSIX_H
#define LLDB_HOST_POSIX_PIPEPOSIX_H

#include "lldb/Host/PipeBase.h"
#include <mutex>

namespace lldb_private {

/// \class PipePosix PipePosix.h "lldb/Host/posix/PipePosix.h"
/// A posix-based implementation of Pipe, a class that abtracts
///        unix style pipes.
///
/// A class that abstracts the LLDB core from host pipe functionality.
class PipePosix : public PipeBase {
public:
  static int kInvalidDescriptor;

  PipePosix();
  PipePosix(lldb::pipe_t read, lldb::pipe_t write);
  PipePosix(const PipePosix &) = delete;
  PipePosix(PipePosix &&pipe_posix);
  PipePosix &operator=(const PipePosix &) = delete;
  PipePosix &operator=(PipePosix &&pipe_posix);

  ~PipePosix() override;

  Status CreateNew() override;
  Status CreateNew(llvm::StringRef name) override;
  Status CreateWithUniqueName(llvm::StringRef prefix,
                              llvm::SmallVectorImpl<char> &name) override;
  Status OpenAsReader(llvm::StringRef name) override;
  llvm::Error OpenAsWriter(llvm::StringRef name,
                           const Timeout<std::micro> &timeout) override;

  bool CanRead() const override;
  bool CanWrite() const override;

  lldb::pipe_t GetReadPipe() const override {
    return lldb::pipe_t(GetReadFileDescriptor());
  }
  lldb::pipe_t GetWritePipe() const override {
    return lldb::pipe_t(GetWriteFileDescriptor());
  }

  int GetReadFileDescriptor() const override;
  int GetWriteFileDescriptor() const override;
  int ReleaseReadFileDescriptor() override;
  int ReleaseWriteFileDescriptor() override;
  void CloseReadFileDescriptor() override;
  void CloseWriteFileDescriptor() override;

  // Close both descriptors
  void Close() override;

  Status Delete(llvm::StringRef name) override;

  llvm::Expected<size_t>
  Write(const void *buf, size_t size,
        const Timeout<std::micro> &timeout = std::nullopt) override;

  llvm::Expected<size_t>
  Read(void *buf, size_t size,
       const Timeout<std::micro> &timeout = std::nullopt) override;

private:
  bool CanReadUnlocked() const;
  bool CanWriteUnlocked() const;

  int GetReadFileDescriptorUnlocked() const;
  int GetWriteFileDescriptorUnlocked() const;
  int ReleaseReadFileDescriptorUnlocked();
  int ReleaseWriteFileDescriptorUnlocked();
  void CloseReadFileDescriptorUnlocked();
  void CloseWriteFileDescriptorUnlocked();
  void CloseUnlocked();

  int m_fds[2];

  /// Mutexes for m_fds;
  mutable std::mutex m_read_mutex;
  mutable std::mutex m_write_mutex;
};

} // namespace lldb_private

#endif // LLDB_HOST_POSIX_PIPEPOSIX_H
