//===-- ArchitectureAArch64.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_ARCHITECTURE_AARCH64_ARCHITECTUREAARCH64_H
#define LLDB_SOURCE_PLUGINS_ARCHITECTURE_AARCH64_ARCHITECTUREAARCH64_H

#include "Plugins/Process/Utility/MemoryTagManagerAArch64MTE.h"
#include "lldb/Core/Architecture.h"

namespace lldb_private {

class ArchitectureAArch64 : public Architecture {
public:
  static llvm::StringRef GetPluginNameStatic() { return "aarch64"; }
  static void Initialize();
  static void Terminate();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  void OverrideStopInfo(Thread &thread) const override {}

  const MemoryTagManager *GetMemoryTagManager() const override {
    return &m_memory_tag_manager;
  }

  bool
  RegisterWriteCausesReconfigure(const llvm::StringRef name) const override {
    // lldb treats svg as read only, so only vg can be written. This results in
    // the SVE registers changing size.
    return name == "vg";
  }

  bool ReconfigureRegisterInfo(DynamicRegisterInfo &reg_info,
                               DataExtractor &reg_data,
                               RegisterContext &reg_context) const override;

private:
  static std::unique_ptr<Architecture> Create(const ArchSpec &arch);
  ArchitectureAArch64() = default;
  MemoryTagManagerAArch64MTE m_memory_tag_manager;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_ARCHITECTURE_AARCH64_ARCHITECTUREAARCH64_H
