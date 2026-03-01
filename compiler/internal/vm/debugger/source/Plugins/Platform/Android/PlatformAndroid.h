//===-- PlatformAndroid.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_ANDROID_PLATFORMANDROID_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_ANDROID_PLATFORMANDROID_H

#include <memory>
#include <string>

#include "Plugins/Platform/Linux/PlatformLinux.h"

#include "AdbClient.h"

namespace lldb_private {
namespace platform_android {

class PlatformAndroid : public platform_linux::PlatformLinux {
public:
  PlatformAndroid(bool is_host);

  static void Initialize();

  static void Terminate();

  // lldb_private::PluginInterface functions
  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static void DebuggerInitialize(lldb_private::Debugger &debugger);

  static llvm::StringRef GetPluginNameStatic(bool is_host) {
    return is_host ? Platform::GetHostPlatformName() : "remote-android";
  }

  static llvm::StringRef GetPluginDescriptionStatic(bool is_host);

  llvm::StringRef GetPluginName() override {
    return GetPluginNameStatic(IsHost());
  }

  // lldb_private::Platform functions

  Status ConnectRemote(Args &args) override;

  Status GetFile(const FileSpec &source, const FileSpec &destination) override;

  Status PutFile(const FileSpec &source, const FileSpec &destination,
                 uint32_t uid = UINT32_MAX, uint32_t gid = UINT32_MAX) override;

  uint32_t GetSdkVersion();

  bool GetRemoteOSVersion() override;

  Status DisconnectRemote() override;

  uint32_t GetDefaultMemoryCacheLineSize() override;

  uint32_t FindProcesses(const ProcessInstanceInfoMatch &match_info,
                         ProcessInstanceInfoList &process_infos) override;

protected:
  const char *GetCacheHostname() override;

  Status DownloadModuleSlice(const FileSpec &src_file_spec,
                             const uint64_t src_offset, const uint64_t src_size,
                             const FileSpec &dst_file_spec) override;

  Status DownloadSymbolFile(const lldb::ModuleSP &module_sp,
                            const FileSpec &dst_file_spec) override;

  llvm::StringRef
  GetLibdlFunctionDeclarations(lldb_private::Process *process) override;

  typedef std::unique_ptr<AdbClient> AdbClientUP;
  virtual AdbClientUP GetAdbClient(Status &error);

  std::string GetRunAs();

public:
  virtual llvm::StringRef GetPropertyPackageName();

protected:
  virtual std::unique_ptr<AdbSyncService> GetSyncService(Status &error);

  std::string m_device_id;
  uint32_t m_sdk_version;
};

} // namespace platform_android
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_ANDROID_PLATFORMANDROID_H
