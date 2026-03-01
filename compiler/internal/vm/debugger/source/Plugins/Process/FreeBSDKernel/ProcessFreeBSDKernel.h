//===-- ProcessFreeBSDKernel.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_PROCESSFREEBSDKERNEL_H
#define LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_PROCESSFREEBSDKERNEL_H

#include "lldb/Target/PostMortemProcess.h"

class ProcessFreeBSDKernel : public lldb_private::PostMortemProcess {
public:
  ProcessFreeBSDKernel(lldb::TargetSP target_sp, lldb::ListenerSP listener,
                       const lldb_private::FileSpec &core_file);

  static lldb::ProcessSP
  CreateInstance(lldb::TargetSP target_sp, lldb::ListenerSP listener,
                 const lldb_private::FileSpec *crash_file_path,
                 bool can_connect);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "freebsd-kernel"; }

  static llvm::StringRef GetPluginDescriptionStatic() {
    return "FreeBSD kernel vmcore debugging plug-in.";
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  lldb_private::Status DoDestroy() override;

  bool CanDebug(lldb::TargetSP target_sp,
                bool plugin_specified_by_name) override;

  void RefreshStateAfterStop() override;

  lldb_private::Status DoLoadCore() override;

  lldb_private::DynamicLoader *GetDynamicLoader() override;

protected:
  bool DoUpdateThreadList(lldb_private::ThreadList &old_thread_list,
                          lldb_private::ThreadList &new_thread_list) override;

  lldb::addr_t FindSymbol(const char* name);
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_PROCESSFREEBSDKERNEL_H
