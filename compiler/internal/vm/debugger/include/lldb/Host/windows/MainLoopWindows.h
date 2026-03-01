//===-- MainLoopWindows.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_WINDOWS_MAINLOOPWINDOWS_H
#define LLDB_HOST_WINDOWS_MAINLOOPWINDOWS_H

#include "lldb/Host/Config.h"
#include "lldb/Host/MainLoopBase.h"
#include <csignal>
#include <list>
#include <vector>

namespace lldb_private {

using handle_t = void *;

// Windows-specific implementation of the MainLoopBase class. It can monitor
// socket descriptors for readability using WSAEventSelect. Non-socket file
// descriptors are not supported.
class MainLoopWindows : public MainLoopBase {
public:
  MainLoopWindows();
  ~MainLoopWindows() override;

  ReadHandleUP RegisterReadObject(const lldb::IOObjectSP &object_sp,
                                  const Callback &callback,
                                  Status &error) override;

  Status Run() override;

  class IOEvent {
  public:
    IOEvent(handle_t event) : m_event(event) {}
    virtual ~IOEvent() {}
    virtual void WillPoll() {}
    virtual void DidPoll() {}
    virtual void Disarm() {}
    handle_t GetHandle() { return m_event; }

  protected:
    handle_t m_event;
  };
  using IOEventUP = std::unique_ptr<IOEvent>;

protected:
  void UnregisterReadObject(IOObject::WaitableHandle handle) override;

  bool Interrupt() override;

private:
  llvm::Expected<size_t> Poll();

  struct FdInfo {
    IOEventUP event;
    Callback callback;
  };
  llvm::DenseMap<IOObject::WaitableHandle, FdInfo> m_read_fds;
  void *m_interrupt_event;
};

} // namespace lldb_private

#endif // LLDB_HOST_WINDOWS_MAINLOOPWINDOWS_H
