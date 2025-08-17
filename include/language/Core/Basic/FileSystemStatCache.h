//===- FileSystemStatCache.h - Caching for 'stat' calls ---------*- C++ -*-===//
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
/// \file
/// Defines the FileSystemStatCache interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_FILESYSTEMSTATCACHE_H
#define LANGUAGE_CORE_BASIC_FILESYSTEMSTATCACHE_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <cstdint>
#include <ctime>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace language::Core {

/// Abstract interface for introducing a FileManager cache for 'stat'
/// system calls, which is used by precompiled and pretokenized headers to
/// improve performance.
class FileSystemStatCache {
  virtual void anchor();

public:
  virtual ~FileSystemStatCache() = default;

  /// Get the 'stat' information for the specified path, using the cache
  /// to accelerate it if possible.
  ///
  /// \returns \c true if the path does not exist or \c false if it exists.
  ///
  /// If isFile is true, then this lookup should only return success for files
  /// (not directories).  If it is false this lookup should only return
  /// success for directories (not files).  On a successful file lookup, the
  /// implementation can optionally fill in \p F with a valid \p File object and
  /// the client guarantees that it will close it.
  static std::error_code get(StringRef Path, toolchain::vfs::Status &Status,
                             bool isFile, std::unique_ptr<toolchain::vfs::File> *F,
                             FileSystemStatCache *Cache,
                             toolchain::vfs::FileSystem &FS, bool IsText = true);

protected:
  // FIXME: The pointer here is a non-owning/optional reference to the
  // unique_ptr. std::optional<unique_ptr<vfs::File>&> might be nicer, but
  // Optional needs some work to support references so this isn't possible yet.
  virtual std::error_code getStat(StringRef Path, toolchain::vfs::Status &Status,
                                  bool isFile,
                                  std::unique_ptr<toolchain::vfs::File> *F,
                                  toolchain::vfs::FileSystem &FS) = 0;
};

/// A stat "cache" that can be used by FileManager to keep
/// track of the results of stat() calls that occur throughout the
/// execution of the front end.
class MemorizeStatCalls : public FileSystemStatCache {
public:
  /// The set of stat() calls that have been seen.
  toolchain::StringMap<toolchain::vfs::Status, toolchain::BumpPtrAllocator> StatCalls;

  using iterator =
      toolchain::StringMap<toolchain::vfs::Status,
                      toolchain::BumpPtrAllocator>::const_iterator;

  iterator begin() const { return StatCalls.begin(); }
  iterator end() const { return StatCalls.end(); }

  std::error_code getStat(StringRef Path, toolchain::vfs::Status &Status,
                          bool isFile,
                          std::unique_ptr<toolchain::vfs::File> *F,
                          toolchain::vfs::FileSystem &FS) override;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_FILESYSTEMSTATCACHE_H
