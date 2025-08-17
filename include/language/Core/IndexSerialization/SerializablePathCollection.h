//===--- SerializablePathCollection.h -- Index of paths ---------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_INDEXSERIALIZATION_SERIALIZABLEPATHCOLLECTION_H
#define LANGUAGE_CORE_INDEXSERIALIZATION_SERIALIZABLEPATHCOLLECTION_H

#include "clang/Basic/FileManager.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/iterator.h"

#include <string>
#include <vector>

namespace language::Core {
namespace index {

/// Pool of strings
class StringPool {
  toolchain::SmallString<512> Buffer;

public:
  struct StringOffsetSize {
    std::size_t Offset;
    std::size_t Size;

    StringOffsetSize(size_t Offset, size_t Size) : Offset(Offset), Size(Size) {}
  };

  StringOffsetSize add(StringRef Str);
  StringRef getBuffer() const { return Buffer; }
};

/// Pool of filesystem paths backed by a StringPool
class PathPool {
public:
  /// Special root directory of a filesystem path.
  enum class RootDirKind {
    Regular = 0,
    CurrentWorkDir = 1,
    SysRoot = 2,
  };

  struct DirPath {
    RootDirKind Root;
    StringPool::StringOffsetSize Path;

    DirPath(RootDirKind Root, const StringPool::StringOffsetSize &Path)
        : Root(Root), Path(Path) {}
  };

  struct FilePath {
    DirPath Dir;
    StringPool::StringOffsetSize Filename;

    FilePath(const DirPath &Dir, const StringPool::StringOffsetSize &Filename)
        : Dir(Dir), Filename(Filename) {}
  };

  /// \returns index of the newly added file in FilePaths.
  size_t addFilePath(RootDirKind Root, const StringPool::StringOffsetSize &Dir,
                     StringRef Filename);

  /// \returns offset in Paths and size of newly added directory.
  StringPool::StringOffsetSize addDirPath(StringRef Dir);

  toolchain::ArrayRef<FilePath> getFilePaths() const;

  StringRef getPaths() const;

private:
  StringPool Paths;
  std::vector<FilePath> FilePaths;
};

/// Stores file paths and produces serialization-friendly representation.
class SerializablePathCollection {
  std::string WorkDir;
  std::string SysRoot;

  PathPool Paths;
  toolchain::DenseMap<const clang::FileEntry *, std::size_t> UniqueFiles;
  toolchain::StringMap<PathPool::DirPath, toolchain::BumpPtrAllocator> UniqueDirs;

public:
  const StringPool::StringOffsetSize WorkDirPath;
  const StringPool::StringOffsetSize SysRootPath;
  const StringPool::StringOffsetSize OutputFilePath;

  SerializablePathCollection(toolchain::StringRef CurrentWorkDir,
                             toolchain::StringRef SysRoot,
                             toolchain::StringRef OutputFile);

  /// \returns buffer containing all the paths.
  toolchain::StringRef getPathsBuffer() const { return Paths.getPaths(); }

  /// \returns file paths (no directories) backed by buffer exposed in
  /// getPathsBuffer.
  ArrayRef<PathPool::FilePath> getFilePaths() const {
    return Paths.getFilePaths();
  }

  /// Stores path to \p FE if it hasn't been stored yet.
  /// \returns index to array exposed by getPathsBuffer().
  size_t tryStoreFilePath(FileEntryRef FE);

private:
  /// Stores \p Path if it is non-empty.
  /// Warning: this method doesn't check for uniqueness.
  /// \returns offset of \p Path value begin in buffer with stored paths.
  StringPool::StringOffsetSize storePath(toolchain::StringRef Path);

  /// Stores \p dirStr path if it hasn't been stored yet.
  PathPool::DirPath tryStoreDirPath(toolchain::StringRef dirStr);
};

} // namespace index
} // namespace language::Core

#endif // LANGUAGE_CORE_INDEXSERIALIZATION_SERIALIZABLEPATHCOLLECTION_H
