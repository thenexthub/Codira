//===- FileOutputBuffer.cpp - File Output Buffer ----------------*- C++ -*-===//
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
//
// Utility for creating a in-memory buffer that will be written to a file.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/FileOutputBuffer.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/Memory.h"
#include "vm/core/Support/TimeProfiler.h"
#include <system_error>

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif

using namespace vm::core;
using namespace vm::core::sys;

namespace {
// A FileOutputBuffer which creates a temporary file in the same directory
// as the final output file. The final output file is atomically replaced
// with the temporary file on commit().
class OnDiskBuffer : public FileOutputBuffer {
public:
  OnDiskBuffer(StringRef Path, fs::TempFile Temp, fs::mapped_file_region Buf)
      : FileOutputBuffer(Path), Buffer(std::move(Buf)), Temp(std::move(Temp)) {}

  uint8_t *getBufferStart() const override { return (uint8_t *)Buffer.data(); }

  uint8_t *getBufferEnd() const override {
    return (uint8_t *)Buffer.data() + Buffer.size();
  }

  size_t getBufferSize() const override { return Buffer.size(); }

  Error commit() override {
    toolchain::TimeTraceScope timeScope("Commit buffer to disk");

    // Unmap buffer, letting OS flush dirty pages to file on disk.
    Buffer.unmap();

    // Atomically replace the existing file with the new one.
    return Temp.keep(FinalPath);
  }

  ~OnDiskBuffer() override {
    // Close the mapping before deleting the temp file, so that the removal
    // succeeds.
    Buffer.unmap();
    consumeError(Temp.discard());
  }

  void discard() override {
    // Delete the temp file if it still was open, but keeping the mapping
    // active.
    consumeError(Temp.discard());
  }

private:
  fs::mapped_file_region Buffer;
  fs::TempFile Temp;
};

// A FileOutputBuffer which keeps data in memory and writes to the final
// output file on commit(). This is used only when we cannot use OnDiskBuffer.
class InMemoryBuffer : public FileOutputBuffer {
public:
  InMemoryBuffer(StringRef Path, MemoryBlock Buf, std::size_t BufSize,
                 unsigned Mode)
      : FileOutputBuffer(Path), Buffer(Buf), BufferSize(BufSize),
        Mode(Mode) {}

  uint8_t *getBufferStart() const override { return (uint8_t *)Buffer.base(); }

  uint8_t *getBufferEnd() const override {
    return (uint8_t *)Buffer.base() + BufferSize;
  }

  size_t getBufferSize() const override { return BufferSize; }

  Error commit() override {
    if (FinalPath == "-") {
      toolchain::outs() << StringRef((const char *)Buffer.base(), BufferSize);
      toolchain::outs().flush();
      return Error::success();
    }

    using namespace sys::fs;
    int FD;
    std::error_code EC;
    if (auto EC =
            openFileForWrite(FinalPath, FD, CD_CreateAlways, OF_Delete, Mode))
      return errorCodeToError(EC);
    raw_fd_ostream OS(FD, /*shouldClose=*/true, /*unbuffered=*/true);
    OS << StringRef((const char *)Buffer.base(), BufferSize);
    return Error::success();
  }

private:
  // Buffer may actually contain a larger memory block than BufferSize
  OwningMemoryBlock Buffer;
  size_t BufferSize;
  unsigned Mode;
};
} // namespace

static Expected<std::unique_ptr<InMemoryBuffer>>
createInMemoryBuffer(StringRef Path, size_t Size, unsigned Mode) {
  std::error_code EC;
  MemoryBlock MB = Memory::allocateMappedMemory(
      Size, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);
  if (EC)
    return errorCodeToError(EC);
  return std::make_unique<InMemoryBuffer>(Path, MB, Size, Mode);
}

static Expected<std::unique_ptr<FileOutputBuffer>>
createOnDiskBuffer(StringRef Path, size_t Size, unsigned Mode) {
  Expected<fs::TempFile> FileOrErr =
      fs::TempFile::create(Path + ".tmp%%%%%%%", Mode);
  if (!FileOrErr)
    return FileOrErr.takeError();
  fs::TempFile File = std::move(*FileOrErr);

  if (auto EC = fs::resize_file_before_mapping_readwrite(File.FD, Size)) {
    consumeError(File.discard());
    return errorCodeToError(EC);
  }

  // Mmap it.
  std::error_code EC;
  fs::mapped_file_region MappedFile =
      fs::mapped_file_region(fs::convertFDToNativeFile(File.FD),
                             fs::mapped_file_region::readwrite, Size, 0, EC);

  // mmap(2) can fail if the underlying filesystem does not support it.
  // If that happens, we fall back to in-memory buffer as the last resort.
  if (EC) {
    consumeError(File.discard());
    return createInMemoryBuffer(Path, Size, Mode);
  }

  return std::make_unique<OnDiskBuffer>(Path, std::move(File),
                                         std::move(MappedFile));
}

// Create an instance of FileOutputBuffer.
Expected<std::unique_ptr<FileOutputBuffer>>
FileOutputBuffer::create(StringRef Path, size_t Size, unsigned Flags) {
  // Handle "-" as stdout just like toolchain::raw_ostream does.
  if (Path == "-")
    return createInMemoryBuffer("-", Size, /*Mode=*/0);

  unsigned Mode = fs::all_read | fs::all_write;
  if (Flags & F_executable)
    Mode |= fs::all_exe;

  // If Size is zero, don't use mmap which will fail with EINVAL.
  if (Size == 0)
    return createInMemoryBuffer(Path, Size, Mode);

  fs::file_status Stat;
  fs::status(Path, Stat);

  // Usually, we want to create OnDiskBuffer to create a temporary file in
  // the same directory as the destination file and atomically replaces it
  // by rename(2).
  //
  // However, if the destination file is a special file, we don't want to
  // use rename (e.g. we don't want to replace /dev/null with a regular
  // file.) If that's the case, we create an in-memory buffer, open the
  // destination file and write to it on commit().
  switch (Stat.type()) {
  case fs::file_type::directory_file:
    return errorCodeToError(errc::is_a_directory);
  case fs::file_type::regular_file:
  case fs::file_type::file_not_found:
  case fs::file_type::status_error:
    if (Flags & F_mmap)
      return createInMemoryBuffer(Path, Size, Mode);
    else
      return createOnDiskBuffer(Path, Size, Mode);
  default:
    return createInMemoryBuffer(Path, Size, Mode);
  }
}
