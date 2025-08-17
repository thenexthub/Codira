//===--- SerializablePathCollection.cpp -- Index of paths -------*- C++ -*-===//
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

#include "language/Core/IndexSerialization/SerializablePathCollection.h"
#include "toolchain/Support/Path.h"

using namespace toolchain;
using namespace language::Core;
using namespace language::Core::index;

StringPool::StringOffsetSize StringPool::add(StringRef Str) {
  const std::size_t Offset = Buffer.size();
  Buffer += Str;
  return StringPool::StringOffsetSize(Offset, Str.size());
}

size_t PathPool::addFilePath(RootDirKind Root,
                             const StringPool::StringOffsetSize &Dir,
                             StringRef Filename) {
  FilePaths.emplace_back(DirPath(Root, Dir), Paths.add(Filename));
  return FilePaths.size() - 1;
}

StringPool::StringOffsetSize PathPool::addDirPath(StringRef Dir) {
  return Paths.add(Dir);
}

toolchain::ArrayRef<PathPool::FilePath> PathPool::getFilePaths() const {
  return FilePaths;
}

StringRef PathPool::getPaths() const { return Paths.getBuffer(); }

SerializablePathCollection::SerializablePathCollection(
    StringRef CurrentWorkDir, StringRef SysRoot, toolchain::StringRef OutputFile)
    : WorkDir(CurrentWorkDir),
      SysRoot(toolchain::sys::path::parent_path(SysRoot).empty() ? StringRef()
                                                            : SysRoot),
      WorkDirPath(Paths.addDirPath(WorkDir)),
      SysRootPath(Paths.addDirPath(SysRoot)),
      OutputFilePath(Paths.addDirPath(OutputFile)) {}

size_t SerializablePathCollection::tryStoreFilePath(FileEntryRef FE) {
  auto FileIt = UniqueFiles.find(FE);
  if (FileIt != UniqueFiles.end())
    return FileIt->second;

  const auto Dir = tryStoreDirPath(sys::path::parent_path(FE.getName()));
  const auto FileIdx =
      Paths.addFilePath(Dir.Root, Dir.Path, sys::path::filename(FE.getName()));

  UniqueFiles.try_emplace(FE, FileIdx);
  return FileIdx;
}

PathPool::DirPath SerializablePathCollection::tryStoreDirPath(StringRef Dir) {
  // We don't want to strip separator if Dir is "/" - so we check size > 1.
  while (Dir.size() > 1 && toolchain::sys::path::is_separator(Dir.back()))
    Dir = Dir.drop_back();

  auto DirIt = UniqueDirs.find(Dir);
  if (DirIt != UniqueDirs.end())
    return DirIt->second;

  const std::string OrigDir = Dir.str();

  PathPool::RootDirKind Root = PathPool::RootDirKind::Regular;
  if (!SysRoot.empty() && Dir.starts_with(SysRoot) &&
      toolchain::sys::path::is_separator(Dir[SysRoot.size()])) {
    Root = PathPool::RootDirKind::SysRoot;
    Dir = Dir.drop_front(SysRoot.size());
  } else if (!WorkDir.empty() && Dir.starts_with(WorkDir) &&
             toolchain::sys::path::is_separator(Dir[WorkDir.size()])) {
    Root = PathPool::RootDirKind::CurrentWorkDir;
    Dir = Dir.drop_front(WorkDir.size());
  }

  if (Root != PathPool::RootDirKind::Regular) {
    while (!Dir.empty() && toolchain::sys::path::is_separator(Dir.front()))
      Dir = Dir.drop_front();
  }

  PathPool::DirPath Result(Root, Paths.addDirPath(Dir));
  UniqueDirs.try_emplace(OrigDir, Result);
  return Result;
}
