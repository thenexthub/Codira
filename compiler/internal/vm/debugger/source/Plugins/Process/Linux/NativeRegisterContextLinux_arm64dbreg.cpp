//===-- NativeRegisterContextLinux_arm64dbreg.cpp -------------------------===//
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

#if defined(__arm64__) || defined(__aarch64__)

#include "NativeRegisterContextLinux_arm64dbreg.h"
#include "lldb/Host/linux/Ptrace.h"

#include <asm/ptrace.h>
// System includes - They have to be included after framework includes because
// they define some macros which collide with variable names in other modules
#include <sys/uio.h>
// NT_PRSTATUS and NT_FPREGSET definition
#include <elf.h>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_linux;

static Status ReadHardwareDebugInfoHelper(int regset, ::pid_t tid,
                                          uint32_t &max_supported) {
  struct iovec ioVec;
  struct user_hwdebug_state dreg_state;
  Status error;

  ioVec.iov_base = &dreg_state;
  ioVec.iov_len = sizeof(dreg_state);
  error = NativeProcessLinux::PtraceWrapper(PTRACE_GETREGSET, tid, &regset,
                                            &ioVec, ioVec.iov_len);

  if (error.Fail())
    return error;

  max_supported = dreg_state.dbg_info & 0xff;
  return error;
}

Status lldb_private::process_linux::arm64::ReadHardwareDebugInfo(
    ::pid_t tid, uint32_t &max_hwp_supported, uint32_t &max_hbp_supported) {
  Status error =
      ReadHardwareDebugInfoHelper(NT_ARM_HW_WATCH, tid, max_hwp_supported);

  if (error.Fail())
    return error;

  return ReadHardwareDebugInfoHelper(NT_ARM_HW_BREAK, tid, max_hbp_supported);
}

Status lldb_private::process_linux::arm64::WriteHardwareDebugRegs(
    int hwbType, ::pid_t tid, uint32_t max_supported,
    const std::array<NativeRegisterContextDBReg::DREG, 16> &regs) {
  int regset = hwbType == NativeRegisterContextDBReg::eDREGTypeWATCH
                   ? NT_ARM_HW_WATCH
                   : NT_ARM_HW_BREAK;

  struct user_hwdebug_state dreg_state;
  memset(&dreg_state, 0, sizeof(dreg_state));
  for (uint32_t i = 0; i < max_supported; i++) {
    dreg_state.dbg_regs[i].addr = regs[i].address;
    dreg_state.dbg_regs[i].ctrl = regs[i].control;
  }

  struct iovec ioVec;
  ioVec.iov_base = &dreg_state;
  ioVec.iov_len = sizeof(dreg_state.dbg_info) + sizeof(dreg_state.pad) +
                  (sizeof(dreg_state.dbg_regs[0]) * max_supported);

  return NativeProcessLinux::PtraceWrapper(PTRACE_SETREGSET, tid, &regset,
                                           &ioVec, ioVec.iov_len);
}

#endif // defined (__arm64__) || defined (__aarch64__)
