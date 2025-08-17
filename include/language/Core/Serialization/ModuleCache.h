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

#ifndef LANGUAGE_CORE_SERIALIZATION_MODULECACHE_H
#define LANGUAGE_CORE_SERIALIZATION_MODULECACHE_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"

#include <ctime>

namespace toolchain {
class AdvisoryLock;
} // namespace toolchain

namespace language::Core {
class InMemoryModuleCache;

/// The module cache used for compiling modules implicitly. This centralizes the
/// operations the compiler might want to perform on the cache.
class ModuleCache : public RefCountedBase<ModuleCache> {
public:
  /// May perform any work that only needs to be performed once for multiple
  /// calls \c getLock() with the same module filename.
  virtual void prepareForGetLock(StringRef ModuleFilename) = 0;

  /// Returns lock for the given module file. The lock is initially unlocked.
  virtual std::unique_ptr<toolchain::AdvisoryLock>
  getLock(StringRef ModuleFilename) = 0;

  // TODO: Abstract away timestamps with isUpToDate() and markUpToDate().
  // TODO: Consider exposing a "validation lock" API to prevent multiple clients
  // concurrently noticing an out-of-date module file and validating its inputs.

  /// Returns the timestamp denoting the last time inputs of the module file
  /// were validated.
  virtual std::time_t getModuleTimestamp(StringRef ModuleFilename) = 0;

  /// Updates the timestamp denoting the last time inputs of the module file
  /// were validated.
  virtual void updateModuleTimestamp(StringRef ModuleFilename) = 0;

  /// Returns this process's view of the module cache.
  virtual InMemoryModuleCache &getInMemoryModuleCache() = 0;
  virtual const InMemoryModuleCache &getInMemoryModuleCache() const = 0;

  // TODO: Virtualize writing/reading PCM files, pruning, etc.

  virtual ~ModuleCache() = default;
};

/// Creates new \c ModuleCache backed by a file system directory that may be
/// operated on by multiple processes. This instance must be used across all
/// \c CompilerInstance instances participating in building modules for single
/// translation unit in order to share the same \c InMemoryModuleCache.
IntrusiveRefCntPtr<ModuleCache> createCrossProcessModuleCache();
} // namespace language::Core

#endif
