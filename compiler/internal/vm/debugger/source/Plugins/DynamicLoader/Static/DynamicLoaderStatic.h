//===-- DynamicLoaderStatic.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_DYNAMICLOADER_STATIC_DYNAMICLOADERSTATIC_H
#define LLDB_SOURCE_PLUGINS_DYNAMICLOADER_STATIC_DYNAMICLOADERSTATIC_H

#include "lldb/Target/DynamicLoader.h"
#include "lldb/Target/Process.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/UUID.h"

class DynamicLoaderStatic : public lldb_private::DynamicLoader {
public:
  DynamicLoaderStatic(lldb_private::Process *process);

  // Static Functions
  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "static"; }

  static llvm::StringRef GetPluginDescriptionStatic();

  static lldb_private::DynamicLoader *
  CreateInstance(lldb_private::Process *process, bool force);

  /// Called after attaching a process.
  ///
  /// Allow DynamicLoader plug-ins to execute some code after
  /// attaching to a process.
  void DidAttach() override;

  void DidLaunch() override;

  lldb::ThreadPlanSP GetStepThroughTrampolinePlan(lldb_private::Thread &thread,
                                                  bool stop_others) override;

  lldb_private::Status CanLoadImage() override;

  // PluginInterface protocol
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

private:
  void LoadAllImagesAtFileAddresses();
};

#endif // LLDB_SOURCE_PLUGINS_DYNAMICLOADER_STATIC_DYNAMICLOADERSTATIC_H
