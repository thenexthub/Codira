//===-- PlatformRemoteiOS.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMREMOTEIOS_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMREMOTEIOS_H

#include "PlatformRemoteDarwinDevice.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/StringRef.h"

#include <vector>

namespace lldb_private {
class ArchSpec;

class PlatformRemoteiOS : public PlatformRemoteDarwinDevice {
public:
  PlatformRemoteiOS();

  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "remote-ios"; }

  static llvm::StringRef GetDescriptionStatic();

  llvm::StringRef GetDescription() override { return GetDescriptionStatic(); }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override;

protected:
  bool CheckLocalSharedCache() const override;

  llvm::StringRef GetDeviceSupportDirectoryName() override;
  llvm::StringRef GetPlatformName() override;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMREMOTEIOS_H
