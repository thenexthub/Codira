//===-- PlatformRemoteAppleXR.cpp -----------------------------------------===//
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

#include <string>
#include <vector>

#include "PlatformRemoteAppleXR.h"
#include "lldb/Breakpoint/BreakpointLocation.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/ModuleList.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"

using namespace lldb;
using namespace lldb_private;

// Static Variables
static uint32_t g_xr_initialize_count = 0;

// Static Functions
void PlatformRemoteAppleXR::Initialize() {
  PlatformDarwin::Initialize();

  if (g_xr_initialize_count++ == 0) {
    PluginManager::RegisterPlugin(PlatformRemoteAppleXR::GetPluginNameStatic(),
                                  PlatformRemoteAppleXR::GetDescriptionStatic(),
                                  PlatformRemoteAppleXR::CreateInstance);
  }
}

void PlatformRemoteAppleXR::Terminate() {
  if (g_xr_initialize_count > 0) {
    if (--g_xr_initialize_count == 0) {
      PluginManager::UnregisterPlugin(PlatformRemoteAppleXR::CreateInstance);
    }
  }

  PlatformDarwin::Terminate();
}

PlatformSP PlatformRemoteAppleXR::CreateInstance(bool force,
                                                 const ArchSpec *arch) {
  Log *log = GetLog(LLDBLog::Platform);
  if (log) {
    const char *arch_name;
    if (arch && arch->GetArchitectureName())
      arch_name = arch->GetArchitectureName();
    else
      arch_name = "<null>";

    const char *triple_cstr =
        arch ? arch->GetTriple().getTriple().c_str() : "<null>";

    LLDB_LOGF(log, "PlatformRemoteAppleXR::%s(force=%s, arch={%s,%s})",
              __FUNCTION__, force ? "true" : "false", arch_name, triple_cstr);
  }

  bool create = force;
  if (!create && arch && arch->IsValid()) {
    switch (arch->GetMachine()) {
    case llvm::Triple::arm:
    case llvm::Triple::aarch64:
    case llvm::Triple::aarch64_32:
    case llvm::Triple::thumb: {
      const llvm::Triple &triple = arch->GetTriple();
      llvm::Triple::VendorType vendor = triple.getVendor();
      switch (vendor) {
      case llvm::Triple::Apple:
        create = true;
        break;

#if defined(__APPLE__)
      // Only accept "unknown" for the vendor if the host is Apple and
      // "unknown" wasn't specified (it was just returned because it was NOT
      // specified)
      case llvm::Triple::UnknownVendor:
        create = !arch->TripleVendorWasSpecified();
        break;

#endif
      default:
        break;
      }
      if (create) {
        switch (triple.getOS()) {
        case llvm::Triple::XROS: // This is the right triple value for Apple
                                 // XR debugging
          break;

        default:
          create = false;
          break;
        }
      }
    } break;
    default:
      break;
    }
  }

#if defined(TARGET_OS_XR) && TARGET_OS_XR == 1
  // If lldb is running on a XR device, this isn't a RemoteXR.
  if (force == false) {
    create = false;
  }
#endif

  if (create) {
    LLDB_LOGF(log, "PlatformRemoteAppleXR::%s() creating platform",
              __FUNCTION__);

    return lldb::PlatformSP(new PlatformRemoteAppleXR());
  }

  LLDB_LOGF(log, "PlatformRemoteAppleXR::%s() aborting creation of platform",
            __FUNCTION__);

  return lldb::PlatformSP();
}

llvm::StringRef PlatformRemoteAppleXR::GetPluginNameStatic() {
  return "remote-xros";
}

llvm::StringRef PlatformRemoteAppleXR::GetDescriptionStatic() {
  return "Remote Apple XR platform plug-in.";
}

/// Default Constructor
PlatformRemoteAppleXR::PlatformRemoteAppleXR() : PlatformRemoteDarwinDevice() {}

std::vector<lldb_private::ArchSpec>
PlatformRemoteAppleXR::GetSupportedArchitectures(
    const ArchSpec &process_host_arch) {
  std::vector<ArchSpec> result;
  result.push_back(ArchSpec("arm64-apple-xros"));
  result.push_back(ArchSpec("arm64-apple-xros"));
  return result;
}

llvm::StringRef PlatformRemoteAppleXR::GetDeviceSupportDirectoryName() {
  return "XROS DeviceSupport";
}

llvm::StringRef PlatformRemoteAppleXR::GetPlatformName() {
  return "XROS.platform";
}
