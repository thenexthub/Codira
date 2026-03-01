//===-- InstrumentationRuntimeASanLibsanitizers.h ---------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_ASANLIBSANITIZERS_INSTRUMENTATIONRUNTIMEASANLIBSANITIZERS_H
#define LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_ASANLIBSANITIZERS_INSTRUMENTATIONRUNTIMEASANLIBSANITIZERS_H

#include "lldb/Target/InstrumentationRuntime.h"

class InstrumentationRuntimeASanLibsanitizers
    : public lldb_private::InstrumentationRuntime {
public:
  ~InstrumentationRuntimeASanLibsanitizers() override;

  static lldb::InstrumentationRuntimeSP
  CreateInstance(const lldb::ProcessSP &process_sp);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "Libsanitizers-ASan"; }

  static lldb::InstrumentationRuntimeType GetTypeStatic();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  virtual lldb::InstrumentationRuntimeType GetType() { return GetTypeStatic(); }

private:
  InstrumentationRuntimeASanLibsanitizers(const lldb::ProcessSP &process_sp)
      : lldb_private::InstrumentationRuntime(process_sp) {}

  const lldb_private::RegularExpression &GetPatternForRuntimeLibrary() override;

  bool CheckIfRuntimeIsValid(const lldb::ModuleSP module_sp) override;

  void Activate() override;

  void Deactivate();

  static bool
  NotifyBreakpointHit(void *baton,
                      lldb_private::StoppointCallbackContext *context,
                      lldb::user_id_t break_id, lldb::user_id_t break_loc_id);
};

#endif // LLDB_SOURCE_PLUGINS_INSTRUMENTATIONRUNTIME_ASANLIBSANITIZERS_INSTRUMENTATIONRUNTIMEASANLIBSANITIZERS_H
