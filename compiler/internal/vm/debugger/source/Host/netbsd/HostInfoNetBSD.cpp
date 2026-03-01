//===-- HostInfoNetBSD.cpp ------------------------------------------------===//
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

#include "lldb/Host/netbsd/HostInfoNetBSD.h"

#include <cinttypes>
#include <climits>
#include <cstdio>
#include <cstring>
#include <optional>
#include <pthread.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

using namespace lldb_private;

llvm::VersionTuple HostInfoNetBSD::GetOSVersion() {
  struct utsname un;

  ::memset(&un, 0, sizeof(un));
  if (::uname(&un) < 0)
    return llvm::VersionTuple();

  /* Accept versions like 7.99.21 and 6.1_STABLE */
  uint32_t major, minor, update;
  int status = ::sscanf(un.release, "%" PRIu32 ".%" PRIu32 ".%" PRIu32, &major,
                        &minor, &update);
  switch (status) {
  case 1:
    return llvm::VersionTuple(major);
  case 2:
    return llvm::VersionTuple(major, minor);
  case 3:
    return llvm::VersionTuple(major, minor, update);
  }
  return llvm::VersionTuple();
}

std::optional<std::string> HostInfoNetBSD::GetOSBuildString() {
  int mib[2] = {CTL_KERN, KERN_OSREV};
  int osrev = 0;
  size_t osrev_len = sizeof(osrev);

  if (::sysctl(mib, 2, &osrev, &osrev_len, NULL, 0) == 0)
    return llvm::formatv("{0,10:10}", osrev).str();

  return std::nullopt;
}

FileSpec HostInfoNetBSD::GetProgramFileSpec() {
  static FileSpec g_program_filespec;

  if (!g_program_filespec) {
    static const int name[] = {
        CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME,
    };
    char path[MAXPATHLEN];
    size_t len;

    len = sizeof(path);
    if (sysctl(name, __arraycount(name), path, &len, NULL, 0) != -1) {
      g_program_filespec.SetFile(path, FileSpec::Style::native);
    }
  }
  return g_program_filespec;
}
