//===----------------------------------------------------------------------===//
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

#ifndef LLDB_PLUGINS_SYNTHETICFRAMEPROVIDER_SCRIPTEDFRAMEPROVIDER_SCRIPTEDFRAMEPROVIDER_H
#define LLDB_PLUGINS_SYNTHETICFRAMEPROVIDER_SCRIPTEDFRAMEPROVIDER_SCRIPTEDFRAMEPROVIDER_H

#include "lldb/Target/SyntheticFrameProvider.h"
#include "lldb/Utility/ScriptedMetadata.h"
#include "lldb/Utility/Status.h"
#include "lldb/lldb-forward.h"
#include "llvm/Support/Error.h"

namespace lldb_private {

class ScriptedFrameProvider : public SyntheticFrameProvider {
public:
  static llvm::StringRef GetPluginNameStatic() {
    return "ScriptedFrameProvider";
  }

  static llvm::Expected<lldb::SyntheticFrameProviderSP>
  CreateInstance(lldb::StackFrameListSP input_frames,
                 const ScriptedFrameProviderDescriptor &descriptor);

  static void Initialize();

  static void Terminate();

  ScriptedFrameProvider(lldb::StackFrameListSP input_frames,
                        lldb::ScriptedFrameProviderInterfaceSP interface_sp,
                        const ScriptedFrameProviderDescriptor &descriptor);
  ~ScriptedFrameProvider() override;

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  std::string GetDescription() const override;

  std::optional<uint32_t> GetPriority() const override;

  /// Get a single stack frame at the specified index.
  llvm::Expected<lldb::StackFrameSP> GetFrameAtIndex(uint32_t idx) override;

private:
  lldb::ScriptedFrameProviderInterfaceSP m_interface_sp;
  const ScriptedFrameProviderDescriptor &m_descriptor;
};

} // namespace lldb_private

#endif // LLDB_PLUGINS_SYNTHETICFRAMEPROVIDER_SCRIPTEDFRAMEPROVIDER_SCRIPTEDFRAMEPROVIDER_H
