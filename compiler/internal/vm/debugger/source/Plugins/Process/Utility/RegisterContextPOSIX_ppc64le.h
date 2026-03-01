//===-- RegisterContextPOSIX_ppc64le.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_PPC64LE_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_PPC64LE_H

#include "Plugins/Process/Utility/lldb-ppc64le-register-enums.h"
#include "RegisterInfoInterface.h"
#include "Utility/PPC64LE_DWARF_Registers.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Utility/Log.h"

class RegisterContextPOSIX_ppc64le : public lldb_private::RegisterContext {
public:
  RegisterContextPOSIX_ppc64le(
      lldb_private::Thread &thread, uint32_t concrete_frame_idx,
      lldb_private::RegisterInfoInterface *register_info);

  void InvalidateAllRegisters() override;

  size_t GetRegisterCount() override;

  virtual size_t GetGPRSize();

  virtual unsigned GetRegisterSize(unsigned reg);

  virtual unsigned GetRegisterOffset(unsigned reg);

  const lldb_private::RegisterInfo *GetRegisterInfoAtIndex(size_t reg) override;

  size_t GetRegisterSetCount() override;

  const lldb_private::RegisterSet *GetRegisterSet(size_t set) override;

  const char *GetRegisterName(unsigned reg);

protected:
  // 64-bit general purpose registers.
  uint64_t m_gpr_ppc64le[k_num_gpr_registers_ppc64le];

  // floating-point registers including extended register.
  uint64_t m_fpr_ppc64le[k_num_fpr_registers_ppc64le];

  // VMX registers.
  uint64_t m_vmx_ppc64le[k_num_vmx_registers_ppc64le * 2];

  // VSX registers.
  uint64_t m_vsx_ppc64le[k_num_vsx_registers_ppc64le * 2];

  std::unique_ptr<lldb_private::RegisterInfoInterface> m_register_info_up;

  // Determines if an extended register set is supported on the processor
  // running the inferior process.
  virtual bool IsRegisterSetAvailable(size_t set_index);

  virtual const lldb_private::RegisterInfo *GetRegisterInfo();

  bool IsGPR(unsigned reg);

  bool IsFPR(unsigned reg);

  bool IsVMX(unsigned reg);

  bool IsVSX(unsigned reg);

};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_PPC64LE_H
