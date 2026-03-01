//===-- DAPLog.cpp --------------------------------------------------------===//
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

#include "DAPLog.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>
#include <mutex>

using namespace llvm;

namespace lldb_dap {

void Log::Emit(StringRef message) { Emit(message, "", 0); }

void Log::Emit(StringRef message, StringRef file, size_t line) {
  std::lock_guard<Log::Mutex> lock(m_mutex);
  const llvm::sys::TimePoint<> time = std::chrono::system_clock::now();
  m_stream << formatv("[{0:%H:%M:%S.%L}]", time) << " ";
  if (!file.empty())
    m_stream << sys::path::filename(file) << ":" << line << " ";
  if (!m_prefix.empty())
    m_stream << m_prefix;
  m_stream << message << "\n";
  m_stream.flush();
}

} // namespace lldb_dap
