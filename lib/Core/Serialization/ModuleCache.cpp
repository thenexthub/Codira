//===----------------------------------------------------------------------===//
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

#include "language/Core/Serialization/ModuleCache.h"

#include "language/Core/Serialization/InMemoryModuleCache.h"
#include "language/Core/Serialization/ModuleFile.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/LockFileManager.h"
#include "toolchain/Support/Path.h"

using namespace language::Core;

namespace {
class CrossProcessModuleCache : public ModuleCache {
  InMemoryModuleCache InMemory;

public:
  void prepareForGetLock(StringRef ModuleFilename) override {
    // FIXME: Do this in LockFileManager and only if the directory doesn't
    // exist.
    StringRef Dir = toolchain::sys::path::parent_path(ModuleFilename);
    toolchain::sys::fs::create_directories(Dir);
  }

  std::unique_ptr<toolchain::AdvisoryLock>
  getLock(StringRef ModuleFilename) override {
    return std::make_unique<toolchain::LockFileManager>(ModuleFilename);
  }

  std::time_t getModuleTimestamp(StringRef ModuleFilename) override {
    toolchain::sys::fs::file_status Status;
    if (toolchain::sys::fs::status(ModuleFilename, Status) != std::error_code{})
      return 0;
    return toolchain::sys::toTimeT(Status.getLastModificationTime());
  }

  void updateModuleTimestamp(StringRef ModuleFilename) override {
    // Overwrite the timestamp file contents so that file's mtime changes.
    std::error_code EC;
    toolchain::raw_fd_ostream OS(
        serialization::ModuleFile::getTimestampFilename(ModuleFilename), EC,
        toolchain::sys::fs::OF_TextWithCRLF);
    if (EC)
      return;
    OS << "Timestamp file\n";
    OS.close();
    OS.clear_error(); // Avoid triggering a fatal error.
  }

  InMemoryModuleCache &getInMemoryModuleCache() override { return InMemory; }
  const InMemoryModuleCache &getInMemoryModuleCache() const override {
    return InMemory;
  }
};
} // namespace

IntrusiveRefCntPtr<ModuleCache> language::Core::createCrossProcessModuleCache() {
  return toolchain::makeIntrusiveRefCnt<CrossProcessModuleCache>();
}
