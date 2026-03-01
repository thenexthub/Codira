//===-- RegisterInfoPOSIX_loongarch64.h -------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOPOSIX_LOONGARCH64_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOPOSIX_LOONGARCH64_H

#include "RegisterInfoAndSetInterface.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/lldb-private.h"
#include <map>

class RegisterInfoPOSIX_loongarch64
    : public lldb_private::RegisterInfoAndSetInterface {
public:
  static const lldb_private::RegisterInfo *
  GetRegisterInfoPtr(const lldb_private::ArchSpec &target_arch);
  static uint32_t
  GetRegisterInfoCount(const lldb_private::ArchSpec &target_arch);

public:
  enum RegSetKind {
    GPRegSet,
    FPRegSet,
    LSXRegSet,
    LASXRegSet,
  };

  struct GPR {
    uint64_t gpr[32];

    uint64_t orig_a0;
    uint64_t csr_era;
    uint64_t csr_badv;
    uint64_t reserved[10];
  };

  struct FPR {
    uint64_t fpr[32];
    uint64_t fcc;
    uint32_t fcsr;
  };

  /* 32 registers, 128 bits width per register. */
  struct LSX {
    uint64_t vr[32 * 2];
  };

  /* 32 registers, 256 bits width per register. */
  struct LASX {
    uint64_t xr[32 * 4];
  };

  RegisterInfoPOSIX_loongarch64(const lldb_private::ArchSpec &target_arch,
                                lldb_private::Flags flags);

  size_t GetGPRSize() const override;

  size_t GetFPRSize() const override;

  const lldb_private::RegisterInfo *GetRegisterInfo() const override;

  uint32_t GetRegisterCount() const override;

  const lldb_private::RegisterSet *
  GetRegisterSet(size_t reg_set) const override;

  size_t GetRegisterSetCount() const override;

  size_t GetRegisterSetFromRegisterIndex(uint32_t reg_index) const override;

private:
  const lldb_private::RegisterInfo *m_register_info_p;
  uint32_t m_register_info_count;
};

#endif
