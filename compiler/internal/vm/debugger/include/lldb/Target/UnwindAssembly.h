//===-- UnwindAssembly.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_UNWINDASSEMBLY_H
#define LLDB_TARGET_UNWINDASSEMBLY_H

#include "lldb/Core/PluginInterface.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class UnwindAssembly : public std::enable_shared_from_this<UnwindAssembly>,
                       public PluginInterface {
public:
  static lldb::UnwindAssemblySP FindPlugin(const ArchSpec &arch);

  virtual bool
  GetNonCallSiteUnwindPlanFromAssembly(AddressRange &func, Thread &thread,
                                       UnwindPlan &unwind_plan) = 0;

  virtual bool AugmentUnwindPlanFromCallSite(AddressRange &func, Thread &thread,
                                             UnwindPlan &unwind_plan) = 0;

  virtual bool GetFastUnwindPlan(AddressRange &func, Thread &thread,
                                 UnwindPlan &unwind_plan) = 0;

  // thread may be NULL in which case we only use the Target (e.g. if this is
  // called pre-process-launch).
  virtual bool
  FirstNonPrologueInsn(AddressRange &func,
                       const lldb_private::ExecutionContext &exe_ctx,
                       Address &first_non_prologue_insn) = 0;

protected:
  UnwindAssembly(const ArchSpec &arch);
  ArchSpec m_arch;
};

} // namespace lldb_private

#endif // LLDB_TARGET_UNWINDASSEMBLY_H
