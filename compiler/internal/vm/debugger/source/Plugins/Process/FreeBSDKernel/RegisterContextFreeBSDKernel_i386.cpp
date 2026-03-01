//===-- RegisterContextFreeBSDKernel_i386.cpp -----------------------------===//
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

#include "RegisterContextFreeBSDKernel_i386.h"

#include "lldb/Target/Process.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/RegisterValue.h"
#include "llvm/Support/Endian.h"

using namespace lldb;
using namespace lldb_private;

RegisterContextFreeBSDKernel_i386::RegisterContextFreeBSDKernel_i386(
    Thread &thread, RegisterInfoInterface *register_info, lldb::addr_t pcb_addr)
    : RegisterContextPOSIX_x86(thread, 0, register_info), m_pcb_addr(pcb_addr) {
}

bool RegisterContextFreeBSDKernel_i386::ReadGPR() { return true; }

bool RegisterContextFreeBSDKernel_i386::ReadFPR() { return true; }

bool RegisterContextFreeBSDKernel_i386::WriteGPR() {
  assert(0);
  return false;
}

bool RegisterContextFreeBSDKernel_i386::WriteFPR() {
  assert(0);
  return false;
}

bool RegisterContextFreeBSDKernel_i386::ReadRegister(
    const RegisterInfo *reg_info, RegisterValue &value) {
  if (m_pcb_addr == LLDB_INVALID_ADDRESS)
    return false;

  struct {
    llvm::support::ulittle32_t edi;
    llvm::support::ulittle32_t esi;
    llvm::support::ulittle32_t ebp;
    llvm::support::ulittle32_t esp;
    llvm::support::ulittle32_t ebx;
    llvm::support::ulittle32_t eip;
  } pcb;

  Status error;
  size_t rd =
      m_thread.GetProcess()->ReadMemory(m_pcb_addr, &pcb, sizeof(pcb), error);
  if (rd != sizeof(pcb))
    return false;

  uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
  switch (reg) {
#define REG(x)                                                                 \
  case lldb_##x##_i386:                                                      \
    value = pcb.x;                                                             \
    break;

    REG(edi);
    REG(esi);
    REG(ebp);
    REG(esp);
    REG(eip);

#undef REG

  default:
    return false;
  }

  return true;
}

bool RegisterContextFreeBSDKernel_i386::WriteRegister(
    const RegisterInfo *reg_info, const RegisterValue &value) {
  return false;
}
