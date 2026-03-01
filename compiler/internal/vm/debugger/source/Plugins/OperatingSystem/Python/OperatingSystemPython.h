//===-- OperatingSystemPython.h ---------------------------------*- C++ -*-===//
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

#ifndef liblldb_OperatingSystemPython_h_
#define liblldb_OperatingSystemPython_h_

#include "lldb/Host/Config.h"

#if LLDB_ENABLE_PYTHON

#include "lldb/Target/DynamicRegisterInfo.h"
#include "lldb/Target/OperatingSystem.h"
#include "lldb/Utility/StructuredData.h"

namespace lldb_private {
class ScriptInterpreter;
}

class OperatingSystemPython : public lldb_private::OperatingSystem {
public:
  OperatingSystemPython(lldb_private::Process *process,
                        const lldb_private::FileSpec &python_module_path);

  ~OperatingSystemPython() override;

  // Static Functions
  static lldb_private::OperatingSystem *
  CreateInstance(lldb_private::Process *process, bool force);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "python"; }

  static llvm::StringRef GetPluginDescriptionStatic();

  // lldb_private::PluginInterface Methods
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  // lldb_private::OperatingSystem Methods
  bool UpdateThreadList(lldb_private::ThreadList &old_thread_list,
                        lldb_private::ThreadList &real_thread_list,
                        lldb_private::ThreadList &new_thread_list) override;

  void ThreadWasSelected(lldb_private::Thread *thread) override;

  lldb::RegisterContextSP
  CreateRegisterContextForThread(lldb_private::Thread *thread,
                                 lldb::addr_t reg_data_addr) override;

  lldb::StopInfoSP
  CreateThreadStopReason(lldb_private::Thread *thread) override;

  // Method for lazy creation of threads on demand
  lldb::ThreadSP CreateThread(lldb::tid_t tid, lldb::addr_t context) override;

  bool DoesPluginReportAllThreads() override;

protected:
  bool IsValid() const {
    return m_script_object_sp && m_script_object_sp->IsValid();
  }

  lldb::ThreadSP CreateThreadFromThreadInfo(
      lldb_private::StructuredData::Dictionary &thread_dict,
      lldb_private::ThreadList &core_thread_list,
      lldb_private::ThreadList &old_thread_list,
      std::vector<bool> &core_used_map, bool *did_create_ptr);

  lldb_private::DynamicRegisterInfo *GetDynamicRegisterInfo();

  lldb::ValueObjectSP m_thread_list_valobj_sp;
  std::unique_ptr<lldb_private::DynamicRegisterInfo> m_register_info_up;
  lldb_private::ScriptInterpreter *m_interpreter = nullptr;
  lldb::OperatingSystemInterfaceSP m_operating_system_interface_sp = nullptr;
  lldb_private::StructuredData::GenericSP m_script_object_sp = nullptr;
};

#endif // LLDB_ENABLE_PYTHON

#endif // liblldb_OperatingSystemPython_h_
