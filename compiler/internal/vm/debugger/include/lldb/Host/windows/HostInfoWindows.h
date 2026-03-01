//===-- HostInfoWindows.h ---------------------------------------*- C++ -*-===//
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

#ifndef lldb_Host_windows_HostInfoWindows_h_
#define lldb_Host_windows_HostInfoWindows_h_

#include "lldb/Host/HostInfoBase.h"
#include "lldb/Utility/FileSpec.h"
#include "llvm/Support/VersionTuple.h"
#include <optional>

namespace lldb_private {
class UserIDResolver;

class HostInfoWindows : public HostInfoBase {
  friend class HostInfoBase;

public:
  static void Initialize(SharedLibraryDirectoryHelper *helper = nullptr);
  static void Terminate();

  static size_t GetPageSize();
  static UserIDResolver &GetUserIDResolver();

  static llvm::VersionTuple GetOSVersion();
  static std::optional<std::string> GetOSBuildString();
  static std::optional<std::string> GetOSKernelDescription();
  static bool GetHostname(std::string &s);
  static FileSpec GetProgramFileSpec();
  static FileSpec GetDefaultShell();

  static bool GetEnvironmentVar(const std::string &var_name, std::string &var);

private:
  static FileSpec m_program_filespec;
};
}

#endif
