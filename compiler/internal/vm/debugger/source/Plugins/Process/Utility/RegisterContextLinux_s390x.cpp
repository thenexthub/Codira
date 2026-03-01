//===-- RegisterContextLinux_s390x.cpp ------------------------------------===//
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

#include "RegisterContextLinux_s390x.h"
#include "RegisterContextPOSIX_s390x.h"

using namespace lldb_private;
using namespace lldb;

// Include RegisterInfos_s390x to declare our g_register_infos_s390x structure.
#define DECLARE_REGISTER_INFOS_S390X_STRUCT
#include "RegisterInfos_s390x.h"
#undef DECLARE_REGISTER_INFOS_S390X_STRUCT

static const RegisterInfo *GetRegisterInfoPtr(const ArchSpec &target_arch) {
  switch (target_arch.GetMachine()) {
  case llvm::Triple::systemz:
    return g_register_infos_s390x;
  default:
    assert(false && "Unhandled target architecture.");
    return nullptr;
  }
}

static uint32_t GetRegisterInfoCount(const ArchSpec &target_arch) {
  switch (target_arch.GetMachine()) {
  case llvm::Triple::systemz:
    return k_num_registers_s390x;
  default:
    assert(false && "Unhandled target architecture.");
    return 0;
  }
}

static uint32_t GetUserRegisterInfoCount(const ArchSpec &target_arch) {
  switch (target_arch.GetMachine()) {
  case llvm::Triple::systemz:
    return k_num_user_registers_s390x + k_num_linux_registers_s390x;
  default:
    assert(false && "Unhandled target architecture.");
    return 0;
  }
}

RegisterContextLinux_s390x::RegisterContextLinux_s390x(
    const ArchSpec &target_arch)
    : lldb_private::RegisterInfoInterface(target_arch),
      m_register_info_p(GetRegisterInfoPtr(target_arch)),
      m_register_info_count(GetRegisterInfoCount(target_arch)),
      m_user_register_count(GetUserRegisterInfoCount(target_arch)) {}

const RegisterInfo *RegisterContextLinux_s390x::GetRegisterInfo() const {
  return m_register_info_p;
}

uint32_t RegisterContextLinux_s390x::GetRegisterCount() const {
  return m_register_info_count;
}

uint32_t RegisterContextLinux_s390x::GetUserRegisterCount() const {
  return m_user_register_count;
}

size_t RegisterContextLinux_s390x::GetGPRSize() const { return 0; }
