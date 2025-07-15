//===--- FileSystem.h - Extra helpers for manipulating files ----*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_FILESYSTEM_H
#define LANGUAGE_BASIC_FILESYSTEM_H

#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/MemoryBuffer.h"
#include <system_error>

namespace toolchain {
  class raw_pwrite_stream;
  class Twine;
}

namespace toolchain {
  namespace vfs {
    class FileSystem;
  }
}

namespace language {
  /// Invokes \p action with a raw_ostream that refers to a temporary file,
  /// which is then renamed into place as \p outputPath when the action
  /// completes.
  ///
  /// If a temporary file cannot be created for whatever reason, \p action will
  /// be invoked with a stream directly opened at \p outputPath. Otherwise, if
  /// there is already a file at \p outputPath, it will not be overwritten if
  /// the new contents are identical.
  ///
  /// If the process is interrupted with a signal, any temporary file will be
  /// removed.
  ///
  /// As a special case, an output path of "-" is treated as referring to
  /// stdout.
  std::error_code atomicallyWritingToFile(
      toolchain::StringRef outputPath,
      toolchain::function_ref<void(toolchain::raw_pwrite_stream &)> action);

  /// Moves a file from \p source to \p destination, unless there is already
  /// a file at \p destination that contains the same data as \p source.
  ///
  /// In the latter case, the file at \p source is deleted. If an error occurs,
  /// the file at \p source will still be present at \p source.
  std::error_code moveFileIfDifferent(const toolchain::Twine &source,
                                      const toolchain::Twine &destination);

  enum class FileDifference : uint8_t {
    /// The source and destination paths refer to the exact same file.
    IdenticalFile,
    /// The source and destination paths refer to separate files with identical
    /// contents.
    SameContents,
    /// The source and destination paths refer to separate files with different
    /// contents.
    DifferentContents
  };

  /// Compares the files at \p source and \p destination to determine if they
  /// are the exact same files, different files with the same contents, or
  /// different files with different contents. If \p allowDestinationErrors is
  /// set, file system errors relating to the \p destination file return a
  /// \c DifferentFile result, rather than an error.
  toolchain::ErrorOr<FileDifference>
  areFilesDifferent(const toolchain::Twine &source, const toolchain::Twine &destination,
                    bool allowDestinationErrors);

  namespace vfs {
    toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
    getFileOrSTDIN(toolchain::vfs::FileSystem &FS,
                   const toolchain::Twine &Name, int64_t FileSize = -1,
                   bool RequiresNullTerminator = true, bool IsVolatile = false,
                   unsigned BADFRetry = 0);
  } // end namespace vfs

} // end namespace language

#endif // LANGUAGE_BASIC_FILESYSTEM_H
