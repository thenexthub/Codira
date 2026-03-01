//===-- NativeRegisterContextFreeBSD_arm.h ----------------------*- C++ -*-===//
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

#if defined(__arm__)

#ifndef lldb_NativeRegisterContextFreeBSD_arm_h
#define lldb_NativeRegisterContextFreeBSD_arm_h

// clang-format off
#include <sys/types.h>
#include <machine/reg.h>
#include <machine/vfp.h>
// clang-format on

#include "Plugins/Process/FreeBSD/NativeRegisterContextFreeBSD.h"
#include "Plugins/Process/Utility/RegisterInfoPOSIX_arm.h"

#include <array>

namespace lldb_private {
namespace process_freebsd {

class NativeProcessFreeBSD;

class NativeRegisterContextFreeBSD_arm : public NativeRegisterContextFreeBSD {
public:
  NativeRegisterContextFreeBSD_arm(const ArchSpec &target_arch,
                                   NativeThreadFreeBSD &native_thread);

  uint32_t GetRegisterSetCount() const override;

  uint32_t GetUserRegisterCount() const override;

  const RegisterSet *GetRegisterSet(uint32_t set_index) const override;

  Status ReadRegister(const RegisterInfo *reg_info,
                      RegisterValue &reg_value) override;

  Status WriteRegister(const RegisterInfo *reg_info,
                       const RegisterValue &reg_value) override;

  Status ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  Status WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  llvm::Error
  CopyHardwareWatchpointsFrom(NativeRegisterContextFreeBSD &source) override;

private:
  std::array<uint8_t, sizeof(reg) + sizeof(vfp_state)> m_reg_data;

  Status ReadRegisterSet(uint32_t set);
  Status WriteRegisterSet(uint32_t set);

  RegisterInfoPOSIX_arm &GetRegisterInfo() const;
};

} // namespace process_freebsd
} // namespace lldb_private

#endif // #ifndef lldb_NativeRegisterContextFreeBSD_arm_h

#endif // defined (__arm__)
