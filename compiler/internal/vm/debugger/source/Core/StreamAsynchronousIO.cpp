//===-- StreamAsynchronousIO.cpp ------------------------------------------===//
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

#include "lldb/Core/StreamAsynchronousIO.h"

#include "lldb/Core/Debugger.h"
#include "lldb/lldb-enumerations.h"

using namespace lldb;
using namespace lldb_private;

StreamAsynchronousIO::StreamAsynchronousIO(
    Debugger &debugger, StreamAsynchronousIO::ForSTDOUT for_stdout)
    : Stream(0, 4, eByteOrderBig, debugger.GetUseColor()), m_debugger(debugger),
      m_data(), m_for_stdout(for_stdout) {}

StreamAsynchronousIO::~StreamAsynchronousIO() {
  // Flush when we destroy to make sure we display the data.
  Flush();
}

void StreamAsynchronousIO::Flush() {
  if (!m_data.empty()) {
    m_debugger.PrintAsync(m_data.data(), m_data.size(), m_for_stdout);
    m_data.clear();
  }
}

size_t StreamAsynchronousIO::WriteImpl(const void *s, size_t length) {
  m_data.append((const char *)s, length);
  return length;
}
