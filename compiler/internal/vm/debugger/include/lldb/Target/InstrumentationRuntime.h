//===-- InstrumentationRuntime.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_INSTRUMENTATIONRUNTIME_H
#define LLDB_TARGET_INSTRUMENTATIONRUNTIME_H

#include <map>
#include <vector>

#include "lldb/Core/PluginInterface.h"
#include "lldb/Utility/StructuredData.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-private.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

typedef std::map<lldb::InstrumentationRuntimeType,
                 lldb::InstrumentationRuntimeSP>
    InstrumentationRuntimeCollection;

class InstrumentationRuntime
    : public std::enable_shared_from_this<InstrumentationRuntime>,
      public PluginInterface {
  /// The instrumented process.
  lldb::ProcessWP m_process_wp;

  /// The module containing the instrumentation runtime.
  lldb::ModuleSP m_runtime_module;

  /// The breakpoint in the instrumentation runtime.
  lldb::user_id_t m_breakpoint_id;

  /// Indicates whether or not breakpoints have been registered in the
  /// instrumentation runtime.
  bool m_is_active;

protected:
  InstrumentationRuntime(const lldb::ProcessSP &process_sp)
      : m_breakpoint_id(0), m_is_active(false) {
    if (process_sp)
      m_process_wp = process_sp;
  }

  lldb::ProcessSP GetProcessSP() { return m_process_wp.lock(); }

  lldb::ModuleSP GetRuntimeModuleSP() { return m_runtime_module; }

  void SetRuntimeModuleSP(lldb::ModuleSP module_sp) {
    m_runtime_module = std::move(module_sp);
  }

  lldb::user_id_t GetBreakpointID() const { return m_breakpoint_id; }

  void SetBreakpointID(lldb::user_id_t ID) { m_breakpoint_id = ID; }

  void SetActive(bool IsActive) { m_is_active = IsActive; }

  /// Return a regular expression which can be used to identify a valid version
  /// of the runtime library.
  virtual const RegularExpression &GetPatternForRuntimeLibrary() = 0;

  /// Check whether \p module_sp corresponds to a valid runtime library.
  virtual bool CheckIfRuntimeIsValid(const lldb::ModuleSP module_sp) = 0;

  /// Register a breakpoint in the runtime library and perform any other
  /// necessary initialization. The runtime library
  /// is guaranteed to be loaded.
  virtual void Activate() = 0;

  /// \return true if `CheckIfRuntimeIsValid` should be called on all modules.
  /// In this case the return value of `GetPatternForRuntimeLibrary` will be
  /// ignored. Return false if `CheckIfRuntimeIsValid` should only be called
  /// for modules whose name matches `GetPatternForRuntimeLibrary`.
  ///
  virtual bool MatchAllModules() { return false; }

public:
  static void ModulesDidLoad(lldb_private::ModuleList &module_list,
                             Process *process,
                             InstrumentationRuntimeCollection &runtimes);

  /// Look for the instrumentation runtime in \p module_list. Register and
  /// activate the runtime if this hasn't already
  /// been done.
  void ModulesDidLoad(lldb_private::ModuleList &module_list);

  bool IsActive() const { return m_is_active; }

  virtual lldb::ThreadCollectionSP
  GetBacktracesFromExtendedStopInfo(StructuredData::ObjectSP info);
};

} // namespace lldb_private

#endif // LLDB_TARGET_INSTRUMENTATIONRUNTIME_H
