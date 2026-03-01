//===-- IDebugDelegate.h ----------------------------------------*- C++ -*-===//
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

#ifndef liblldb_Plugins_Process_Windows_IDebugDelegate_H_
#define liblldb_Plugins_Process_Windows_IDebugDelegate_H_

#include "ForwardDecl.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-types.h"
#include <string>

namespace lldb_private {
class Status;
class HostThread;

// IDebugDelegate
//
// IDebugDelegate defines an interface which allows implementors to receive
// notification of events that happen in a debugged process.
class IDebugDelegate {
public:
  virtual ~IDebugDelegate() {}

  virtual void OnExitProcess(uint32_t exit_code) = 0;
  virtual void OnDebuggerConnected(lldb::addr_t image_base) = 0;
  virtual ExceptionResult OnDebugException(bool first_chance,
                                           const ExceptionRecord &record) = 0;
  virtual void OnCreateThread(const HostThread &thread) = 0;
  virtual void OnExitThread(lldb::tid_t thread_id, uint32_t exit_code) = 0;
  virtual void OnLoadDll(const ModuleSpec &module_spec,
                         lldb::addr_t module_addr) = 0;
  virtual void OnUnloadDll(lldb::addr_t module_addr) = 0;
  virtual void OnDebugString(const std::string &string) = 0;
  virtual void OnDebuggerError(const Status &error, uint32_t type) = 0;
};
}

#endif
