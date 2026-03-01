//===-- OutputRedirector.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_OUTPUT_REDIRECTOR_H
#define LLDB_TOOLS_LLDB_DAP_OUTPUT_REDIRECTOR_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <atomic>
#include <functional>
#include <thread>

namespace lldb_dap {

class OutputRedirector {
public:
  static int kInvalidDescriptor;

  /// Creates writable file descriptor that will invoke the given callback on
  /// each write in a background thread.
  ///
  /// \param[in] file_override
  ///     Updates the file descriptor to the redirection pipe, if not null.
  ///
  /// \param[in] callback
  ///     A callback invoked when any data is written to the file handle.
  ///
  /// \return
  ///     \a Error::success if the redirection was set up correctly, or an error
  ///     otherwise.
  llvm::Error RedirectTo(std::FILE *file_override,
                         std::function<void(llvm::StringRef)> callback);

  llvm::Expected<int> GetWriteFileDescriptor();
  void Stop();

  ~OutputRedirector() { Stop(); }

  OutputRedirector();
  OutputRedirector(const OutputRedirector &) = delete;
  OutputRedirector &operator=(const OutputRedirector &) = delete;

private:
  std::atomic<bool> m_stopped = false;
  int m_fd;
  int m_original_fd;
  int m_restore_fd;
  std::thread m_forwarder;
};

} // namespace lldb_dap

#endif // LLDB_TOOLS_LLDB_DAP_OUTPUT_REDIRECTOR_H
