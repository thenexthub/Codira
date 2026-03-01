//===-- PlatformAppleSimulator.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMAPPLESIMULATOR_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMAPPLESIMULATOR_H

#include "Plugins/Platform/MacOSX/PlatformDarwin.h"
#include "Plugins/Platform/MacOSX/objcxx/PlatformiOSSimulatorCoreSimulatorSupport.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/ProcessInfo.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/XcodeSDK.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/TargetParser/Triple.h"

#include <mutex>
#include <optional>
#include <vector>

namespace lldb_private {
class ArchSpec;
class Args;
class Debugger;
class FileSpecList;
class ModuleSpec;
class Process;
class ProcessLaunchInfo;
class Stream;
class Target;
class UUID;

class PlatformAppleSimulator : public PlatformDarwin {
public:
  // Class Functions
  static void Initialize();

  static void Terminate();

  // Class Methods
  PlatformAppleSimulator(
      const char *class_name, const char *description, ConstString plugin_name,
      llvm::Triple::OSType preferred_os,
      llvm::SmallVector<llvm::StringRef, 4> supported_triples,
      std::string sdk_name_primary, std::string sdk_name_secondary,
      XcodeSDK::Type sdk_type,
      CoreSimulatorSupport::DeviceType::ProductFamilyID kind);

  static lldb::PlatformSP
  CreateInstance(const char *class_name, const char *description,
                 ConstString plugin_name,
                 llvm::SmallVector<llvm::Triple::ArchType, 4> supported_arch,
                 llvm::Triple::OSType preferred_os,
                 llvm::SmallVector<llvm::Triple::OSType, 4> supported_os,
                 llvm::SmallVector<llvm::StringRef, 4> supported_triples,
                 std::string sdk_name_primary, std::string sdk_name_secondary,
                 XcodeSDK::Type sdk_type,
                 CoreSimulatorSupport::DeviceType::ProductFamilyID kind,
                 bool force, const ArchSpec *arch);

  ~PlatformAppleSimulator() override;

  llvm::StringRef GetPluginName() override {
    return m_plugin_name.GetStringRef();
  }
  llvm::StringRef GetDescription() override { return m_description; }

  Status LaunchProcess(ProcessLaunchInfo &launch_info) override;

  void GetStatus(Stream &strm) override;

  Status ConnectRemote(Args &args) override;

  Status DisconnectRemote() override;

  lldb::ProcessSP DebugProcess(ProcessLaunchInfo &launch_info,
                               Debugger &debugger, Target &target,
                               Status &error) override;

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override;

  Status GetSharedModule(const ModuleSpec &module_spec, Process *process,
                         lldb::ModuleSP &module_sp,
                         llvm::SmallVectorImpl<lldb::ModuleSP> *old_modules,
                         bool *did_create_ptr) override;

  uint32_t FindProcesses(const ProcessInstanceInfoMatch &match_info,
                         ProcessInstanceInfoList &process_infos) override;

  void
  AddClangModuleCompilationOptions(Target *target,
                                   std::vector<std::string> &options) override {
    return PlatformDarwin::AddClangModuleCompilationOptionsForSDKType(
        target, options, m_sdk_type);
  }

protected:
  const char *m_class_name;
  const char *m_description;
  ConstString m_plugin_name;
  std::mutex m_core_sim_path_mutex;
  std::optional<FileSpec> m_core_simulator_framework_path;
  std::optional<CoreSimulatorSupport::Device> m_device;
  CoreSimulatorSupport::DeviceType::ProductFamilyID m_kind;

  FileSpec GetCoreSimulatorPath();

  llvm::StringRef GetSDKFilepath();

  llvm::Triple::OSType m_os_type = llvm::Triple::UnknownOS;
  llvm::SmallVector<llvm::StringRef, 4> m_supported_triples = {};
  std::string m_sdk_name_primary;
  std::string m_sdk_name_secondary;
  bool m_have_searched_for_sdk = false;
  llvm::StringRef m_sdk;
  XcodeSDK::Type m_sdk_type;

  void LoadCoreSimulator();

#if defined(__APPLE__)
  CoreSimulatorSupport::Device GetSimulatorDevice();
#endif

private:
  PlatformAppleSimulator(const PlatformAppleSimulator &) = delete;
  const PlatformAppleSimulator &
  operator=(const PlatformAppleSimulator &) = delete;
  Status

  GetSymbolFile(const FileSpec &platform_file, const UUID *uuid_ptr,
                FileSpec &local_file);
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMAPPLESIMULATOR_H
