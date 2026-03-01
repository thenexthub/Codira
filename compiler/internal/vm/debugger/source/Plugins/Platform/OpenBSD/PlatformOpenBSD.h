//===-- PlatformOpenBSD.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_OPENBSD_PLATFORMOPENBSD_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_OPENBSD_PLATFORMOPENBSD_H

#include "Plugins/Platform/POSIX/PlatformPOSIX.h"

namespace lldb_private {
namespace platform_openbsd {

class PlatformOpenBSD : public PlatformPOSIX {
public:
  PlatformOpenBSD(bool is_host);

  static void Initialize();

  static void Terminate();

  // lldb_private::PluginInterface functions
  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static llvm::StringRef GetPluginNameStatic(bool is_host) {
    return is_host ? Platform::GetHostPlatformName() : "remote-openbsd";
  }

  static llvm::StringRef GetPluginDescriptionStatic(bool is_host);

  llvm::StringRef GetPluginName() override {
    return GetPluginNameStatic(IsHost());
  }

  // lldb_private::Platform functions
  llvm::StringRef GetDescription() override {
    return GetPluginDescriptionStatic(IsHost());
  }

  void GetStatus(Stream &strm) override;

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override;

  bool CanDebugProcess() override;

  void CalculateTrapHandlerSymbolNames() override;

  MmapArgList GetMmapArgumentList(const ArchSpec &arch, lldb::addr_t addr,
                                  lldb::addr_t length, unsigned prot,
                                  unsigned flags, lldb::addr_t fd,
                                  lldb::addr_t offset) override;

  std::vector<ArchSpec> m_supported_architectures;
};

} // namespace platform_openbsd
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_OPENBSD_PLATFORMOPENBSD_H
