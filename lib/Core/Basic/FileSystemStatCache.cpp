//===- FileSystemStatCache.cpp - Caching for 'stat' calls -----------------===//
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
//  This file defines the FileSystemStatCache interface.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/FileSystemStatCache.h"
#include "toolchain/Support/ErrorOr.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <utility>

using namespace language::Core;

void FileSystemStatCache::anchor() {}

/// FileSystemStatCache::get - Get the 'stat' information for the specified
/// path, using the cache to accelerate it if possible.  This returns true if
/// the path does not exist or false if it exists.
///
/// If isFile is true, then this lookup should only return success for files
/// (not directories).  If it is false this lookup should only return
/// success for directories (not files).  On a successful file lookup, the
/// implementation can optionally fill in FileDescriptor with a valid
/// descriptor and the client guarantees that it will close it.
std::error_code FileSystemStatCache::get(StringRef Path,
                                         toolchain::vfs::Status &Status, bool isFile,
                                         std::unique_ptr<toolchain::vfs::File> *F,
                                         FileSystemStatCache *Cache,
                                         toolchain::vfs::FileSystem &FS,
                                         bool IsText) {
  bool isForDir = !isFile;
  std::error_code RetCode;

  // If we have a cache, use it to resolve the stat query.
  if (Cache)
    RetCode = Cache->getStat(Path, Status, isFile, F, FS);
  else if (isForDir || !F) {
    // If this is a directory or a file descriptor is not needed and we have
    // no cache, just go to the file system.
    toolchain::ErrorOr<toolchain::vfs::Status> StatusOrErr = FS.status(Path);
    if (!StatusOrErr) {
      RetCode = StatusOrErr.getError();
    } else {
      Status = *StatusOrErr;
    }
  } else {
    // Otherwise, we have to go to the filesystem.  We can always just use
    // 'stat' here, but (for files) the client is asking whether the file exists
    // because it wants to turn around and *open* it.  It is more efficient to
    // do "open+fstat" on success than it is to do "stat+open".
    //
    // Because of this, check to see if the file exists with 'open'.  If the
    // open succeeds, use fstat to get the stat info.
    auto OwnedFile =
        IsText ? FS.openFileForRead(Path) : FS.openFileForReadBinary(Path);

    if (!OwnedFile) {
      // If the open fails, our "stat" fails.
      RetCode = OwnedFile.getError();
    } else {
      // Otherwise, the open succeeded.  Do an fstat to get the information
      // about the file.  We'll end up returning the open file descriptor to the
      // client to do what they please with it.
      toolchain::ErrorOr<toolchain::vfs::Status> StatusOrErr = (*OwnedFile)->status();
      if (StatusOrErr) {
        Status = *StatusOrErr;
        *F = std::move(*OwnedFile);
      } else {
        // fstat rarely fails.  If it does, claim the initial open didn't
        // succeed.
        *F = nullptr;
        RetCode = StatusOrErr.getError();
      }
    }
  }

  // If the path doesn't exist, return failure.
  if (RetCode)
    return RetCode;

  // If the path exists, make sure that its "directoryness" matches the clients
  // demands.
  if (Status.isDirectory() != isForDir) {
    // If not, close the file if opened.
    if (F)
      *F = nullptr;
    return std::make_error_code(
        Status.isDirectory() ?
            std::errc::is_a_directory : std::errc::not_a_directory);
  }

  return std::error_code();
}

std::error_code
MemorizeStatCalls::getStat(StringRef Path, toolchain::vfs::Status &Status,
                           bool isFile,
                           std::unique_ptr<toolchain::vfs::File> *F,
                           toolchain::vfs::FileSystem &FS) {
  auto err = get(Path, Status, isFile, F, nullptr, FS);
  if (err) {
    // Do not cache failed stats, it is easy to construct common inconsistent
    // situations if we do, and they are not important for PCH performance
    // (which currently only needs the stats to construct the initial
    // FileManager entries).
    return err;
  }

  // Cache file 'stat' results and directories with absolutely paths.
  if (!Status.isDirectory() || toolchain::sys::path::is_absolute(Path))
    StatCalls[Path] = Status;

  return std::error_code();
}
