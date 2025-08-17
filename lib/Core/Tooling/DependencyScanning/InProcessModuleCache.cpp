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

#include "language/Core/Tooling/DependencyScanning/InProcessModuleCache.h"

#include "language/Core/Serialization/InMemoryModuleCache.h"
#include "toolchain/Support/AdvisoryLock.h"
#include "toolchain/Support/Chrono.h"

#include <mutex>

using namespace language::Core;
using namespace tooling;
using namespace dependencies;

namespace {
class ReaderWriterLock : public toolchain::AdvisoryLock {
  // TODO: Consider using std::atomic::{wait,notify_all} when we move to C++20.
  std::unique_lock<std::shared_mutex> OwningLock;

public:
  ReaderWriterLock(std::shared_mutex &Mutex)
      : OwningLock(Mutex, std::defer_lock) {}

  Expected<bool> tryLock() override { return OwningLock.try_lock(); }

  toolchain::WaitForUnlockResult
  waitForUnlockFor(std::chrono::seconds MaxSeconds) override {
    assert(!OwningLock);
    // We do not respect the timeout here. It's very generous for implicit
    // modules, so we'd typically only reach it if the owner crashed (but so did
    // we, since we run in the same process), or encountered deadlock.
    (void)MaxSeconds;
    std::shared_lock<std::shared_mutex> Lock(*OwningLock.mutex());
    return toolchain::WaitForUnlockResult::Success;
  }

  std::error_code unsafeMaybeUnlock() override {
    // Unlocking the mutex here would trigger UB and we don't expect this to be
    // actually called when compiling scanning modules due to the no-timeout
    // guarantee above.
    return {};
  }

  ~ReaderWriterLock() override = default;
};

class InProcessModuleCache : public ModuleCache {
  ModuleCacheEntries &Entries;

  // TODO: If we changed the InMemoryModuleCache API and relied on strict
  // context hash, we could probably create more efficient thread-safe
  // implementation of the InMemoryModuleCache such that it doesn't need to be
  // recreated for each translation unit.
  InMemoryModuleCache InMemory;

public:
  InProcessModuleCache(ModuleCacheEntries &Entries) : Entries(Entries) {}

  void prepareForGetLock(StringRef Filename) override {}

  std::unique_ptr<toolchain::AdvisoryLock> getLock(StringRef Filename) override {
    auto &CompilationMutex = [&]() -> std::shared_mutex & {
      std::lock_guard<std::mutex> Lock(Entries.Mutex);
      auto &Entry = Entries.Map[Filename];
      if (!Entry)
        Entry = std::make_unique<ModuleCacheEntry>();
      return Entry->CompilationMutex;
    }();
    return std::make_unique<ReaderWriterLock>(CompilationMutex);
  }

  std::time_t getModuleTimestamp(StringRef Filename) override {
    auto &Timestamp = [&]() -> std::atomic<std::time_t> & {
      std::lock_guard<std::mutex> Lock(Entries.Mutex);
      auto &Entry = Entries.Map[Filename];
      if (!Entry)
        Entry = std::make_unique<ModuleCacheEntry>();
      return Entry->Timestamp;
    }();

    return Timestamp.load();
  }

  void updateModuleTimestamp(StringRef Filename) override {
    // Note: This essentially replaces FS contention with mutex contention.
    auto &Timestamp = [&]() -> std::atomic<std::time_t> & {
      std::lock_guard<std::mutex> Lock(Entries.Mutex);
      auto &Entry = Entries.Map[Filename];
      if (!Entry)
        Entry = std::make_unique<ModuleCacheEntry>();
      return Entry->Timestamp;
    }();

    Timestamp.store(toolchain::sys::toTimeT(std::chrono::system_clock::now()));
  }

  InMemoryModuleCache &getInMemoryModuleCache() override { return InMemory; }
  const InMemoryModuleCache &getInMemoryModuleCache() const override {
    return InMemory;
  }
};
} // namespace

IntrusiveRefCntPtr<ModuleCache>
dependencies::makeInProcessModuleCache(ModuleCacheEntries &Entries) {
  return toolchain::makeIntrusiveRefCnt<InProcessModuleCache>(Entries);
}
