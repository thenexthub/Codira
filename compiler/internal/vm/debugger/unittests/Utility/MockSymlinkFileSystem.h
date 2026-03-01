//===-- MockSymlinkFileSystem.h
//--------------------------------------------------===//
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

#include "lldb/Utility/FileSpec.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace lldb_private {

// A mock file system that realpath's a given symlink to a given realpath.
class MockSymlinkFileSystem : public llvm::vfs::FileSystem {
public:
  // Treat all files as non-symlinks.
  MockSymlinkFileSystem() = default;

  /// Treat \a symlink as a symlink to \a realpath. Treat all other files as
  /// non-symlinks.
  MockSymlinkFileSystem(FileSpec &&symlink, FileSpec &&realpath,
                        FileSpec::Style style = FileSpec::Style::native)
      : m_symlink(std::move(symlink)), m_realpath(std::move(realpath)),
        m_style(style) {}

  /// If \a Path matches the symlink given in the ctor, put the realpath given
  /// in the ctor into \a Output.
  std::error_code getRealPath(const llvm::Twine &Path,
                              llvm::SmallVectorImpl<char> &Output) override {
    if (FileSpec(Path.str(), m_style) == m_symlink) {
      std::string path = m_realpath.GetPath();
      Output.assign(path.begin(), path.end());
    } else {
      Path.toVector(Output);
    }
    return {};
  }

  // Implement the rest of the interface
  llvm::ErrorOr<llvm::vfs::Status> status(const llvm::Twine &Path) override {
    return llvm::errc::operation_not_permitted;
  }
  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(const llvm::Twine &Path) override {
    return llvm::errc::operation_not_permitted;
  }
  llvm::vfs::directory_iterator dir_begin(const llvm::Twine &Dir,
                                          std::error_code &EC) override {
    return llvm::vfs::directory_iterator();
  }
  std::error_code setCurrentWorkingDirectory(const llvm::Twine &Path) override {
    return llvm::errc::operation_not_permitted;
  }
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    return llvm::errc::operation_not_permitted;
  }

private:
  FileSpec m_symlink;
  FileSpec m_realpath;
  FileSpec::Style m_style;
};

} // namespace lldb_private
