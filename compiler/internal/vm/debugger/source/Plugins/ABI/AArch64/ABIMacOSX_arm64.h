//===-- ABIMacOSX_arm64.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ABI_AARCH64_ABIMACOSX_ARM64_H
#define LLDB_SOURCE_PLUGINS_ABI_AARCH64_ABIMACOSX_ARM64_H

#include "Plugins/ABI/AArch64/ABIAArch64.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/lldb-private.h"

class ABIMacOSX_arm64 : public ABIAArch64 {
public:
  ~ABIMacOSX_arm64() override = default;

  size_t GetRedZoneSize() const override;

  bool PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                          lldb::addr_t functionAddress,
                          lldb::addr_t returnAddress,
                          llvm::ArrayRef<lldb::addr_t> args) const override;

  bool GetArgumentValues(lldb_private::Thread &thread,
                         lldb_private::ValueList &values) const override;

  bool RegisterIsVolatile(const lldb_private::RegisterInfo *reg_info) override;

  // The arm64 ABI requires that stack frames be 16 byte aligned.
  // When there is a trap handler on the stack, e.g. _sigtramp in userland
  // code, we've seen that the stack pointer is often not aligned properly
  // before the handler is invoked.  This means that lldb will stop the unwind
  // early -- before the function which caused the trap.
  //
  // To work around this, we relax that alignment to be just word-size
  // (8-bytes).
  // Allowing the trap handlers for user space would be easy (_sigtramp) but
  // in other environments there can be a large number of different functions
  // involved in async traps.
  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    // Make sure the stack call frame addresses are 8 byte aligned
    if (cfa & (8ull - 1ull))
      return false; // Not 8 byte aligned
    if (cfa == 0)
      return false; // Zero is not a valid stack address
    return true;
  }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    if (pc & (4ull - 1ull))
      return false; // Not 4 byte aligned

    // Anything else if fair game..
    return true;
  }

  lldb::addr_t FixCodeAddress(lldb::addr_t pc) override;
  lldb::addr_t FixDataAddress(lldb::addr_t pc) override;

  // Static Functions

  static void Initialize();

  static void Terminate();

  static lldb::ABISP CreateInstance(lldb::ProcessSP process_sp, const lldb_private::ArchSpec &arch);

  // PluginInterface protocol

  static llvm::StringRef GetPluginNameStatic() { return "ABIMacOSX_arm64"; }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  lldb_private::Status
  SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                       lldb::ValueObjectSP &new_value) override;

protected:
  lldb::ValueObjectSP
  GetReturnValueObjectImpl(lldb_private::Thread &thread,
                           lldb_private::CompilerType &ast_type) const override;

private:
  using ABIAArch64::ABIAArch64; // Call CreateInstance instead.
};

#endif // LLDB_SOURCE_PLUGINS_ABI_AARCH64_ABIMACOSX_ARM64_H
