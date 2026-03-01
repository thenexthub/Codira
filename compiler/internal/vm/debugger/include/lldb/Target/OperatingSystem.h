//===-- OperatingSystem.h ----------------------------------------------*- C++
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

#ifndef LLDB_TARGET_OPERATINGSYSTEM_H
#define LLDB_TARGET_OPERATINGSYSTEM_H

#include "lldb/Core/PluginInterface.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

/// \class OperatingSystem OperatingSystem.h "lldb/Target/OperatingSystem.h"
/// A plug-in interface definition class for halted OS helpers.
///
/// Halted OS plug-ins can be used by any process to locate and create
/// OS objects, like threads, during the lifetime of a debug session.
/// This is commonly used when attaching to an operating system that is
/// halted, such as when debugging over JTAG or connecting to low level kernel
/// debug services.

class OperatingSystem : public PluginInterface {
public:
  /// Find a halted OS plugin for a given process.
  ///
  /// Scans the installed OperatingSystem plug-ins and tries to find an
  /// instance that matches the current target triple and executable.
  ///
  /// \param[in] process
  ///     The process for which to try and locate a halted OS
  ///     plug-in instance.
  ///
  /// \param[in] plugin_name
  ///     An optional name of a specific halted OS plug-in that
  ///     should be used. If NULL, pick the best plug-in.
  static OperatingSystem *FindPlugin(Process *process, const char *plugin_name);

  OperatingSystem(Process *process);

  // Plug-in Methods
  virtual bool UpdateThreadList(ThreadList &old_thread_list,
                                ThreadList &real_thread_list,
                                ThreadList &new_thread_list) = 0;

  virtual void ThreadWasSelected(Thread *thread) = 0;

  virtual lldb::RegisterContextSP
  CreateRegisterContextForThread(Thread *thread,
                                 lldb::addr_t reg_data_addr) = 0;

  virtual lldb::StopInfoSP CreateThreadStopReason(Thread *thread) = 0;

  virtual lldb::ThreadSP CreateThread(lldb::tid_t tid, lldb::addr_t context) {
    return lldb::ThreadSP();
  }

  virtual bool IsOperatingSystemPluginThread(const lldb::ThreadSP &thread_sp);

  virtual bool DoesPluginReportAllThreads() = 0;

protected:
  // Member variables.
  Process
      *m_process; ///< The process that this dynamic loader plug-in is tracking.
};

} // namespace lldb_private

#endif // LLDB_TARGET_OPERATINGSYSTEM_H
