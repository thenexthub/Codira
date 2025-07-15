//===--- FileSystem.cpp - Extra helpers for manipulating files ------------===//
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

#include "language/Basic/FileSystem.h"

#include "language/Basic/Assertions.h"
#include "clang/Basic/FileManager.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Process.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <optional>

using namespace language;

namespace {
  class OpenFileRAII {
    static const int INVALID_FD = -1;
  public:
    int fd = INVALID_FD;

    ~OpenFileRAII() {
      if (fd != INVALID_FD)
        toolchain::sys::Process::SafelyCloseFileDescriptor(fd);
    }
  };
} // end anonymous namespace

/// Does some simple checking to see if a temporary file can be written next to
/// \p outputPath and then renamed into place.
///
/// Helper for language::atomicallyWritingToFile.
///
/// If the result is an error, the write won't succeed at all, and the calling
/// operation should bail out early.
static toolchain::ErrorOr<bool>
canUseTemporaryForWrite(const toolchain::StringRef outputPath) {
  namespace fs = toolchain::sys::fs;

  if (outputPath == "-") {
    // Special case: "-" represents stdout, and LLVM's output stream APIs are
    // aware of this. It doesn't make sense to use a temporary in this case.
    return false;
  }

  fs::file_status status;
  (void)fs::status(outputPath, status);
  if (!fs::exists(status)) {
    // Assume we'll be able to write to both a temporary file and to the final
    // destination if the final destination doesn't exist yet.
    return true;
  }

  // Fail early if we can't write to the final destination.
  if (!fs::can_write(outputPath))
    return toolchain::make_error_code(toolchain::errc::operation_not_permitted);

  // Only use a temporary if the output is a regular file. This handles
  // things like '-o /dev/null'
  return fs::is_regular_file(status);
}

/// Attempts to open a temporary file next to \p outputPath, with the intent
/// that once the file has been written it will be renamed into place.
///
/// Helper for language::atomicallyWritingToFile.
///
/// \param[out] openedStream On success, a stream opened for writing to the
/// temporary file that was just created.
/// \param outputPath The path to the final output file, which is used to decide
/// where to put the temporary.
///
/// \returns The path to the temporary file that was opened, or \c None if the
/// file couldn't be created.
static std::optional<std::string>
tryToOpenTemporaryFile(std::optional<toolchain::raw_fd_ostream> &openedStream,
                       const toolchain::StringRef outputPath) {
  namespace fs = toolchain::sys::fs;

  // Create a temporary file path.
  // Insert a placeholder for a random suffix before the extension (if any).
  // Then because some tools glob for build artifacts (such as clang's own
  // GlobalModuleIndex.cpp), also append .tmp.
  toolchain::SmallString<128> tempPath;
  const toolchain::StringRef outputExtension =
      toolchain::sys::path::extension(outputPath);
  tempPath = outputPath.drop_back(outputExtension.size());
  tempPath += "-%%%%%%%%";
  tempPath += outputExtension;
  tempPath += ".tmp";

  int fd;
  const unsigned perms = fs::all_read | fs::all_write;
  std::error_code EC = fs::createUniqueFile(tempPath, fd, tempPath,
                                            fs::OF_None, perms);

  if (EC) {
    // Ignore the specific error; the caller has to fall back to not using a
    // temporary anyway.
    return std::nullopt;
  }

  openedStream.emplace(fd, /*shouldClose=*/true);
  // Make sure the temporary file gets removed if we crash.
  toolchain::sys::RemoveFileOnSignal(tempPath);
  return tempPath.str().str();
}

std::error_code language::atomicallyWritingToFile(
    const toolchain::StringRef outputPath,
    const toolchain::function_ref<void(toolchain::raw_pwrite_stream &)> action) {
  namespace fs = toolchain::sys::fs;

  // FIXME: This is mostly a simplified version of
  // clang::CompilerInstance::createOutputFile. It would be great to share the
  // implementation.
  assert(!outputPath.empty());

  toolchain::ErrorOr<bool> canUseTemporary = canUseTemporaryForWrite(outputPath);
  if (std::error_code error = canUseTemporary.getError())
    return error;

  std::optional<std::string> temporaryPath;
  {
    std::optional<toolchain::raw_fd_ostream> OS;
    if (canUseTemporary.get()) {
      temporaryPath = tryToOpenTemporaryFile(OS, outputPath);

      if (!temporaryPath) {
        assert(!OS.has_value());
        // If we failed to create the temporary, fall back to writing to the
        // file directly. This handles the corner case where we cannot write to
        // the directory, but can write to the file.
      }
    }

    if (!OS.has_value()) {
      std::error_code error;
      OS.emplace(outputPath, error, fs::OF_None);
      if (error) {
        return error;
      }
    }

    action(OS.value());
    // In addition to scoping the use of 'OS', ending the scope here also
    // ensures that it's been flushed (by destroying it).
  }

  if (!temporaryPath.has_value()) {
    // If we didn't use a temporary, we're done!
    return std::error_code();
  }

  return language::moveFileIfDifferent(temporaryPath.value(), outputPath);
}

toolchain::ErrorOr<FileDifference>
language::areFilesDifferent(const toolchain::Twine &source,
                         const toolchain::Twine &destination,
                         bool allowDestinationErrors) {
  namespace fs = toolchain::sys::fs;

  if (fs::equivalent(source, destination)) {
    return FileDifference::IdenticalFile;
  }

  OpenFileRAII sourceFile;
  fs::file_status sourceStatus;
  if (std::error_code error = fs::openFileForRead(source, sourceFile.fd)) {
    // If we can't open the source file, fail.
    return error;
  }
  if (std::error_code error = fs::status(sourceFile.fd, sourceStatus)) {
    // If we can't stat the source file, fail.
    return error;
  }

  /// Converts an error from the destination file into either an error or
  /// DifferentContents return, depending on `allowDestinationErrors`.
  auto convertDestinationError = [=](std::error_code error) ->
      toolchain::ErrorOr<FileDifference> {
    if (allowDestinationErrors){
      return FileDifference::DifferentContents;
    }
    return error;
  };

  OpenFileRAII destFile;
  fs::file_status destStatus;
  if (std::error_code error = fs::openFileForRead(destination, destFile.fd)) {
    // If we can't open the destination file, fail in the specified fashion.
    return convertDestinationError(error);
  }
  if (std::error_code error = fs::status(destFile.fd, destStatus)) {
    // If we can't open the destination file, fail in the specified fashion.
    return convertDestinationError(error);
  }

  uint64_t size = sourceStatus.getSize();
  if (size != destStatus.getSize()) {
    // If the files are different sizes, they must be different.
    return FileDifference::DifferentContents;
  }
  if (size == 0) {
    // If both files are zero size, they must be the same.
    return FileDifference::SameContents;
  }

  // The two files match in size, so we have to compare the bytes to determine
  // if they're the same.
  std::error_code sourceRegionErr;
  fs::mapped_file_region sourceRegion(fs::convertFDToNativeFile(sourceFile.fd),
                                      fs::mapped_file_region::readonly,
                                      size, 0, sourceRegionErr);
  if (sourceRegionErr) {
    return sourceRegionErr;
  }

  std::error_code destRegionErr;
  fs::mapped_file_region destRegion(fs::convertFDToNativeFile(destFile.fd),
                                    fs::mapped_file_region::readonly,
                                    size, 0, destRegionErr);

  if (destRegionErr) {
    return convertDestinationError(destRegionErr);
  }

  if (memcmp(sourceRegion.const_data(), destRegion.const_data(), size) != 0) {
    return FileDifference::DifferentContents;
  }

  return FileDifference::SameContents;
}

std::error_code language::moveFileIfDifferent(const toolchain::Twine &source,
                                           const toolchain::Twine &destination) {
  namespace fs = toolchain::sys::fs;

  auto result = areFilesDifferent(source, destination,
                                  /*allowDestinationErrors=*/true);

  if (!result)
    return result.getError();

  switch (*result) {
  case FileDifference::IdenticalFile:
    // Do nothing for a self-move.
    return std::error_code();

  case FileDifference::SameContents:
    // Files are identical; remove the source file.
    return fs::remove(source);

  case FileDifference::DifferentContents:
    // Files are different; overwrite the destination file.
    return fs::rename(source, destination);
  }
  toolchain_unreachable("Unhandled FileDifference in switch");
}

toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
language::vfs::getFileOrSTDIN(toolchain::vfs::FileSystem &FS,
                           const toolchain::Twine &Filename,
                           int64_t FileSize,
                           bool RequiresNullTerminator,
                           bool IsVolatile,
                           unsigned BADFRetry) {
  toolchain::SmallString<256> NameBuf;
  toolchain::StringRef NameRef = Filename.toStringRef(NameBuf);

  if (NameRef == "-")
    return toolchain::MemoryBuffer::getSTDIN();
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> inputFileOrErr = nullptr;
  for (unsigned I = 0; I != BADFRetry + 1; ++ I) {
    inputFileOrErr = FS.getBufferForFile(Filename, FileSize,
                                         RequiresNullTerminator, IsVolatile);
    if (inputFileOrErr)
      return inputFileOrErr;
    if (inputFileOrErr.getError().value() != EBADF)
      return inputFileOrErr;
  }
  return inputFileOrErr;
}
