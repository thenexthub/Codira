//===-- ConnectionGenericFileWindows.h --------------------------*- C++ -*-===//
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

#ifndef liblldb_Host_windows_ConnectionGenericFileWindows_h_
#define liblldb_Host_windows_ConnectionGenericFileWindows_h_

#include "lldb/Host/windows/windows.h"
#include "lldb/Utility/Connection.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

class Status;

class ConnectionGenericFile : public lldb_private::Connection {
public:
  ConnectionGenericFile();

  ConnectionGenericFile(lldb::file_t file, bool owns_file);

  ~ConnectionGenericFile() override;

  bool IsConnected() const override;

  lldb::ConnectionStatus Connect(llvm::StringRef s, Status *error_ptr) override;

  lldb::ConnectionStatus Disconnect(Status *error_ptr) override;

  size_t Read(void *dst, size_t dst_len, const Timeout<std::micro> &timeout,
              lldb::ConnectionStatus &status, Status *error_ptr) override;

  size_t Write(const void *src, size_t src_len, lldb::ConnectionStatus &status,
               Status *error_ptr) override;

  std::string GetURI() override;

  bool InterruptRead() override;

protected:
  OVERLAPPED m_overlapped;
  HANDLE m_file;
  HANDLE m_event_handles[2];
  bool m_owns_file;
  LARGE_INTEGER m_file_position;

  enum { kBytesAvailableEvent, kInterruptEvent };

private:
  void InitializeEventHandles();
  void IncrementFilePointer(DWORD amount);

  std::string m_uri;

  ConnectionGenericFile(const ConnectionGenericFile &) = delete;
  const ConnectionGenericFile &
  operator=(const ConnectionGenericFile &) = delete;
};
}

#endif
