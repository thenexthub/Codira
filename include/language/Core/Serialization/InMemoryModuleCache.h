//===- InMemoryModuleCache.h - In-memory cache for modules ------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_SERIALIZATION_INMEMORYMODULECACHE_H
#define LANGUAGE_CORE_SERIALIZATION_INMEMORYMODULECACHE_H

#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/MemoryBuffer.h"
#include <memory>

namespace language::Core {

/// In-memory cache for modules.
///
/// This is a cache for modules for use across a compilation, sharing state
/// between the CompilerInstances in an implicit modules build.  It must be
/// shared by each CompilerInstance, ASTReader, ASTWriter, and ModuleManager
/// that are coordinating.
///
/// Critically, it ensures that a single process has a consistent view of each
/// PCM.  This is used by \a CompilerInstance when building PCMs to ensure that
/// each \a ModuleManager sees the same files.
class InMemoryModuleCache : public toolchain::RefCountedBase<InMemoryModuleCache> {
  struct PCM {
    std::unique_ptr<toolchain::MemoryBuffer> Buffer;

    /// Track whether this PCM is known to be good (either built or
    /// successfully imported by a CompilerInstance/ASTReader using this
    /// cache).
    bool IsFinal = false;

    PCM() = default;
    PCM(std::unique_ptr<toolchain::MemoryBuffer> Buffer)
        : Buffer(std::move(Buffer)) {}
  };

  /// Cache of buffers.
  toolchain::StringMap<PCM> PCMs;

public:
  /// There are four states for a PCM.  It must monotonically increase.
  ///
  ///  1. Unknown: the PCM has neither been read from disk nor built.
  ///  2. Tentative: the PCM has been read from disk but not yet imported or
  ///     built.  It might work.
  ///  3. ToBuild: the PCM read from disk did not work but a new one has not
  ///     been built yet.
  ///  4. Final: indicating that the current PCM was either built in this
  ///     process or has been successfully imported.
  enum State { Unknown, Tentative, ToBuild, Final };

  /// Get the state of the PCM.
  State getPCMState(toolchain::StringRef Filename) const;

  /// Store the PCM under the Filename.
  ///
  /// \pre state is Unknown
  /// \post state is Tentative
  /// \return a reference to the buffer as a convenience.
  toolchain::MemoryBuffer &addPCM(toolchain::StringRef Filename,
                             std::unique_ptr<toolchain::MemoryBuffer> Buffer);

  /// Store a just-built PCM under the Filename.
  ///
  /// \pre state is Unknown or ToBuild.
  /// \pre state is not Tentative.
  /// \return a reference to the buffer as a convenience.
  toolchain::MemoryBuffer &addBuiltPCM(toolchain::StringRef Filename,
                                  std::unique_ptr<toolchain::MemoryBuffer> Buffer);

  /// Try to remove a buffer from the cache.  No effect if state is Final.
  ///
  /// \pre state is Tentative/Final.
  /// \post Tentative => ToBuild or Final => Final.
  /// \return false on success, i.e. if Tentative => ToBuild.
  bool tryToDropPCM(toolchain::StringRef Filename);

  /// Mark a PCM as final.
  ///
  /// \pre state is Tentative or Final.
  /// \post state is Final.
  void finalizePCM(toolchain::StringRef Filename);

  /// Get a pointer to the pCM if it exists; else nullptr.
  toolchain::MemoryBuffer *lookupPCM(toolchain::StringRef Filename) const;

  /// Check whether the PCM is final and has been shown to work.
  ///
  /// \return true iff state is Final.
  bool isPCMFinal(toolchain::StringRef Filename) const;

  /// Check whether the PCM is waiting to be built.
  ///
  /// \return true iff state is ToBuild.
  bool shouldBuildPCM(toolchain::StringRef Filename) const;
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_SERIALIZATION_INMEMORYMODULECACHE_H
