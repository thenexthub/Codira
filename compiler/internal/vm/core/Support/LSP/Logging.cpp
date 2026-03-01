//===- Logging.cpp --------------------------------------------------------===//
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

#include "vm/core/Support/LSP/Logging.h"
#include "vm/core/Support/Chrono.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace vm::core::lsp;

void Logger::setLogLevel(Level LogLevel) { get().LogLevel = LogLevel; }

Logger &Logger::get() {
  static Logger Logger;
  return Logger;
}

void Logger::log(Level LogLevel, const char *Fmt,
                 const toolchain::formatv_object_base &Message) {
  Logger &Logger = get();

  // Ignore messages with log levels below the current setting in the logger.
  if (LogLevel < Logger.LogLevel)
    return;

  // An indicator character for each log level.
  const char *LogLevelIndicators = "DIE";

  // Format the message and print to errs.
  toolchain::sys::TimePoint<> Timestamp = std::chrono::system_clock::now();
  std::lock_guard<std::mutex> LogGuard(Logger.Mutex);
  toolchain::errs() << toolchain::formatv(
      "{0}[{1:%H:%M:%S.%L}] {2}\n",
      LogLevelIndicators[static_cast<unsigned>(LogLevel)], Timestamp, Message);
  toolchain::errs().flush();
}
