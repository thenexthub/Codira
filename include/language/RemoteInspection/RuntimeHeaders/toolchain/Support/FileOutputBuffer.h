//=== FileOutputBuffer.h - File Output Buffer -------------------*- C++ -*-===//
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
//
// Utility for creating a in-memory buffer that will be written to a file.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_FILEOUTPUTBUFFER_H
#define TOOLCHAIN_SUPPORT_FILEOUTPUTBUFFER_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/DataTypes.h"
#include "toolchain/Support/Error.h"

namespace toolchain {
/// FileOutputBuffer - This interface provides simple way to create an in-memory
/// buffer which will be written to a file. During the lifetime of these
/// objects, the content or existence of the specified file is undefined. That
/// is, creating an OutputBuffer for a file may immediately remove the file.
/// If the FileOutputBuffer is committed, the target file's content will become
/// the buffer content at the time of the commit.  If the FileOutputBuffer is
/// not committed, the file will be deleted in the FileOutputBuffer destructor.
class FileOutputBuffer {
public:
  enum {
    /// Set the 'x' bit on the resulting file.
    F_executable = 1,

    /// Don't use mmap and instead write an in-memory buffer to a file when this
    /// buffer is closed.
    F_no_mmap = 2,
  };

  /// Factory method to create an OutputBuffer object which manages a read/write
  /// buffer of the specified size. When committed, the buffer will be written
  /// to the file at the specified path.
  ///
  /// When F_modify is specified and \p FilePath refers to an existing on-disk
  /// file \p Size may be set to -1, in which case the entire file is used.
  /// Otherwise, the file shrinks or grows as necessary based on the value of
  /// \p Size.  It is an error to specify F_modify and Size=-1 if \p FilePath
  /// does not exist.
  static Expected<std::unique_ptr<FileOutputBuffer>>
  create(StringRef FilePath, size_t Size, unsigned Flags = 0);

  /// Returns a pointer to the start of the buffer.
  virtual uint8_t *getBufferStart() const = 0;

  /// Returns a pointer to the end of the buffer.
  virtual uint8_t *getBufferEnd() const = 0;

  /// Returns size of the buffer.
  virtual size_t getBufferSize() const = 0;

  /// Returns path where file will show up if buffer is committed.
  StringRef getPath() const { return FinalPath; }

  /// Flushes the content of the buffer to its file and deallocates the
  /// buffer.  If commit() is not called before this object's destructor
  /// is called, the file is deleted in the destructor. The optional parameter
  /// is used if it turns out you want the file size to be smaller than
  /// initially requested.
  virtual Error commit() = 0;

  /// If this object was previously committed, the destructor just deletes
  /// this object.  If this object was not committed, the destructor
  /// deallocates the buffer and the target file is never written.
  virtual ~FileOutputBuffer() = default;

  /// This removes the temporary file (unless it already was committed)
  /// but keeps the memory mapping alive.
  virtual void discard() {}

protected:
  FileOutputBuffer(StringRef Path) : FinalPath(Path) {}

  std::string FinalPath;
};
} // end namespace toolchain

#endif
