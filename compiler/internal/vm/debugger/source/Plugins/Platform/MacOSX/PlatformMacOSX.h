//===-- PlatformMacOSX.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMMACOSX_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMMACOSX_H

#include "PlatformDarwinDevice.h"
#include "lldb/Target/Platform.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/XcodeSDK.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

#include <vector>

namespace lldb_private {
class ArchSpec;
class FileSpec;
class FileSpecList;
class ModuleSpec;
class Process;
class Target;

class PlatformMacOSX : public PlatformDarwinDevice {
public:
  PlatformMacOSX();

  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() {
    return Platform::GetHostPlatformName();
  }

  static llvm::StringRef GetDescriptionStatic();

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  Status GetSharedModule(const ModuleSpec &module_spec, Process *process,
                         lldb::ModuleSP &module_sp,
                         llvm::SmallVectorImpl<lldb::ModuleSP> *old_modules,
                         bool *did_create_ptr) override;

  llvm::StringRef GetDescription() override { return GetDescriptionStatic(); }

  Status GetFile(const FileSpec &source, const FileSpec &destination) override {
    return PlatformDarwin::GetFile(source, destination);
  }

  std::vector<ArchSpec>
  GetSupportedArchitectures(const ArchSpec &process_host_arch) override;

  ConstString GetSDKDirectory(Target &target) override;

  void
  AddClangModuleCompilationOptions(Target *target,
                                   std::vector<std::string> &options) override {
    return PlatformDarwin::AddClangModuleCompilationOptionsForSDKType(
        target, options, XcodeSDK::Type::MacOSX);
  }

protected:
  llvm::StringRef GetDeviceSupportDirectoryName() override;
  llvm::StringRef GetPlatformName() override;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_MACOSX_PLATFORMMACOSX_H
