//===----------- DeviceOffload.h - Device Offloading ------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements classes required for offloading to CUDA devices.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_INTERPRETER_DEVICE_OFFLOAD_H
#define LANGUAGE_CORE_LIB_INTERPRETER_DEVICE_OFFLOAD_H

#include "IncrementalParser.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VirtualFileSystem.h"

namespace language::Core {
struct PartialTranslationUnit;
class CompilerInstance;
class CodeGenOptions;
class TargetOptions;

class IncrementalCUDADeviceParser : public IncrementalParser {
  const std::list<PartialTranslationUnit> &PTUs;

public:
  IncrementalCUDADeviceParser(
      CompilerInstance &DeviceInstance, CompilerInstance &HostInstance,
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::InMemoryFileSystem> VFS,
      toolchain::Error &Err, const std::list<PartialTranslationUnit> &PTUs);

  // Generate PTX for the last PTU.
  toolchain::Expected<toolchain::StringRef> GeneratePTX();

  // Generate fatbinary contents in memory
  toolchain::Error GenerateFatbinary();

  ~IncrementalCUDADeviceParser();

protected:
  int SMVersion;
  toolchain::SmallString<1024> PTXCode;
  toolchain::SmallVector<char, 1024> FatbinContent;
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::InMemoryFileSystem> VFS;
  CodeGenOptions &CodeGenOpts; // Intentionally a reference.
  const TargetOptions &TargetOpts;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_INTERPRETER_DEVICE_OFFLOAD_H
