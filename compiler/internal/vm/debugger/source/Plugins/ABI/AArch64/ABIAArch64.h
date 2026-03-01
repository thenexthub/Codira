//===-- AArch64.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ABI_AARCH64_ABIAARCH64_H
#define LLDB_SOURCE_PLUGINS_ABI_AARCH64_ABIAARCH64_H

#include "lldb/Target/ABI.h"

class ABIAArch64 : public lldb_private::MCBasedABI {
public:
  static void Initialize();
  static void Terminate();

  lldb::addr_t FixCodeAddress(lldb::addr_t pc) override;
  lldb::addr_t FixDataAddress(lldb::addr_t pc) override;

  lldb::UnwindPlanSP CreateFunctionEntryUnwindPlan() override;
  lldb::UnwindPlanSP CreateDefaultUnwindPlan() override;

protected:
  virtual lldb::addr_t FixAddress(lldb::addr_t pc, lldb::addr_t mask) {
    return pc;
  }

  std::pair<uint32_t, uint32_t>
  GetEHAndDWARFNums(llvm::StringRef name) override;

  std::string GetMCName(std::string reg) override;

  uint32_t GetGenericNum(llvm::StringRef name) override;

  void AugmentRegisterInfo(
      std::vector<lldb_private::DynamicRegisterInfo::Register> &regs) override;

  using lldb_private::MCBasedABI::MCBasedABI;
};
#endif
