//===-- NativeProcessAIX.h ---------------------------------- -*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVEPROCESSAIX_H
#define LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVEPROCESSAIX_H

#include "Plugins/Process/Utility/NativeProcessSoftwareSingleStep.h"
#include "lldb/Host/Debug.h"
#include "lldb/Host/common/NativeProcessProtocol.h"
#include "lldb/Host/posix/Support.h"
#include "lldb/Target/MemoryRegionInfo.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/lldb-types.h"
#include "llvm/ADT/SmallPtrSet.h"
#include <csignal>
#include <unordered_set>

namespace lldb_private::process_aix {
/// \class NativeProcessAIX
/// Manages communication with the inferior (debugee) process.
///
/// Upon construction, this class prepares and launches an inferior process
/// for debugging.
///
/// Changes in the inferior process state are broadcasted.
class NativeProcessAIX : public NativeProcessProtocol {
public:
  class Manager : public NativeProcessProtocol::Manager {
  public:
    Manager(MainLoop &mainloop);

    llvm::Expected<std::unique_ptr<NativeProcessProtocol>>
    Launch(ProcessLaunchInfo &launch_info,
           NativeDelegate &native_delegate) override;

    llvm::Expected<std::unique_ptr<NativeProcessProtocol>>
    Attach(lldb::pid_t pid, NativeDelegate &native_delegate) override;

    void AddProcess(NativeProcessAIX &process) { m_processes.insert(&process); }

    void RemoveProcess(NativeProcessAIX &process) {
      m_processes.erase(&process);
    }

    // Collect an event for the given tid, waiting for it if necessary.
    void CollectThread(::pid_t tid);

  private:
    MainLoop::SignalHandleUP m_sigchld_handle;

    llvm::SmallPtrSet<NativeProcessAIX *, 2> m_processes;

    void SigchldHandler();
  };

  // NativeProcessProtocol Interface

  ~NativeProcessAIX() override { m_manager.RemoveProcess(*this); }

  Status Resume(const ResumeActionList &resume_actions) override;

  Status Halt() override;

  Status Detach() override;

  Status Signal(int signo) override;

  Status Interrupt() override;

  Status Kill() override;

  lldb::addr_t GetSharedLibraryInfoAddress() override;

  Status ReadMemory(lldb::addr_t addr, void *buf, size_t size,
                    size_t &bytes_read) override;

  Status WriteMemory(lldb::addr_t addr, const void *buf, size_t size,
                     size_t &bytes_written) override;

  size_t UpdateThreads() override;

  const ArchSpec &GetArchitecture() const override { return m_arch; }

  Status SetBreakpoint(lldb::addr_t addr, uint32_t size,
                       bool hardware) override;

  Status RemoveBreakpoint(lldb::addr_t addr, bool hardware = false) override;

  Status GetLoadedModuleFileSpec(const char *module_path,
                                 FileSpec &file_spec) override;

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  GetAuxvData() const override {
    return getProcFile(GetID(), "auxv");
  }

  Status GetFileLoadAddress(const llvm::StringRef &file_name,
                            lldb::addr_t &load_addr) override;

  static llvm::Expected<int> PtraceWrapper(int req, lldb::pid_t pid,
                                           void *addr = nullptr,
                                           void *data = nullptr,
                                           size_t data_size = 0);

  bool SupportHardwareSingleStepping() const;

private:
  Manager &m_manager;
  ArchSpec m_arch;

  // Private Instance Methods
  NativeProcessAIX(::pid_t pid, int terminal_fd, NativeDelegate &delegate,
                   const ArchSpec &arch, Manager &manager,
                   llvm::ArrayRef<::pid_t> tids);

  bool TryHandleWaitStatus(lldb::pid_t pid, WaitStatus status);

  // Returns a list of process threads that we have attached to.
  static llvm::Expected<std::vector<::pid_t>> Attach(::pid_t pid);

  llvm::Error Detach(lldb::tid_t tid);

  void SigchldHandler();
};

} // namespace lldb_private::process_aix

#endif // #ifndef LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVEPROCESSAIX_H
