//===-- LLDBAssert.cpp ----------------------------------------------------===//
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

#include "lldb/Utility/LLDBAssert.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include <mutex>

#if LLVM_SUPPORT_XCODE_SIGNPOSTS
#include <os/log.h>
#endif

#include <atomic>

namespace lldb_private {

/// The default callback prints to stderr.
static void DefaultAssertCallback(llvm::StringRef message,
                                  llvm::StringRef backtrace,
                                  llvm::StringRef prompt) {
  llvm::errs() << message << '\n';
  llvm::errs() << backtrace; // Backtrace includes a newline.
  llvm::errs() << prompt << '\n';
}

static std::atomic<LLDBAssertCallback> g_lldb_assert_callback =
    &DefaultAssertCallback;

void _lldb_assert(bool expression, const char *expr_text, const char *func,
                  const char *file, unsigned int line,
                  std::once_flag &once_flag) {
  if (LLVM_LIKELY(expression))
    return;

  std::call_once(once_flag, [&]() {
#if LLVM_SUPPORT_XCODE_SIGNPOSTS
    if (__builtin_available(macos 10.12, iOS 10, tvOS 10, watchOS 3, *)) {
      os_log_fault(OS_LOG_DEFAULT,
                   "Assertion failed: (%s), function %s, file %s, line %u\n",
                   expr_text, func, file, line);
    }
#endif

    std::string buffer;
    llvm::raw_string_ostream backtrace(buffer);
    llvm::sys::PrintStackTrace(backtrace);

    (*g_lldb_assert_callback.load())(
        llvm::formatv(
            "Assertion failed: ({0}), function {1}, file {2}, line {3}",
            expr_text, func, file, line)
            .str(),
        buffer,
        "Please file a bug report against lldb and include the backtrace, the "
        "version and as many details as possible.");
  });
}

void SetLLDBAssertCallback(LLDBAssertCallback callback) {
  g_lldb_assert_callback.exchange(callback);
}

} // namespace lldb_private
