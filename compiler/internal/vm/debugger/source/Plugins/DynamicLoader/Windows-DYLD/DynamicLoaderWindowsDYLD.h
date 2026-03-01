//===-- DynamicLoaderWindowsDYLD.h ------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_DYNAMICLOADER_WINDOWS_DYLD_DYNAMICLOADERWINDOWSDYLD_H
#define LLDB_SOURCE_PLUGINS_DYNAMICLOADER_WINDOWS_DYLD_DYNAMICLOADERWINDOWSDYLD_H

#include "lldb/Target/DynamicLoader.h"
#include "lldb/lldb-forward.h"

#include <map>

namespace lldb_private {

class DynamicLoaderWindowsDYLD : public DynamicLoader {
public:
  DynamicLoaderWindowsDYLD(Process *process);

  ~DynamicLoaderWindowsDYLD() override;

  static void Initialize();
  static void Terminate();
  static llvm::StringRef GetPluginNameStatic() { return "windows-dyld"; }
  static llvm::StringRef GetPluginDescriptionStatic();

  static DynamicLoader *CreateInstance(Process *process, bool force);

  void OnLoadModule(lldb::ModuleSP module_sp, const ModuleSpec module_spec,
                    lldb::addr_t module_addr);
  void OnUnloadModule(lldb::addr_t module_addr);

  void DidAttach() override;
  void DidLaunch() override;
  Status CanLoadImage() override;
  lldb::ThreadPlanSP GetStepThroughTrampolinePlan(Thread &thread,
                                                  bool stop) override;

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

protected:
  lldb::addr_t GetLoadAddress(lldb::ModuleSP executable);

private:
  std::map<lldb::ModuleWP, lldb::addr_t, std::owner_less<lldb::ModuleWP>>
      m_loaded_modules;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_DYNAMICLOADER_WINDOWS_DYLD_DYNAMICLOADERWINDOWSDYLD_H
