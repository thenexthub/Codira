//===-- RegisterContextFreeBSDKernel_x86_64.cpp ---------------------------===//
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

#include "RegisterContextFreeBSDKernel_x86_64.h"

#include "lldb/Target/Process.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/RegisterValue.h"
#include "llvm/Support/Endian.h"

using namespace lldb;
using namespace lldb_private;

RegisterContextFreeBSDKernel_x86_64::RegisterContextFreeBSDKernel_x86_64(
    Thread &thread, RegisterInfoInterface *register_info, lldb::addr_t pcb_addr)
    : RegisterContextPOSIX_x86(thread, 0, register_info), m_pcb_addr(pcb_addr) {
}

bool RegisterContextFreeBSDKernel_x86_64::ReadGPR() { return true; }

bool RegisterContextFreeBSDKernel_x86_64::ReadFPR() { return true; }

bool RegisterContextFreeBSDKernel_x86_64::WriteGPR() {
  assert(0);
  return false;
}

bool RegisterContextFreeBSDKernel_x86_64::WriteFPR() {
  assert(0);
  return false;
}

bool RegisterContextFreeBSDKernel_x86_64::ReadRegister(
    const RegisterInfo *reg_info, RegisterValue &value) {
  if (m_pcb_addr == LLDB_INVALID_ADDRESS)
    return false;

  struct {
    llvm::support::ulittle64_t r15;
    llvm::support::ulittle64_t r14;
    llvm::support::ulittle64_t r13;
    llvm::support::ulittle64_t r12;
    llvm::support::ulittle64_t rbp;
    llvm::support::ulittle64_t rsp;
    llvm::support::ulittle64_t rbx;
    llvm::support::ulittle64_t rip;
  } pcb;

  Status error;
  size_t rd =
      m_thread.GetProcess()->ReadMemory(m_pcb_addr, &pcb, sizeof(pcb), error);
  if (rd != sizeof(pcb))
    return false;

  uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
  switch (reg) {
#define REG(x)                                                                 \
  case lldb_##x##_x86_64:                                                      \
    value = pcb.x;                                                             \
    break;

    REG(r15);
    REG(r14);
    REG(r13);
    REG(r12);
    REG(rbp);
    REG(rsp);
    REG(rbx);
    REG(rip);

#undef REG

  default:
    return false;
  }

  return true;
}

bool RegisterContextFreeBSDKernel_x86_64::WriteRegister(
    const RegisterInfo *reg_info, const RegisterValue &value) {
  return false;
}
