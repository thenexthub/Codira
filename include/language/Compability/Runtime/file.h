//===-- language/Compability-rt/runtime/file.h -------------------------*- C++ -*-===//
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

// Raw system I/O wrappers

#ifndef FLANG_RT_RUNTIME_FILE_H_
#define FLANG_RT_RUNTIME_FILE_H_

#include "io-error.h"
#include "memory.h"
#include "language/Compability/Common/optional.h"
#include <cinttypes>

namespace language::Compability::runtime::io {

enum class OpenStatus { Old, New, Scratch, Replace, Unknown };
enum class CloseStatus { Keep, Delete };
enum class Position { AsIs, Rewind, Append };
enum class Action { Read, Write, ReadWrite };

class OpenFile {
public:
  using FileOffset = std::int64_t;

  const char *path() const { return path_.get(); }
  std::size_t pathLength() const { return pathLength_; }
  void set_path(OwningPtr<char> &&, std::size_t bytes);
  bool mayRead() const { return mayRead_; }
  bool mayWrite() const { return mayWrite_; }
  bool mayPosition() const { return mayPosition_; }
  bool mayAsynchronous() const { return mayAsynchronous_; }
  void set_mayAsynchronous(bool yes) { mayAsynchronous_ = yes; }
  bool isTerminal() const { return isTerminal_; }
  bool isWindowsTextFile() const { return isWindowsTextFile_; }
  language::Compability::common::optional<FileOffset> knownSize() const { return knownSize_; }

  bool IsConnected() const { return fd_ >= 0; }
  void Open(OpenStatus, language::Compability::common::optional<Action>, Position,
      IoErrorHandler &);
  void Predefine(int fd);
  void Close(CloseStatus, IoErrorHandler &);

  // Reads data into memory; returns amount acquired.  Synchronous.
  // Partial reads (less than minBytes) signify end-of-file.  If the
  // buffer is larger than minBytes, and extra returned data will be
  // preserved for future consumption, set maxBytes larger than minBytes
  // to reduce system calls  This routine handles EAGAIN/EWOULDBLOCK and EINTR.
  std::size_t Read(FileOffset, char *, std::size_t minBytes,
      std::size_t maxBytes, IoErrorHandler &);

  // Writes data.  Synchronous.  Partial writes indicate program-handled
  // error conditions.
  std::size_t Write(FileOffset, const char *, std::size_t, IoErrorHandler &);

  // Truncates the file
  void Truncate(FileOffset, IoErrorHandler &);

  // Asynchronous transfers
  int ReadAsynchronously(FileOffset, char *, std::size_t, IoErrorHandler &);
  int WriteAsynchronously(
      FileOffset, const char *, std::size_t, IoErrorHandler &);
  void Wait(int id, IoErrorHandler &);
  void WaitAll(IoErrorHandler &);

  // INQUIRE(POSITION=)
  Position InquirePosition() const;

private:
  struct Pending {
    int id;
    int ioStat{0};
    OwningPtr<Pending> next;
  };

  void CheckOpen(const Terminator &);
  bool Seek(FileOffset, IoErrorHandler &);
  bool RawSeek(FileOffset);
  bool RawSeekToEnd();
  int PendingResult(const Terminator &, int);
  void SetPosition(FileOffset pos) {
    position_ = pos;
    openPosition_.reset();
  }
  void CloseFd(IoErrorHandler &);

  int fd_{-1};
  OwningPtr<char> path_;
  std::size_t pathLength_;
  bool mayRead_{false};
  bool mayWrite_{false};
  bool mayPosition_{false};
  bool mayAsynchronous_{false};
  language::Compability::common::optional<Position>
      openPosition_; // from Open(); reset after positioning
  FileOffset position_{0};
  language::Compability::common::optional<FileOffset> knownSize_;
  bool isTerminal_{false};
  bool isWindowsTextFile_{false}; // expands LF to CR+LF on write

  int nextId_;
  OwningPtr<Pending> pending_;
};

RT_API_ATTRS bool IsATerminal(int fd);
RT_API_ATTRS bool IsExtant(const char *path);
RT_API_ATTRS bool MayRead(const char *path);
RT_API_ATTRS bool MayWrite(const char *path);
RT_API_ATTRS bool MayReadAndWrite(const char *path);
RT_API_ATTRS std::int64_t SizeInBytes(const char *path);
} // namespace language::Compability::runtime::io
#endif // FLANG_RT_RUNTIME_FILE_H_
