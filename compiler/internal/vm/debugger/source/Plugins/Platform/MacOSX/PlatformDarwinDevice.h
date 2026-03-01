//===-- PlatformDarwinDevice.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMDARWINDEVICE_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMDARWINDEVICE_H

#include "PlatformDarwin.h"

#include "llvm/ADT/StringRef.h"

#include <string>

namespace lldb_private {

/// Abstract Darwin platform with a potential device support directory.
class PlatformDarwinDevice : public PlatformDarwin {
public:
  using PlatformDarwin::PlatformDarwin;
  ~PlatformDarwinDevice() override;

protected:
  virtual Status GetSharedModuleWithLocalCache(
      const ModuleSpec &module_spec, lldb::ModuleSP &module_sp,
      llvm::SmallVectorImpl<lldb::ModuleSP> *old_modules, bool *did_create_ptr);

  struct SDKDirectoryInfo {
    SDKDirectoryInfo(const FileSpec &sdk_dir_spec);
    FileSpec directory;
    ConstString build;
    llvm::VersionTuple version;
    bool user_cached;
  };

  typedef std::vector<SDKDirectoryInfo> SDKDirectoryInfoCollection;

  bool UpdateSDKDirectoryInfosIfNeeded();

  const SDKDirectoryInfo *GetSDKDirectoryForLatestOSVersion();
  const SDKDirectoryInfo *GetSDKDirectoryForCurrentOSVersion();

  static FileSystem::EnumerateDirectoryResult
  GetContainedFilesIntoVectorOfStringsCallback(void *baton,
                                               llvm::sys::fs::file_type ft,
                                               llvm::StringRef path);

  const char *GetDeviceSupportDirectory();
  const char *GetDeviceSupportDirectoryForOSVersion();

  virtual llvm::StringRef GetPlatformName() = 0;
  virtual llvm::StringRef GetDeviceSupportDirectoryName() = 0;

  std::mutex m_sdk_dir_mutex;
  SDKDirectoryInfoCollection m_sdk_directory_infos;

private:
  std::string m_device_support_directory;
  std::string m_device_support_directory_for_os_version;
};
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMDARWINDEVICE_H
