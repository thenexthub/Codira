//===--- Tracing.cpp - Support code for runtime tracing ------------*- C++ -*-//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Support code for tracing events in the Codira runtime
//
//===----------------------------------------------------------------------===//

#include "Tracing.h"

#if LANGUAGE_STDLIB_TRACING

#define LANGUAGE_LOG_SUBSYSTEM "com.apple.code"
#define LANGUAGE_LOG_SECTION_SCAN_CATEGORY "SectionScan"

namespace language {
namespace runtime {
namespace trace {

os_log_t ScanLog;
language::once_t LogsToken;
bool TracingEnabled;

void setupLogs(void *unused) {
  if (!shouldEnableTracing()) {
    TracingEnabled = false;
    return;
  }

  TracingEnabled = true;
  ScanLog = os_log_create(LANGUAGE_LOG_SUBSYSTEM, LANGUAGE_LOG_SECTION_SCAN_CATEGORY);
}

} // namespace trace
} // namespace runtime
} // namespace language

#endif
