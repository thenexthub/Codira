//===- toolchain/TargetParser/Unix/Host.inc --------------------------*- C++ -*-===//
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
//
// This file implements the UNIX Host support.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on *all* UNIX variants.
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/StringRef.h"
#include "vm/core/Config/config.h"
#include <cctype>
#include <string>
#include <sys/utsname.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace vm::core;

static std::string getOSVersion() {
  struct utsname info;

  if (uname(&info))
    return "";

  return info.release;
}

static std::string updateTripleOSVersion(std::string TargetTripleString) {
  // On darwin, we want to update the version to match that of the target.
  std::string::size_type DarwinDashIdx = TargetTripleString.find("-darwin");
  if (DarwinDashIdx != std::string::npos) {
    TargetTripleString.resize(DarwinDashIdx + strlen("-darwin"));
    TargetTripleString += getOSVersion();
    return TargetTripleString;
  }
  std::string::size_type MacOSDashIdx = TargetTripleString.find("-macos");
  if (MacOSDashIdx != std::string::npos) {
    TargetTripleString.resize(MacOSDashIdx);
    // Reset the OS to darwin as the OS version from `uname` doesn't use the
    // macOS version scheme.
    TargetTripleString += "-darwin";
    TargetTripleString += getOSVersion();
  }
  // On AIX, the AIX version and release should be that of the current host
  // unless if the version has already been specified.
  if (Triple(LLVM_HOST_TRIPLE).getOS() == Triple::AIX) {
    Triple TT{TargetTripleString};
    if (TT.getOS() == Triple::AIX && !TT.getOSMajorVersion()) {
      struct utsname name;
      if (uname(&name) != -1) {
        std::string release = name.release;

        if (strcmp(name.sysname, "OS400") == 0) {
          /*
            PASE uses different versioning system than AIX.
            The following table shows the currently supported PASE
            releases and the corresponding AIX release:
            --------------------------
              PASE    |    AIX
            --------------------------
              V7R4    |    7.2 (TL2)
            --------------------------
              V7R5    |    7.2 (TL5)
            --------------------------
              V7R6    |    7.3 (TL1)
            --------------------------
          */
          release = (release == "4" || release == "5") ? "2" : "3";
        }

        std::string NewOSName = std::string(Triple::getOSTypeName(Triple::AIX));
        NewOSName += name.version;
        NewOSName += '.';
        NewOSName += release;
        NewOSName += ".0.0";
        TT.setOSName(NewOSName);
        return TT.str();
      }
    }
  }
  return TargetTripleString;
}

std::string sys::getDefaultTargetTriple() {
  std::string TargetTripleString =
      updateTripleOSVersion(LLVM_DEFAULT_TARGET_TRIPLE);

  // Override the default target with an environment variable named by
  // LLVM_TARGET_TRIPLE_ENV.
#if defined(LLVM_TARGET_TRIPLE_ENV)
  if (const char *EnvTriple = std::getenv(LLVM_TARGET_TRIPLE_ENV))
    TargetTripleString = EnvTriple;
#endif

  return TargetTripleString;
}
