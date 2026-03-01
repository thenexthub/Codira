//===----------------- LLDBAssert.h ------------------------------*- C++-*-===//
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

#ifndef LLDB_UTILITY_LLDBASSERT_H
#define LLDB_UTILITY_LLDBASSERT_H

#include "llvm/ADT/StringRef.h"
#include <mutex>

#ifndef NDEBUG
#define lldbassert(x) assert(x)
#else
#if defined(__clang__)
// __FILE_NAME__ is a Clang-specific extension that functions similar to
// __FILE__ but only renders the last path component (the filename) instead of
// an invocation dependent full path to that file.
#define lldbassert(x)                                                          \
  do {                                                                         \
    static std::once_flag _once_flag;                                          \
    lldb_private::_lldb_assert(static_cast<bool>(x), #x, __FUNCTION__,         \
                               __FILE_NAME__, __LINE__, _once_flag);           \
  } while (0)
#else
#define lldbassert(x)                                                          \
  do {                                                                         \
    static std::once_flag _once_flag;                                          \
    lldb_private::_lldb_assert(static_cast<bool>(x), #x, __FUNCTION__,         \
                               __FILE__,  __LINE__, _once_flag);               \
  } while (0)
#endif
#endif

namespace lldb_private {

/// Don't use _lldb_assert directly. Use the lldbassert macro instead so that
/// LLDB asserts become regular asserts in NDEBUG builds.
void _lldb_assert(bool expression, const char *expr_text, const char *func,
                  const char *file, unsigned int line,
                  std::once_flag &once_flag);

/// The default LLDB assert callback, which prints to stderr.
typedef void (*LLDBAssertCallback)(llvm::StringRef message,
                                   llvm::StringRef backtrace,
                                   llvm::StringRef prompt);

/// Replace the LLDB assert callback.
void SetLLDBAssertCallback(LLDBAssertCallback callback);

} // namespace lldb_private

#endif // LLDB_UTILITY_LLDBASSERT_H
