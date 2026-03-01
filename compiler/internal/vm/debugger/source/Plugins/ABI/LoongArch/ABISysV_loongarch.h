//===-- ABISysV_loongarch.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ABI_LOONGARCH_ABISYSV_LOONGARCH_H
#define LLDB_SOURCE_PLUGINS_ABI_LOONGARCH_ABISYSV_LOONGARCH_H

// Other libraries and framework includes
#include "llvm/TargetParser/Triple.h"

// Project includes
#include "lldb/Target/ABI.h"
#include "lldb/Target/Process.h"
#include "lldb/Utility/Flags.h"
#include "lldb/lldb-private.h"

class ABISysV_loongarch : public lldb_private::RegInfoBasedABI {
public:
  ~ABISysV_loongarch() override = default;

  size_t GetRedZoneSize() const override { return 0; }

  bool PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                          lldb::addr_t functionAddress,
                          lldb::addr_t returnAddress,
                          llvm::ArrayRef<lldb::addr_t> args) const override;

  bool GetArgumentValues(lldb_private::Thread &thread,
                         lldb_private::ValueList &values) const override;

  lldb_private::Status
  SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                       lldb::ValueObjectSP &new_value) override;

  lldb::ValueObjectSP
  GetReturnValueObjectImpl(lldb_private::Thread &thread,
                           lldb_private::CompilerType &type) const override;

  lldb::UnwindPlanSP CreateFunctionEntryUnwindPlan() override;

  lldb::UnwindPlanSP CreateDefaultUnwindPlan() override;

  bool RegisterIsVolatile(const lldb_private::RegisterInfo *reg_info) override;

  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    // The CFA must be 16 byte aligned.
    return (cfa & 0xfull) == 0;
  }

  void SetIsLA64(bool is_la64) { m_is_la64 = is_la64; }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    // Code address must be 4 byte aligned.
    if (pc & (4ull - 1ull))
      return false;

    return true;
  }

  const lldb_private::RegisterInfo *
  GetRegisterInfoArray(uint32_t &count) override;

  //------------------------------------------------------------------
  // Static Functions
  //------------------------------------------------------------------

  static void Initialize();

  static void Terminate();

  static lldb::ABISP CreateInstance(lldb::ProcessSP process_sp,
                                    const lldb_private::ArchSpec &arch);

  static llvm::StringRef GetPluginNameStatic() { return "sysv-loongarch"; }

  //------------------------------------------------------------------
  // PluginInterface protocol
  //------------------------------------------------------------------

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

protected:
  void AugmentRegisterInfo(
      std::vector<lldb_private::DynamicRegisterInfo::Register> &regs) override;

  bool RegisterIsCalleeSaved(const lldb_private::RegisterInfo *reg_info);

private:
  lldb::ValueObjectSP
  GetReturnValueObjectSimple(lldb_private::Thread &thread,
                             lldb_private::CompilerType &ast_type) const;

  using lldb_private::RegInfoBasedABI::RegInfoBasedABI; // Call CreateInstance
                                                        // instead.
  bool m_is_la64;
};

#endif // LLDB_SOURCE_PLUGINS_ABI_LOONGARCH_ABISYSV_LOONGARCH_H
