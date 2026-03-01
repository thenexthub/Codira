//===-- RegisterInfoPOSIX_ppc64le.cpp -------------------------------------===//
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
//===---------------------------------------------------------------------===//

#include <cassert>
#include <cstddef>
#include <vector>

#include "lldb/lldb-defines.h"
#include "llvm/Support/Compiler.h"

#include "RegisterInfoPOSIX_ppc64le.h"

// Include RegisterInfoPOSIX_ppc64le to declare our g_register_infos_ppc64le
#define DECLARE_REGISTER_INFOS_PPC64LE_STRUCT
#include "RegisterInfos_ppc64le.h"
#undef DECLARE_REGISTER_INFOS_PPC64LE_STRUCT

static const lldb_private::RegisterInfo *
GetRegisterInfoPtr(const lldb_private::ArchSpec &target_arch) {
  switch (target_arch.GetMachine()) {
  case llvm::Triple::ppc64le:
    return g_register_infos_ppc64le;
  default:
    assert(false && "Unhandled target architecture.");
    return nullptr;
  }
}

static uint32_t
GetRegisterInfoCount(const lldb_private::ArchSpec &target_arch) {
  switch (target_arch.GetMachine()) {
  case llvm::Triple::ppc64le:
    return static_cast<uint32_t>(sizeof(g_register_infos_ppc64le) /
                                 sizeof(g_register_infos_ppc64le[0]));
  default:
    assert(false && "Unhandled target architecture.");
    return 0;
  }
}

RegisterInfoPOSIX_ppc64le::RegisterInfoPOSIX_ppc64le(
    const lldb_private::ArchSpec &target_arch)
    : lldb_private::RegisterInfoInterface(target_arch),
      m_register_info_p(GetRegisterInfoPtr(target_arch)),
      m_register_info_count(GetRegisterInfoCount(target_arch)) {}

size_t RegisterInfoPOSIX_ppc64le::GetGPRSize() const {
  return sizeof(GPR);
}

const lldb_private::RegisterInfo *
RegisterInfoPOSIX_ppc64le::GetRegisterInfo() const {
  return m_register_info_p;
}

uint32_t RegisterInfoPOSIX_ppc64le::GetRegisterCount() const {
  return m_register_info_count;
}
