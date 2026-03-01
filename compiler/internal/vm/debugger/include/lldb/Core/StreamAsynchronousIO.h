//===-- StreamAsynchronousIO.h -----------------------------------*- C++-*-===//
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

#ifndef LLDB_CORE_STREAMASYNCHRONOUSIO_H
#define LLDB_CORE_STREAMASYNCHRONOUSIO_H

#include "lldb/Utility/Stream.h"

#include <string>

#include <cstddef>

namespace lldb_private {
class Debugger;

/// A stream meant for asynchronously printing output. Output is buffered until
/// the stream is flushed or destroyed. Printing is handled by the currently
/// active IOHandler, or the debugger's output or error stream if there is none.
class StreamAsynchronousIO : public Stream {
public:
  enum ForSTDOUT : bool {
    STDOUT = true,
    STDERR = false,
  };

  StreamAsynchronousIO(Debugger &debugger, ForSTDOUT for_stdout);

  ~StreamAsynchronousIO() override;

  void Flush() override;

protected:
  size_t WriteImpl(const void *src, size_t src_len) override;

private:
  Debugger &m_debugger;
  std::string m_data;
  ForSTDOUT m_for_stdout;
};

} // namespace lldb_private

#endif // LLDB_CORE_STREAMASYNCHRONOUSIO_H
