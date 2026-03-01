//===-- NativeRegisterContextNetBSD_x86_64.h --------------------*- C++ -*-===//
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

#if defined(__i386__) || defined(__x86_64__)

#ifndef lldb_NativeRegisterContextNetBSD_x86_64_h
#define lldb_NativeRegisterContextNetBSD_x86_64_h

// clang-format off
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <machine/reg.h>
// clang-format on

#include <array>
#include <optional>

#include "Plugins/Process/NetBSD/NativeRegisterContextNetBSD.h"
#include "Plugins/Process/Utility/RegisterContext_x86.h"
#include "Plugins/Process/Utility/NativeRegisterContextDBReg_x86.h"
#include "Plugins/Process/Utility/lldb-x86-register-enums.h"

namespace lldb_private {
namespace process_netbsd {

class NativeProcessNetBSD;

class NativeRegisterContextNetBSD_x86_64
    : public NativeRegisterContextNetBSD,
      public NativeRegisterContextDBReg_x86 {
public:
  NativeRegisterContextNetBSD_x86_64(const ArchSpec &target_arch,
                                     NativeThreadProtocol &native_thread);
  uint32_t GetRegisterSetCount() const override;

  const RegisterSet *GetRegisterSet(uint32_t set_index) const override;

  Status ReadRegister(const RegisterInfo *reg_info,
                      RegisterValue &reg_value) override;

  Status WriteRegister(const RegisterInfo *reg_info,
                       const RegisterValue &reg_value) override;

  Status ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  Status WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  llvm::Error
  CopyHardwareWatchpointsFrom(NativeRegisterContextNetBSD &source) override;

private:
  // Private member types.
  enum RegSetKind {
    GPRegSet,
    FPRegSet,
    DBRegSet,
    MaxRegularRegSet = DBRegSet,
    YMMRegSet,
    MPXRegSet,
    MaxRegSet = MPXRegSet,
  };

  // Private member variables.
  std::array<uint8_t, sizeof(struct reg)> m_gpr;
  std::array<uint8_t, sizeof(struct xstate)> m_xstate;
  std::array<uint8_t, sizeof(struct dbreg)> m_dbr;
  std::array<size_t, MaxRegularRegSet + 1> m_regset_offsets;

  std::optional<RegSetKind> GetSetForNativeRegNum(uint32_t reg_num) const;

  Status ReadRegisterSet(RegSetKind set);
  Status WriteRegisterSet(RegSetKind set);

  uint8_t *GetOffsetRegSetData(RegSetKind set, size_t reg_offset);

  struct YMMSplitPtr {
    void *xmm;
    void *ymm_hi;
  };
  std::optional<YMMSplitPtr> GetYMMSplitReg(uint32_t reg);
};

} // namespace process_netbsd
} // namespace lldb_private

#endif // #ifndef lldb_NativeRegisterContextNetBSD_x86_64_h

#endif // defined(__x86_64__)
