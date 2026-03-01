//===-- ThreadElfCore.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_THREADELFCORE_H
#define LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_THREADELFCORE_H

#include "Plugins/Process/elf-core/RegisterUtilities.h"
#include "lldb/Target/Platform.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/ValueObject/ValueObject.h"
#include "llvm/ADT/DenseMap.h"
#include <optional>
#include <string>

struct compat_timeval {
  alignas(8) uint64_t tv_sec;
  alignas(8) uint64_t tv_usec;
};

namespace lldb_private {
class ProcessInstanceInfo;
}

// PRSTATUS structure's size differs based on architecture.
// This is the layout in the x86-64 arch.
// In the i386 case we parse it manually and fill it again
// in the same structure
// The gp registers are also a part of this struct, but they are handled
// separately

#undef si_signo
#undef si_code
#undef si_errno
#undef si_addr
#undef si_addr_lsb

struct ELFLinuxPrStatus {
  int32_t si_signo;
  int32_t si_code;
  int32_t si_errno;

  int16_t pr_cursig;

  alignas(8) uint64_t pr_sigpend;
  alignas(8) uint64_t pr_sighold;

  uint32_t pr_pid;
  uint32_t pr_ppid;
  uint32_t pr_pgrp;
  uint32_t pr_sid;

  compat_timeval pr_utime;
  compat_timeval pr_stime;
  compat_timeval pr_cutime;
  compat_timeval pr_cstime;

  ELFLinuxPrStatus();

  lldb_private::Status Parse(const lldb_private::DataExtractor &data,
                             const lldb_private::ArchSpec &arch);

  static std::optional<ELFLinuxPrStatus>
  Populate(const lldb::ThreadSP &thread_sp);

  // Return the bytesize of the structure
  // 64 bit - just sizeof
  // 32 bit - hardcoded because we are reusing the struct, but some of the
  // members are smaller -
  // so the layout is not the same
  static size_t GetSize(const lldb_private::ArchSpec &arch);
};

static_assert(sizeof(ELFLinuxPrStatus) == 112,
              "sizeof ELFLinuxPrStatus is not correct!");

struct ThreadData {
  lldb_private::DataExtractor gpregset;
  std::vector<lldb_private::CoreNote> notes;
  lldb::tid_t tid;
  std::string name;
  llvm::StringRef siginfo_bytes;
  int prstatus_sig = 0;
  int signo = 0;
};

// PRPSINFO structure's size differs based on architecture.
// This is the layout in the x86-64 arch case.
// In the i386 case we parse it manually and fill it again
// in the same structure
struct ELFLinuxPrPsInfo {
  char pr_state;
  char pr_sname;
  char pr_zomb;
  char pr_nice;
  alignas(8) uint64_t pr_flag;
  uint32_t pr_uid;
  uint32_t pr_gid;
  int32_t pr_pid;
  int32_t pr_ppid;
  int32_t pr_pgrp;
  int32_t pr_sid;
  char pr_fname[16];
  char pr_psargs[80];

  ELFLinuxPrPsInfo();

  lldb_private::Status Parse(const lldb_private::DataExtractor &data,
                             const lldb_private::ArchSpec &arch);

  static std::optional<ELFLinuxPrPsInfo>
  Populate(const lldb::ProcessSP &process_sp);

  static std::optional<ELFLinuxPrPsInfo>
  Populate(const lldb_private::ProcessInstanceInfo &info,
           lldb::StateType state);

  // Return the bytesize of the structure
  // 64 bit - just sizeof
  // 32 bit - hardcoded because we are reusing the struct, but some of the
  // members are smaller -
  // so the layout is not the same
  static size_t GetSize(const lldb_private::ArchSpec &arch);
};

static_assert(sizeof(ELFLinuxPrPsInfo) == 136,
              "sizeof ELFLinuxPrPsInfo is not correct!");

class ThreadElfCore : public lldb_private::Thread {
public:
  ThreadElfCore(lldb_private::Process &process, const ThreadData &td);

  ~ThreadElfCore() override;

  void RefreshStateAfterStop() override;

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(lldb_private::StackFrame *frame) override;

  static bool ThreadIDIsValid(lldb::tid_t thread) { return thread != 0; }

  const char *GetName() override {
    if (m_thread_name.empty())
      return nullptr;
    return m_thread_name.c_str();
  }

  void SetName(const char *name) override {
    if (name && name[0])
      m_thread_name.assign(name);
    else
      m_thread_name.clear();
  }

  llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>>
  GetSiginfo(size_t max_size) const override;

protected:
  // Member variables.
  std::string m_thread_name;
  lldb::RegisterContextSP m_thread_reg_ctx_sp;

  lldb_private::DataExtractor m_gpregset_data;
  std::vector<lldb_private::CoreNote> m_notes;
  llvm::StringRef m_siginfo_bytes;
  // Only used if no siginfo note.
  int m_signo;

  bool CalculateStopInfo() override;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_THREADELFCORE_H
