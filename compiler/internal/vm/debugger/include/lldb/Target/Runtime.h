//===-- Runtime.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_RUNTIME_H
#define LLDB_TARGET_RUNTIME_H

#include "lldb/Target/Process.h"

namespace lldb_private {
class Runtime {
public:
  Runtime(Process *process) : m_process(process) {}
  virtual ~Runtime() = default;
  Runtime(const Runtime &) = delete;
  const Runtime &operator=(const Runtime &) = delete;

  Process *GetProcess() { return m_process; }
  Target &GetTargetRef() { return m_process->GetTarget(); }

  /// Called when modules have been loaded in the process.
  virtual void ModulesDidLoad(const ModuleList &module_list) = 0;

protected:
  Process *m_process;
};
} // namespace lldb_private

#endif // LLDB_TARGET_RUNTIME_H
