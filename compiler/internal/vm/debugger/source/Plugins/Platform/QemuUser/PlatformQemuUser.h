//===-- PlatformQemuUser.h ------------------------------------------------===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_QEMUUSER_PLATFORMQEMUUSER_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_QEMUUSER_PLATFORMQEMUUSER_H

#include "lldb/Host/Host.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/Platform.h"

namespace lldb_private {

class PlatformQemuUser : public Platform {
public:
  static void Initialize();
  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "qemu-user"; }
  static llvm::StringRef GetPluginDescriptionStatic();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
  llvm::StringRef GetDescription() override {
    return GetPluginDescriptionStatic();
  }

  UserIDResolver &GetUserIDResolver() override {
    return HostInfo::GetUserIDResolver();
  }

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override;

  lldb::ProcessSP DebugProcess(ProcessLaunchInfo &launch_info,
                               Debugger &debugger, Target &target,
                               Status &error) override;

  lldb::ProcessSP Attach(ProcessAttachInfo &attach_info, Debugger &debugger,
                         Target *target, Status &status) override {
    status = Status::FromErrorString("Not supported");
    return nullptr;
  }

  uint32_t FindProcesses(const ProcessInstanceInfoMatch &match_info,
                         ProcessInstanceInfoList &proc_infos) override {
    return 0;
  }

  bool GetProcessInfo(lldb::pid_t pid,
                      ProcessInstanceInfo &proc_info) override {
    return false;
  }

  bool IsConnected() const override { return true; }

  void CalculateTrapHandlerSymbolNames() override {}

  Environment GetEnvironment() override;

  MmapArgList GetMmapArgumentList(const ArchSpec &arch, lldb::addr_t addr,
                                  lldb::addr_t length, unsigned prot,
                                  unsigned flags, lldb::addr_t fd,
                                  lldb::addr_t offset) override {
    return Platform::GetHostPlatform()->GetMmapArgumentList(
        arch, addr, length, prot, flags, fd, offset);
  }

private:
  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);
  static void DebuggerInitialize(Debugger &debugger);

  PlatformQemuUser() : Platform(/*is_host=*/true) {}
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_QEMUUSER_PLATFORMQEMUUSER_H
