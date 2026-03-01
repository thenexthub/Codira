//===-- RegisterContextPOSIX_riscv64.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_RISCV64_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_RISCV64_H

#include "RegisterInfoInterface.h"
#include "RegisterInfoPOSIX_riscv64.h"
#include "lldb-riscv-register-enums.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Utility/Log.h"

class RegisterContextPOSIX_riscv64 : public lldb_private::RegisterContext {
public:
  RegisterContextPOSIX_riscv64(
      lldb_private::Thread &thread,
      std::unique_ptr<RegisterInfoPOSIX_riscv64> register_info);

  ~RegisterContextPOSIX_riscv64() override;

  void invalidate();

  void InvalidateAllRegisters() override;

  size_t GetRegisterCount() override;

  virtual size_t GetGPRSize();

  virtual unsigned GetRegisterSize(unsigned reg);

  virtual unsigned GetRegisterOffset(unsigned reg);

  const lldb_private::RegisterInfo *GetRegisterInfoAtIndex(size_t reg) override;

  size_t GetRegisterSetCount() override;

  const lldb_private::RegisterSet *GetRegisterSet(size_t set) override;

protected:
  std::unique_ptr<RegisterInfoPOSIX_riscv64> m_register_info_up;

  virtual const lldb_private::RegisterInfo *GetRegisterInfo();

  bool IsGPR(unsigned reg);

  bool IsFPR(unsigned reg);

  size_t GetFPRSize() { return sizeof(RegisterInfoPOSIX_riscv64::FPR); }

  uint32_t GetRegNumFCSR() const { return fpr_fcsr_riscv; }

  virtual bool ReadGPR() = 0;
  virtual bool ReadFPR() = 0;
  virtual bool WriteGPR() = 0;
  virtual bool WriteFPR() = 0;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_RISCV64_H
