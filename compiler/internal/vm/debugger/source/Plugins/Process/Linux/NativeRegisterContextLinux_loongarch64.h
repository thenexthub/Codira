//===-- NativeRegisterContextLinux_loongarch64.h ----------------*- C++ -*-===//
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

#if defined(__loongarch__) && __loongarch_grlen == 64

#ifndef lldb_NativeRegisterContextLinux_loongarch64_h
#define lldb_NativeRegisterContextLinux_loongarch64_h

#include "Plugins/Process/Linux/NativeRegisterContextLinux.h"
#include "Plugins/Process/Utility/NativeRegisterContextDBReg_loongarch.h"
#include "Plugins/Process/Utility/RegisterInfoPOSIX_loongarch64.h"

#include <asm/ptrace.h>

namespace lldb_private {
namespace process_linux {

class NativeProcessLinux;

class NativeRegisterContextLinux_loongarch64
    : public NativeRegisterContextLinux,
      public NativeRegisterContextDBReg_loongarch {
public:
  NativeRegisterContextLinux_loongarch64(
      const ArchSpec &target_arch, NativeThreadProtocol &native_thread,
      std::unique_ptr<RegisterInfoPOSIX_loongarch64> register_info_up);

  uint32_t GetRegisterSetCount() const override;

  uint32_t GetUserRegisterCount() const override;

  const RegisterSet *GetRegisterSet(uint32_t set_index) const override;

  Status ReadRegister(const RegisterInfo *reg_info,
                      RegisterValue &reg_value) override;

  Status WriteRegister(const RegisterInfo *reg_info,
                       const RegisterValue &reg_value) override;

  Status ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  Status WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  void InvalidateAllRegisters() override;

  std::vector<uint32_t>
  GetExpeditedRegisters(ExpeditedRegs expType) const override;

  bool RegisterOffsetIsDynamic() const override { return true; }

protected:
  Status ReadGPR() override;

  Status WriteGPR() override;

  Status ReadFPR() override;

  Status WriteFPR() override;

  Status ReadLSX();

  Status WriteLSX();

  Status ReadLASX();

  Status WriteLASX();

  void *GetGPRBuffer() override { return &m_gpr; }

  void *GetFPRBuffer() override { return &m_fpr; }

  size_t GetGPRSize() const override { return GetRegisterInfo().GetGPRSize(); }

  size_t GetFPRSize() override { return GetRegisterInfo().GetFPRSize(); }

private:
  bool m_gpr_is_valid;
  bool m_fpu_is_valid;
  bool m_lsx_is_valid;
  bool m_lasx_is_valid;
  bool m_refresh_hwdebug_info;

  RegisterInfoPOSIX_loongarch64::GPR m_gpr;
  RegisterInfoPOSIX_loongarch64::FPR m_fpr;
  RegisterInfoPOSIX_loongarch64::LSX m_lsx;
  RegisterInfoPOSIX_loongarch64::LASX m_lasx;

  bool IsGPR(unsigned reg) const;

  bool IsFPR(unsigned reg) const;

  bool IsLSX(unsigned reg) const;

  bool IsLASX(unsigned reg) const;

  uint32_t CalculateFprOffset(const RegisterInfo *reg_info) const;

  uint32_t CalculateLsxOffset(const RegisterInfo *reg_info) const;

  uint32_t CalculateLasxOffset(const RegisterInfo *reg_info) const;

  const RegisterInfoPOSIX_loongarch64 &GetRegisterInfo() const;

  llvm::Error ReadHardwareDebugInfo() override;

  llvm::Error WriteHardwareDebugRegs(DREGType hwbType) override;
};

} // namespace process_linux
} // namespace lldb_private

#endif // #ifndef lldb_NativeRegisterContextLinux_loongarch64_h

#endif // defined(__loongarch__) && __loongarch_grlen == 64
