//===-- JITLoader.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_JITLOADER_H
#define LLDB_TARGET_JITLOADER_H

#include <vector>

#include "lldb/Core/PluginInterface.h"
#include "lldb/Target/JITLoaderList.h"

namespace lldb_private {

/// \class JITLoader JITLoader.h "lldb/Target/JITLoader.h"
/// A plug-in interface definition class for JIT loaders.
///
/// Plugins of this kind listen for code generated at runtime in the target.
/// They are very similar to dynamic loader, with the difference that they do
/// not have information about the target's dyld and that there may be
/// multiple JITLoader plugins per process, while there is at most one
/// DynamicLoader.
class JITLoader : public PluginInterface {
public:
  /// Find a JIT loader plugin for a given process.
  ///
  /// Scans the installed DynamicLoader plug-ins and tries to find all
  /// applicable instances for the current process.
  ///
  /// \param[in] process
  ///     The process for which to try and locate a JIT loader
  ///     plug-in instance.
  ///
  static void LoadPlugins(Process *process, lldb_private::JITLoaderList &list);

  /// Construct with a process.
  JITLoader(Process *process);

  ~JITLoader() override;

  /// Called after attaching a process.
  ///
  /// Allow JITLoader plug-ins to execute some code after attaching to a
  /// process.
  virtual void DidAttach() = 0;

  /// Called after launching a process.
  ///
  /// Allow JITLoader plug-ins to execute some code after the process has
  /// stopped for the first time on launch.
  virtual void DidLaunch() = 0;

  /// Called after a new shared object has been loaded so that it can be
  /// probed for JIT entry point hooks.
  virtual void ModulesDidLoad(lldb_private::ModuleList &module_list) = 0;

protected:
  // Member variables.
  Process *m_process;
};

} // namespace lldb_private

#endif // LLDB_TARGET_JITLOADER_H
