//===-- OperatingSystemPlugin.h ---------------------------------------===//
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

#include "lldb/Core/PluginManager.h"
#include "lldb/Target/OperatingSystem.h"
#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadList.h"

/// An operating system plugin that does nothing: simply keeps the thread lists
/// as they are.
class OperatingSystemIdentityMap : public lldb_private::OperatingSystem {
public:
  OperatingSystemIdentityMap(lldb_private::Process *process)
      : OperatingSystem(process) {}

  static OperatingSystem *CreateInstance(lldb_private::Process *process,
                                         bool force) {
    return new OperatingSystemIdentityMap(process);
  }
  static llvm::StringRef GetPluginNameStatic() { return "identity map"; }
  static llvm::StringRef GetPluginDescriptionStatic() { return ""; }

  static void Initialize() {
    lldb_private::PluginManager::RegisterPlugin(GetPluginNameStatic(),
                                                GetPluginDescriptionStatic(),
                                                CreateInstance, nullptr);
  }
  static void Terminate() {
    lldb_private::PluginManager::UnregisterPlugin(CreateInstance);
  }
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  // Simply adds the threads from real_thread_list into new_thread_list.
  bool UpdateThreadList(lldb_private::ThreadList &old_thread_list,
                        lldb_private::ThreadList &real_thread_list,
                        lldb_private::ThreadList &new_thread_list) override {
    for (const auto &real_thread : real_thread_list.Threads())
      new_thread_list.AddThread(real_thread);
    return true;
  }

  void ThreadWasSelected(lldb_private::Thread *thread) override {}

  lldb::RegisterContextSP
  CreateRegisterContextForThread(lldb_private::Thread *thread,
                                 lldb::addr_t reg_data_addr) override {
    return thread->GetRegisterContext();
  }

  lldb::StopInfoSP
  CreateThreadStopReason(lldb_private::Thread *thread) override {
    return thread->GetStopInfo();
  }
};
