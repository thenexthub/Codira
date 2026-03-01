//===-- ArchitectureMips.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ARCHITECTURE_MIPS_ARCHITECTUREMIPS_H
#define LLDB_SOURCE_PLUGINS_ARCHITECTURE_MIPS_ARCHITECTUREMIPS_H

#include "lldb/Core/Architecture.h"
#include "lldb/Utility/ArchSpec.h"

namespace lldb_private {

class ArchitectureMips : public Architecture {
public:
  static llvm::StringRef GetPluginNameStatic() { return "mips"; }
  static void Initialize();
  static void Terminate();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  void OverrideStopInfo(Thread &thread) const override {}

  lldb::addr_t GetBreakableLoadAddress(lldb::addr_t addr,
                                       Target &) const override;

  lldb::addr_t GetCallableLoadAddress(lldb::addr_t load_addr,
                                      AddressClass addr_class) const override;

  lldb::addr_t GetOpcodeLoadAddress(lldb::addr_t load_addr,
                                    AddressClass addr_class) const override;

private:
  Instruction *GetInstructionAtAddress(Target &target,
                                       const Address &resolved_addr,
                                       lldb::addr_t symbol_offset) const;

  static std::unique_ptr<Architecture> Create(const ArchSpec &arch);
  ArchitectureMips(const ArchSpec &arch) : m_arch(arch) {}

  ArchSpec m_arch;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_ARCHITECTURE_MIPS_ARCHITECTUREMIPS_H
