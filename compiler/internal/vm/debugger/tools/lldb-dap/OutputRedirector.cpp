//===-- OutputRedirector.cpp -----------------------------------*- C++ -*-===//
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
//===----------------------------------------------------------------------===/

#include "OutputRedirector.h"
#include "DAP.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <cstring>
#include <system_error>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace llvm;

static constexpr auto kCloseSentinel = StringLiteral::withInnerNUL("\0");

namespace lldb_dap {

int OutputRedirector::kInvalidDescriptor = -1;

OutputRedirector::OutputRedirector()
    : m_fd(kInvalidDescriptor), m_original_fd(kInvalidDescriptor),
      m_restore_fd(kInvalidDescriptor) {}

Expected<int> OutputRedirector::GetWriteFileDescriptor() {
  if (m_fd == kInvalidDescriptor)
    return createStringError(std::errc::bad_file_descriptor,
                             "write handle is not open for writing");
  return m_fd;
}

Error OutputRedirector::RedirectTo(std::FILE *file_override,
                                   std::function<void(StringRef)> callback) {
  assert(m_fd == kInvalidDescriptor && "Output readirector already started.");
  int new_fd[2];

#if defined(_WIN32)
  if (::_pipe(new_fd, OutputBufferSize, O_TEXT) == -1) {
#else
  if (::pipe(new_fd) == -1) {
#endif
    int error = errno;
    return createStringError(inconvertibleErrorCode(),
                             "Couldn't create new pipe %s", strerror(error));
  }

  int read_fd = new_fd[0];
  m_fd = new_fd[1];

  if (file_override) {
    int override_fd = fileno(file_override);

    // Backup the FD to restore once redirection is complete.
    m_original_fd = override_fd;
    m_restore_fd = dup(override_fd);

    // Override the existing fd the new write end of the pipe.
    if (::dup2(m_fd, override_fd) == -1)
      return llvm::errorCodeToError(llvm::errnoAsErrorCode());
  }

  m_forwarder = std::thread([this, callback, read_fd]() {
    char buffer[OutputBufferSize];
    while (!m_stopped) {
      ssize_t bytes_count = ::read(read_fd, &buffer, sizeof(buffer));
      if (bytes_count == -1) {
        // Skip non-fatal errors.
        if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
          continue;
        break;
      }
      // Skip the null byte used to trigger a Stop.
      if (bytes_count == 1 && buffer[0] == '\0')
        continue;

      StringRef data(buffer, bytes_count);
      if (m_stopped)
        data.consume_back(kCloseSentinel);
      if (data.empty())
        break;

      callback(data);
    }
    ::close(read_fd);
  });

  return Error::success();
}

void OutputRedirector::Stop() {
  m_stopped = true;

  if (m_fd != kInvalidDescriptor) {
    int fd = m_fd;
    m_fd = kInvalidDescriptor;
    // Closing the pipe may not be sufficient to wake up the thread in case the
    // write descriptor is duplicated (to stdout/err or to another process).
    // Write a null byte to ensure the read call returns.
    (void)::write(fd, kCloseSentinel.data(), kCloseSentinel.size());
    ::close(fd);
    m_forwarder.join();

    // Restore the fd back to its original state since we stopped the
    // redirection.
    if (m_restore_fd != kInvalidDescriptor &&
        m_original_fd != kInvalidDescriptor) {
      int restore_fd = m_restore_fd;
      m_restore_fd = kInvalidDescriptor;
      int original_fd = m_original_fd;
      m_original_fd = kInvalidDescriptor;
      ::dup2(restore_fd, original_fd);
    }
  }
}

} // namespace lldb_dap
