//===-- HostInfoPosix.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_POSIX_HOSTINFOPOSIX_H
#define LLDB_HOST_POSIX_HOSTINFOPOSIX_H

#include "lldb/Host/HostInfoBase.h"
#include "lldb/Utility/FileSpec.h"
#include <optional>
#include <string>

namespace lldb_private {

class UserIDResolver;

class HostInfoPosix : public HostInfoBase {
  friend class HostInfoBase;

public:
  static size_t GetPageSize();
  static bool GetHostname(std::string &s);
  static std::optional<std::string> GetOSKernelDescription();

  static uint32_t GetUserID();
  static uint32_t GetGroupID();
  static uint32_t GetEffectiveUserID();
  static uint32_t GetEffectiveGroupID();

  static FileSpec GetDefaultShell();

  static bool GetEnvironmentVar(const std::string &var_name, std::string &var);

  static UserIDResolver &GetUserIDResolver();
  static llvm::VersionTuple GetOSVersion();
  static std::optional<std::string> GetOSBuildString();

  static llvm::Expected<llvm::StringRef> GetSDKRoot(SDKOptions options);

protected:
  static bool ComputeSupportExeDirectory(FileSpec &file_spec);
  static bool ComputeHeaderDirectory(FileSpec &file_spec);
  static bool ComputeSystemPluginsDirectory(FileSpec &file_spec);
  static bool ComputeUserPluginsDirectory(FileSpec &file_spec);
};
} // namespace lldb_private

#endif
