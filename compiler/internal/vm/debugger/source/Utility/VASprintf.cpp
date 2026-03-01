//===-- VASprintf.cpp -----------------------------------------------------===//
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

#include "lldb/Utility/VASPrintf.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>

bool lldb_private::VASprintf(llvm::SmallVectorImpl<char> &buf, const char *fmt,
                             va_list args) {
  llvm::SmallString<16> error("<Encoding error>");
  bool result = true;

  // Copy in case our first call to vsnprintf doesn't fit into our buffer
  va_list copy_args;
  va_copy(copy_args, args);

  buf.resize(buf.capacity());
  // Write up to `capacity` bytes, ignoring the current size.
  int length = ::vsnprintf(buf.data(), buf.size(), fmt, args);
  if (length < 0) {
    buf = error;
    result = false;
    goto finish;
  }

  if (size_t(length) >= buf.size()) {
    // The error formatted string didn't fit into our buffer, resize it to the
    // exact needed size, and retry
    buf.resize(length + 1);
    length = ::vsnprintf(buf.data(), buf.size(), fmt, copy_args);
    if (length < 0) {
      buf = error;
      result = false;
      goto finish;
    }
    assert(size_t(length) < buf.size());
  }
  buf.resize(length);

finish:
  va_end(args);
  va_end(copy_args);
  return result;
}
