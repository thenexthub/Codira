//===-- ScriptedThread.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SCRIPTED_THREAD_H
#define LLDB_SOURCE_PLUGINS_SCRIPTED_THREAD_H

#include <string>

#include "ScriptedProcess.h"

#include "Plugins/Process/Utility/RegisterContextMemory.h"
#include "lldb/Interpreter/ScriptInterpreter.h"
#include "lldb/Target/DynamicRegisterInfo.h"
#include "lldb/Target/Thread.h"

namespace lldb_private {
class ScriptedProcess;
class ScriptedFrame;
}

namespace lldb_private {

class ScriptedThread : public lldb_private::Thread {

public:
  ScriptedThread(ScriptedProcess &process,
                 lldb::ScriptedThreadInterfaceSP interface_sp, lldb::tid_t tid,
                 StructuredData::GenericSP script_object_sp = nullptr);

  ~ScriptedThread() override;

  static llvm::Expected<std::shared_ptr<ScriptedThread>>
  Create(ScriptedProcess &process,
         StructuredData::Generic *script_object = nullptr);

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(lldb_private::StackFrame *frame) override;

  bool LoadArtificialStackFrames();

  bool CalculateStopInfo() override;

  const char *GetInfo() override { return nullptr; }

  const char *GetName() override;

  const char *GetQueueName() override;

  void WillResume(lldb::StateType resume_state) override;

  void RefreshStateAfterStop() override;

  void ClearStackFrames() override;

  StructuredData::ObjectSP FetchThreadExtendedInfo() override;

private:
  friend class ScriptedFrame;

  void CheckInterpreterAndScriptObject() const;
  lldb::ScriptedThreadInterfaceSP GetInterface() const;

  ScriptedThread(const ScriptedThread &) = delete;
  const ScriptedThread &operator=(const ScriptedThread &) = delete;

  std::shared_ptr<DynamicRegisterInfo> GetDynamicRegisterInfo();

  const ScriptedProcess &m_scripted_process;
  lldb::ScriptedThreadInterfaceSP m_scripted_thread_interface_sp = nullptr;
  lldb_private::StructuredData::GenericSP m_script_object_sp = nullptr;
  std::shared_ptr<DynamicRegisterInfo> m_register_info_sp = nullptr;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SCRIPTED_THREAD_H
