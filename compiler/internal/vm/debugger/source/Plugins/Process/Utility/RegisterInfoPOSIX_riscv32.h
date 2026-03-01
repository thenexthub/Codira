//===-- RegisterInfoPOSIX_riscv32.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOPOSIX_RISCV32_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOPOSIX_RISCV32_H

#include "RegisterInfoAndSetInterface.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Utility/Flags.h"
#include "lldb/lldb-private.h"

#include <map>

class RegisterInfoPOSIX_riscv32
    : public lldb_private::RegisterInfoAndSetInterface {
public:
  static const lldb_private::RegisterInfo *
  GetRegisterInfoPtr(const lldb_private::ArchSpec &target_arch);
  static uint32_t
  GetRegisterInfoCount(const lldb_private::ArchSpec &target_arch);

public:
  // RISC-V32 register set mask value
  enum {
    eRegsetMaskDefault = 0,
    eRegsetMaskFP = 1,
    eRegsetMaskAll = -1,
  };

  struct GPR {
    // gpr[0] is pc, not x0, which is the zero register.
    uint32_t gpr[32];
  };

  struct FPR {
    uint32_t fpr[32];
    uint32_t fcsr;
  };

  RegisterInfoPOSIX_riscv32(const lldb_private::ArchSpec &target_arch,
                            lldb_private::Flags flags);

  size_t GetGPRSize() const override;

  size_t GetFPRSize() const override;

  const lldb_private::RegisterInfo *GetRegisterInfo() const override;

  uint32_t GetRegisterCount() const override;

  const lldb_private::RegisterSet *
  GetRegisterSet(size_t reg_set) const override;

  size_t GetRegisterSetCount() const override;

  size_t GetRegisterSetFromRegisterIndex(uint32_t reg_index) const override;

  bool IsFPPresent() const { return m_opt_regsets.AnySet(eRegsetMaskFP); }

private:
  const lldb_private::RegisterInfo *m_register_info_p;
  uint32_t m_register_info_count;
  lldb_private::Flags m_opt_regsets;
};

#endif
