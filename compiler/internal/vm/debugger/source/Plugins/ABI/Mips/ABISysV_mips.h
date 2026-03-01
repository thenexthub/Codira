//===-- ABISysV_mips.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ABI_MIPS_ABISYSV_MIPS_H
#define LLDB_SOURCE_PLUGINS_ABI_MIPS_ABISYSV_MIPS_H

#include "lldb/Target/ABI.h"
#include "lldb/lldb-private.h"

class ABISysV_mips : public lldb_private::RegInfoBasedABI {
public:
  ~ABISysV_mips() override = default;

  size_t GetRedZoneSize() const override;

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

  bool IsSoftFloat(uint32_t fp_flag) const;

  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    // Make sure the stack call frame addresses are 8 byte aligned
    if (cfa & (8ull - 1ull))
      return false; // Not 8 byte aligned
    if (cfa == 0)
      return false; // Zero is not a valid stack address
    return true;
  }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    // Just make sure the address is a valid 32 bit address. Bit zero
    // might be set due to MicroMIPS function calls, so don't enforce alignment.
    return (pc <= UINT32_MAX);
  }

  const lldb_private::RegisterInfo *
  GetRegisterInfoArray(uint32_t &count) override;

  // Static Functions

  static void Initialize();

  static void Terminate();

  static lldb::ABISP CreateInstance(lldb::ProcessSP process_sp, const lldb_private::ArchSpec &arch);

  static llvm::StringRef GetPluginNameStatic() { return "sysv-mips"; }

  // PluginInterface protocol

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

protected:
  void CreateRegisterMapIfNeeded();

  lldb::ValueObjectSP
  GetReturnValueObjectSimple(lldb_private::Thread &thread,
                             lldb_private::CompilerType &ast_type) const;

  bool RegisterIsCalleeSaved(const lldb_private::RegisterInfo *reg_info);

private:
  using lldb_private::RegInfoBasedABI::RegInfoBasedABI; // Call CreateInstance instead.
};

#endif // LLDB_SOURCE_PLUGINS_ABI_MIPS_ABISYSV_MIPS_H
