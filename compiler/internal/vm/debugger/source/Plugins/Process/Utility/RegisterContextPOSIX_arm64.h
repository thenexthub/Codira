//===-- RegisterContextPOSIX_arm64.h ----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_ARM64_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_ARM64_H

#include "RegisterInfoInterface.h"
#include "RegisterInfoPOSIX_arm64.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Utility/Log.h"

class RegisterContextPOSIX_arm64 : public lldb_private::RegisterContext {
public:
  RegisterContextPOSIX_arm64(
      lldb_private::Thread &thread,
      std::unique_ptr<RegisterInfoPOSIX_arm64> register_info);

  ~RegisterContextPOSIX_arm64() override;

  void Invalidate();

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
  std::unique_ptr<RegisterInfoPOSIX_arm64> m_register_info_up;

  virtual const lldb_private::RegisterInfo *GetRegisterInfo();

  bool IsGPR(unsigned reg);

  bool IsFPR(unsigned reg);

  size_t GetFPUSize() { return sizeof(RegisterInfoPOSIX_arm64::FPU); }

  bool IsSVE(unsigned reg) const;
  bool IsPAuth(unsigned reg) const;
  bool IsTLS(unsigned reg) const;
  bool IsSME(unsigned reg) const;
  bool IsMTE(unsigned reg) const;
  bool IsFPMR(unsigned reg) const;
  bool IsGCS(unsigned reg) const;

  bool IsSVEZ(unsigned reg) const { return m_register_info_up->IsSVEZReg(reg); }
  bool IsSVEP(unsigned reg) const { return m_register_info_up->IsSVEPReg(reg); }
  bool IsSVEVG(unsigned reg) const {
    return m_register_info_up->IsSVERegVG(reg);
  }
  bool IsSMEZA(unsigned reg) const {
    return m_register_info_up->IsSMERegZA(reg);
  }

  uint32_t GetRegNumSVEZ0() const {
    return m_register_info_up->GetRegNumSVEZ0();
  }
  uint32_t GetRegNumSVEFFR() const {
    return m_register_info_up->GetRegNumSVEFFR();
  }
  uint32_t GetRegNumFPCR() const { return m_register_info_up->GetRegNumFPCR(); }
  uint32_t GetRegNumFPSR() const { return m_register_info_up->GetRegNumFPSR(); }

  virtual bool ReadGPR() = 0;
  virtual bool ReadFPR() = 0;
  virtual bool WriteGPR() = 0;
  virtual bool WriteFPR() = 0;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTPOSIX_ARM64_H
