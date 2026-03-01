//===-- ABIMacOSX_i386.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ABI_X86_ABIMACOSX_I386_H
#define LLDB_SOURCE_PLUGINS_ABI_X86_ABIMACOSX_I386_H

#include "Plugins/ABI/X86/ABIX86_i386.h"
#include "lldb/Core/Value.h"
#include "lldb/lldb-private.h"

class ABIMacOSX_i386 : public ABIX86_i386 {
public:
  ~ABIMacOSX_i386() override = default;

  size_t GetRedZoneSize() const override;

  bool PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                          lldb::addr_t func_addr, lldb::addr_t return_addr,
                          llvm::ArrayRef<lldb::addr_t> args) const override;

  bool GetArgumentValues(lldb_private::Thread &thread,
                         lldb_private::ValueList &values) const override;

  lldb_private::Status
  SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                       lldb::ValueObjectSP &new_value) override;

  lldb::UnwindPlanSP CreateFunctionEntryUnwindPlan() override;

  lldb::UnwindPlanSP CreateDefaultUnwindPlan() override;

  bool RegisterIsVolatile(const lldb_private::RegisterInfo *reg_info) override;

  // The Darwin i386 ABI requires that stack frames be 16 byte aligned.
  // When there is a trap handler on the stack, e.g. _sigtramp in userland
  // code, we've seen that the stack pointer is often not aligned properly
  // before the handler is invoked.  This means that lldb will stop the unwind
  // early -- before the function which caused the trap.
  //
  // To work around this, we relax that alignment to be just word-size
  // (4-bytes).
  // Allowing the trap handlers for user space would be easy (_sigtramp) but
  // in other environments there can be a large number of different functions
  // involved in async traps.
  //
  // If we were to enforce 16-byte alignment, we also need to relax to 4-byte
  // alignment for non-darwin i386 targets.
  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    // Make sure the stack call frame addresses are 4 byte aligned
    if (cfa & (4ull - 1ull))
      return false; // Not 4 byte aligned
    if (cfa == 0)
      return false; // Zero is not a valid stack address
    return true;
  }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    // Just make sure the address is a valid 32 bit address.
    return pc <= UINT32_MAX;
  }

  // Static Functions

  static void Initialize();

  static void Terminate();

  static lldb::ABISP CreateInstance(lldb::ProcessSP process_sp, const lldb_private::ArchSpec &arch);

  // PluginInterface protocol

  static llvm::StringRef GetPluginNameStatic() { return "abi.macosx-i386"; }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

protected:
  lldb::ValueObjectSP
  GetReturnValueObjectImpl(lldb_private::Thread &thread,
                           lldb_private::CompilerType &ast_type) const override;

  bool RegisterIsCalleeSaved(const lldb_private::RegisterInfo *reg_info);

  std::string GetMCName(std::string name) override {
    MapRegisterName(name, "stmm", "st");
    return name;
  }

private:
  using ABIX86_i386::ABIX86_i386; // Call CreateInstance instead.
};

#endif // LLDB_SOURCE_PLUGINS_ABI_X86_ABIMACOSX_I386_H
