//===-- ABISysV_arc.h -------------------------------------------*- C++ -*-===//
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

#ifndef liblldb_ABISysV_arc_h_
#define liblldb_ABISysV_arc_h_

// Other libraries and framework includes
#include <optional>

// Project includes
#include "lldb/Target/ABI.h"
#include "lldb/lldb-private.h"

class ABISysV_arc : public lldb_private::RegInfoBasedABI {
public:
  ~ABISysV_arc() override = default;

  size_t GetRedZoneSize() const override;

  bool PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                          lldb::addr_t functionAddress,
                          lldb::addr_t returnAddress,
                          llvm::ArrayRef<lldb::addr_t> args) const override;

  // Special thread plan for GDB style non-jit function calls.
  bool
  PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                     lldb::addr_t functionAddress, lldb::addr_t returnAddress,
                     llvm::Type &prototype,
                     llvm::ArrayRef<ABI::CallArgument> args) const override;

  bool GetArgumentValues(lldb_private::Thread &thread,
                         lldb_private::ValueList &values) const override;

  lldb_private::Status
  SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                       lldb::ValueObjectSP &new_value) override;

  lldb::ValueObjectSP
  GetReturnValueObjectImpl(lldb_private::Thread &thread,
                           lldb_private::CompilerType &type) const override;

  // Specialized to work with llvm IR types.
  lldb::ValueObjectSP GetReturnValueObjectImpl(lldb_private::Thread &thread,
                                               llvm::Type &type) const override;

  lldb::UnwindPlanSP CreateFunctionEntryUnwindPlan() override;

  lldb::UnwindPlanSP CreateDefaultUnwindPlan() override;

  bool RegisterIsVolatile(const lldb_private::RegisterInfo *reg_info) override;

  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    // Stack call frame address must be 4 byte aligned.
    return (cfa & 0x3ull) == 0;
  }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    // Code addresse must be 2 byte aligned.
    return (pc & 1ull) == 0;
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

  static llvm::StringRef GetPluginNameStatic() { return "sysv-arc"; }

  //------------------------------------------------------------------
  // PluginInterface protocol
  //------------------------------------------------------------------

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

private:
  lldb::ValueObjectSP
  GetReturnValueObjectSimple(lldb_private::Thread &thread,
                             lldb_private::CompilerType &ast_type) const;

  bool IsRegisterFileReduced(lldb_private::RegisterContext &reg_ctx) const;

  using lldb_private::RegInfoBasedABI::RegInfoBasedABI; // Call CreateInstance instead.

  using RegisterFileFlag = std::optional<bool>;
  mutable RegisterFileFlag m_is_reg_file_reduced;
};

#endif // liblldb_ABISysV_arc_h_
