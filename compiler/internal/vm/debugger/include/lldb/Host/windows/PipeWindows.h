//===-- PipeWindows.h -------------------------------------------*- C++ -*-===//
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

#ifndef liblldb_Host_windows_PipeWindows_h_
#define liblldb_Host_windows_PipeWindows_h_

#include "lldb/Host/PipeBase.h"
#include "lldb/Host/windows/windows.h"

namespace lldb_private {

/// \class Pipe PipeWindows.h "lldb/Host/windows/PipeWindows.h"
/// A windows-based implementation of Pipe, a class that abtracts
///        unix style pipes.
///
/// A class that abstracts the LLDB core from host pipe functionality.
class PipeWindows : public PipeBase {
public:
  static const int kInvalidDescriptor = -1;

public:
  PipeWindows();
  PipeWindows(lldb::pipe_t read, lldb::pipe_t write);
  ~PipeWindows() override;

  // Create an unnamed pipe.
  Status CreateNew() override;

  // Create a named pipe.
  Status CreateNew(llvm::StringRef name) override;
  Status CreateWithUniqueName(llvm::StringRef prefix,
                              llvm::SmallVectorImpl<char> &name) override;
  Status OpenAsReader(llvm::StringRef name) override;
  llvm::Error OpenAsWriter(llvm::StringRef name,
                           const Timeout<std::micro> &timeout) override;

  bool CanRead() const override;
  bool CanWrite() const override;

  lldb::pipe_t GetReadPipe() const override { return lldb::pipe_t(m_read); }
  lldb::pipe_t GetWritePipe() const override { return lldb::pipe_t(m_write); }

  int GetReadFileDescriptor() const override;
  int GetWriteFileDescriptor() const override;
  int ReleaseReadFileDescriptor() override;
  int ReleaseWriteFileDescriptor() override;
  void CloseReadFileDescriptor() override;
  void CloseWriteFileDescriptor() override;

  void Close() override;

  Status Delete(llvm::StringRef name) override;

  llvm::Expected<size_t>
  Write(const void *buf, size_t size,
        const Timeout<std::micro> &timeout = std::nullopt) override;

  llvm::Expected<size_t>
  Read(void *buf, size_t size,
       const Timeout<std::micro> &timeout = std::nullopt) override;

  // PipeWindows specific methods.  These allow access to the underlying OS
  // handle.
  HANDLE GetReadNativeHandle();
  HANDLE GetWriteNativeHandle();

private:
  Status OpenNamedPipe(llvm::StringRef name, bool is_read);

  HANDLE m_read;
  HANDLE m_write;

  int m_read_fd;
  int m_write_fd;

  OVERLAPPED m_read_overlapped;
  OVERLAPPED m_write_overlapped;
};

} // namespace lldb_private

#endif // liblldb_Host_posix_PipePosix_h_
