//===-- HostInfoMacOSX.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_MACOSX_HOSTINFOMACOSX_H
#define LLDB_HOST_MACOSX_HOSTINFOMACOSX_H

#include "lldb/Host/posix/HostInfoPosix.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/XcodeSDK.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/VersionTuple.h"
#include <optional>

namespace lldb_private {

class ArchSpec;

class HostInfoMacOSX : public HostInfoPosix {
  friend class HostInfoBase;

public:
  static llvm::VersionTuple GetOSVersion();
  static llvm::VersionTuple GetMacCatalystVersion();
  static std::optional<std::string> GetOSBuildString();
  static FileSpec GetProgramFileSpec();
  static FileSpec GetXcodeContentsDirectory();
  static FileSpec GetXcodeDeveloperDirectory();
  static FileSpec GetCurrentXcodeToolchainDirectory();
  static FileSpec GetCurrentCommandLineToolsDirectory();

  /// Query xcrun to find an Xcode SDK directory.
  ///
  /// Note, this is an expensive operation if the SDK we're querying
  /// does not exist in an Xcode installation path on the host.
  static llvm::Expected<llvm::StringRef> GetSDKRoot(SDKOptions options);
  static llvm::Expected<llvm::StringRef> FindSDKTool(XcodeSDK sdk,
                                                     llvm::StringRef tool);

  /// Shared cache utilities
  static SharedCacheImageInfo
  GetSharedCacheImageInfo(llvm::StringRef image_name);

protected:
  static bool ComputeSupportExeDirectory(FileSpec &file_spec);
  static void ComputeHostArchitectureSupport(ArchSpec &arch_32,
                                             ArchSpec &arch_64);
  static bool ComputeHeaderDirectory(FileSpec &file_spec);
  static bool ComputeSystemPluginsDirectory(FileSpec &file_spec);
  static bool ComputeUserPluginsDirectory(FileSpec &file_spec);

  static std::string FindComponentInPath(llvm::StringRef path,
                                         llvm::StringRef component);
};

} // namespace lldb_private

#endif
