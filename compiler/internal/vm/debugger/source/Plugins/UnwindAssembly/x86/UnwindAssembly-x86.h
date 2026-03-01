//===-- UnwindAssembly-x86.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_UNWINDASSEMBLY_X86_UNWINDASSEMBLY_X86_H
#define LLDB_SOURCE_PLUGINS_UNWINDASSEMBLY_X86_UNWINDASSEMBLY_X86_H

#include "x86AssemblyInspectionEngine.h"

#include "lldb/Target/UnwindAssembly.h"
#include "lldb/lldb-private.h"

class UnwindAssembly_x86 : public lldb_private::UnwindAssembly {
public:
  ~UnwindAssembly_x86() override;

  bool GetNonCallSiteUnwindPlanFromAssembly(
      lldb_private::AddressRange &func, lldb_private::Thread &thread,
      lldb_private::UnwindPlan &unwind_plan) override;

  bool
  AugmentUnwindPlanFromCallSite(lldb_private::AddressRange &func,
                                lldb_private::Thread &thread,
                                lldb_private::UnwindPlan &unwind_plan) override;

  bool GetFastUnwindPlan(lldb_private::AddressRange &func,
                         lldb_private::Thread &thread,
                         lldb_private::UnwindPlan &unwind_plan) override;

  // thread may be NULL in which case we only use the Target (e.g. if this is
  // called pre-process-launch).
  bool
  FirstNonPrologueInsn(lldb_private::AddressRange &func,
                       const lldb_private::ExecutionContext &exe_ctx,
                       lldb_private::Address &first_non_prologue_insn) override;

  static lldb_private::UnwindAssembly *
  CreateInstance(const lldb_private::ArchSpec &arch);

  // PluginInterface protocol
  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "x86"; }

  static llvm::StringRef GetPluginDescriptionStatic();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

private:
  UnwindAssembly_x86(const lldb_private::ArchSpec &arch);

  lldb_private::ArchSpec m_arch;

  lldb_private::x86AssemblyInspectionEngine *m_assembly_inspection_engine;
};

#endif // LLDB_SOURCE_PLUGINS_UNWINDASSEMBLY_X86_UNWINDASSEMBLY_X86_H
