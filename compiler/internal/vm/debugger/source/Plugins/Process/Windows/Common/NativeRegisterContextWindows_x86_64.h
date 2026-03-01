//===-- NativeRegisterContextWindows_x86_64.h -------------------*- C++ -*-===//
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

#if defined(__x86_64__) || defined(_M_X64)
#ifndef liblldb_NativeRegisterContextWindows_x86_64_h_
#define liblldb_NativeRegisterContextWindows_x86_64_h_

#include "Plugins/Process/Utility/RegisterContext_x86.h"
#include "Plugins/Process/Utility/lldb-x86-register-enums.h"

#include "NativeRegisterContextWindows.h"

namespace lldb_private {

class NativeThreadWindows;

class NativeRegisterContextWindows_x86_64
    : public NativeRegisterContextWindows {
public:
  NativeRegisterContextWindows_x86_64(const ArchSpec &target_arch,
                                      NativeThreadProtocol &native_thread);

  uint32_t GetRegisterSetCount() const override;

  const RegisterSet *GetRegisterSet(uint32_t set_index) const override;

  Status ReadRegister(const RegisterInfo *reg_info,
                      RegisterValue &reg_value) override;

  Status WriteRegister(const RegisterInfo *reg_info,
                       const RegisterValue &reg_value) override;

  Status ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  Status WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  Status IsWatchpointHit(uint32_t wp_index, bool &is_hit) override;

  Status GetWatchpointHitIndex(uint32_t &wp_index,
                               lldb::addr_t trap_addr) override;

  Status IsWatchpointVacant(uint32_t wp_index, bool &is_vacant) override;

  bool ClearHardwareWatchpoint(uint32_t wp_index) override;

  Status ClearAllHardwareWatchpoints() override;

  uint32_t SetHardwareWatchpoint(lldb::addr_t addr, size_t size,
                                 uint32_t watch_flags) override;

  lldb::addr_t GetWatchpointAddress(uint32_t wp_index) override;

  uint32_t NumSupportedHardwareWatchpoints() override;

protected:
  Status GPRRead(const uint32_t reg, RegisterValue &reg_value);
  Status GPRWrite(const uint32_t reg, const RegisterValue &reg_value);

  Status FPRRead(const uint32_t reg, RegisterValue &reg_value);
  Status FPRWrite(const uint32_t reg, const RegisterValue &reg_value);

  Status DRRead(const uint32_t reg, RegisterValue &reg_value);
  Status DRWrite(const uint32_t reg, const RegisterValue &reg_value);

private:
  Status ApplyHardwareBreakpoint(uint32_t wp_index, lldb::addr_t addr,
                                 size_t size, uint32_t flags);

  bool IsGPR(uint32_t reg_index) const;
  bool IsFPR(uint32_t reg_index) const;
  bool IsDR(uint32_t reg_index) const;
};

} // namespace lldb_private

#endif // liblldb_NativeRegisterContextWindows_x86_64_h_
#endif // defined(__x86_64__) || defined(_M_X64)
