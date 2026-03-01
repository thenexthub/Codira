//===- FileLineColLocBreakpointManager.cpp - MLIR Optimizer Driver --------===//
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

#include "mlir/Debug/BreakpointManagers/FileLineColLocBreakpointManager.h"

using namespace mlir;
using namespace mlir::tracing;

FailureOr<std::tuple<StringRef, int64_t, int64_t>>
FileLineColLocBreakpoint::parseFromString(StringRef str,
                                          function_ref<void(Twine)> diag) {
  // Watch at debug locations arguments are expected to be in the form:
  // `fileName:line:col`, `fileName:line`, or `fileName`.

  if (str.empty()) {
    if (diag)
      diag("error: initializing FileLineColLocBreakpoint with empty file name");
    return failure();
  }

  // This logic is complex because on Windows `:` is a comment valid path
  // character: `C:\...`.
  auto [fileLine, colStr] = str.rsplit(':');
  auto [file, lineStr] = fileLine.rsplit(':');
  // Extract the line and column value
  int64_t line = -1, col = -1;
  if (lineStr.empty()) {
    // No candidate for line number, try to use the column string as line
    // instead.
    file = fileLine;
    if (!colStr.empty() && colStr.getAsInteger(0, line))
      file = str;
  } else {
    if (lineStr.getAsInteger(0, line)) {
      // Failed to parse a line number, try to use the column string as line
      // instead. If this failed as well, the entire string is the file name.
      file = fileLine;
      if (colStr.getAsInteger(0, line))
        file = str;
    } else {
      // We successfully parsed a line number, try to parse the column number.
      // This shouldn't fail, or the entire string is the file name.
      if (colStr.getAsInteger(0, col)) {
        file = str;
        line = -1;
      }
    }
  }
  return std::tuple<StringRef, int64_t, int64_t>{file, line, col};
}
