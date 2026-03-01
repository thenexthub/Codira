//===-- SBExecutionContext.h -----------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_API_SBEXECUTIONCONTEXT_H
#define LLDB_API_SBEXECUTIONCONTEXT_H

#include "lldb/API/SBDefines.h"

#include <cstdio>
#include <vector>

namespace lldb_private {
class ScriptInterpreter;
namespace python {
class SWIGBridge;
}
} // namespace lldb_private

namespace lldb {

class LLDB_API SBExecutionContext {
  friend class SBCommandInterpreter;

public:
  SBExecutionContext();

  SBExecutionContext(const lldb::SBExecutionContext &rhs);

  SBExecutionContext(const lldb::SBTarget &target);

  SBExecutionContext(const lldb::SBProcess &process);

  SBExecutionContext(lldb::SBThread thread); // can't be a const& because
                                             // SBThread::get() isn't itself a
                                             // const function

  SBExecutionContext(const lldb::SBFrame &frame);

  ~SBExecutionContext();

  const SBExecutionContext &operator=(const lldb::SBExecutionContext &rhs);

  SBTarget GetTarget() const;

  SBProcess GetProcess() const;

  SBThread GetThread() const;

  SBFrame GetFrame() const;

protected:
  friend class SBInstructionList;
  friend class lldb_private::python::SWIGBridge;
  friend class lldb_private::ScriptInterpreter;

  lldb_private::ExecutionContextRef *get() const;

  SBExecutionContext(lldb::ExecutionContextRefSP exe_ctx_ref_sp);

private:
  mutable lldb::ExecutionContextRefSP m_exe_ctx_sp;
};

} // namespace lldb

#endif // LLDB_API_SBEXECUTIONCONTEXT_H
