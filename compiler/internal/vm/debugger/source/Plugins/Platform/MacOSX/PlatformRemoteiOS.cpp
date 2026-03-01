//===-- PlatformRemoteiOS.cpp ---------------------------------------------===//
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

#include "PlatformRemoteiOS.h"

#include "lldb/Breakpoint/BreakpointLocation.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/ModuleList.h"
#include "lldb/Core/ModuleSpec.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Host.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/StreamString.h"

using namespace lldb;
using namespace lldb_private;

LLDB_PLUGIN_DEFINE(PlatformRemoteiOS)

// Static Variables
static uint32_t g_initialize_count = 0;

// Static Functions
void PlatformRemoteiOS::Initialize() {
  PlatformDarwin::Initialize();

  if (g_initialize_count++ == 0) {
    PluginManager::RegisterPlugin(PlatformRemoteiOS::GetPluginNameStatic(),
                                  PlatformRemoteiOS::GetDescriptionStatic(),
                                  PlatformRemoteiOS::CreateInstance);
  }
}

void PlatformRemoteiOS::Terminate() {
  if (g_initialize_count > 0) {
    if (--g_initialize_count == 0) {
      PluginManager::UnregisterPlugin(PlatformRemoteiOS::CreateInstance);
    }
  }

  PlatformDarwin::Terminate();
}

PlatformSP PlatformRemoteiOS::CreateInstance(bool force, const ArchSpec *arch) {
  Log *log = GetLog(LLDBLog::Platform);
  if (log) {
    const char *arch_name;
    if (arch && arch->GetArchitectureName())
      arch_name = arch->GetArchitectureName();
    else
      arch_name = "<null>";

    const char *triple_cstr =
        arch ? arch->GetTriple().getTriple().c_str() : "<null>";

    LLDB_LOGF(log, "PlatformRemoteiOS::%s(force=%s, arch={%s,%s})",
              __FUNCTION__, force ? "true" : "false", arch_name, triple_cstr);
  }

  bool create = force;
  if (!create && arch && arch->IsValid()) {
    switch (arch->GetMachine()) {
    case llvm::Triple::arm:
    case llvm::Triple::aarch64:
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
        case llvm::Triple::Darwin: // Deprecated, but still support Darwin for
                                   // historical reasons
        case llvm::Triple::IOS:    // This is the right triple value for iOS
                                   // debugging
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

  if (create) {
    if (log)
      LLDB_LOGF(log, "PlatformRemoteiOS::%s() creating platform", __FUNCTION__);

    return lldb::PlatformSP(new PlatformRemoteiOS());
  }

  if (log)
    LLDB_LOGF(log, "PlatformRemoteiOS::%s() aborting creation of platform",
              __FUNCTION__);

  return lldb::PlatformSP();
}

llvm::StringRef PlatformRemoteiOS::GetDescriptionStatic() {
  return "Remote iOS platform plug-in.";
}

/// Default Constructor
PlatformRemoteiOS::PlatformRemoteiOS()
    : PlatformRemoteDarwinDevice() {}

std::vector<ArchSpec> PlatformRemoteiOS::GetSupportedArchitectures(
    const ArchSpec &process_host_arch) {
  std::vector<ArchSpec> result;
  ARMGetSupportedArchitectures(result, llvm::Triple::IOS);
  return result;
}

bool PlatformRemoteiOS::CheckLocalSharedCache() const {
  // You can run iPhone and iPad apps on Mac with Apple Silicon. At the
  // platform level there's no way to distinguish them from remote iOS
  // applications. Make sure we still read from our own shared cache.
  return true;
}

llvm::StringRef PlatformRemoteiOS::GetDeviceSupportDirectoryName() {
  return "iOS DeviceSupport";
}

llvm::StringRef PlatformRemoteiOS::GetPlatformName() {
  return "iPhoneOS.platform";
}
