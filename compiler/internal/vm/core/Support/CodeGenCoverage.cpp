//===- lib/Support/CodeGenCoverage.cpp -------------------------------------==//
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
/// \file
/// This file implements the CodeGenCoverage class.
//===----------------------------------------------------------------------===//

#include "vm/core/Support/CodeGenCoverage.h"

#include "vm/core/Support/Endian.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Mutex.h"
#include "vm/core/Support/Process.h"
#include "vm/core/Support/ScopedPrinter.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace vm::core;

CodeGenCoverage::CodeGenCoverage() = default;

void CodeGenCoverage::setCovered(uint64_t RuleID) {
  if (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, false);
  RuleCoverage[RuleID] = true;
}

bool CodeGenCoverage::isCovered(uint64_t RuleID) const {
  if (RuleCoverage.size() <= RuleID)
    return false;
  return RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() const {
  return RuleCoverage.set_bits();
}

bool CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  const char *CurPtr = Buffer.getBufferStart();

  while (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    const char *LexedBackendName = CurPtr;
    while (*CurPtr++ != 0)
      ;
    if (CurPtr == Buffer.getBufferEnd())
      return false; // Data is invalid, expected rule id's to follow.

    bool IsForThisBackend = BackendName == LexedBackendName;
    while (CurPtr != Buffer.getBufferEnd()) {
      if (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        return false; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, toolchain::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      if (RuleID == ~0ull)
        break;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      if (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  return true;
}

bool CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) const {
  if (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    static sys::SmartMutex<true> OutputMutex;
    sys::SmartScopedLock<true> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = toolchain::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    if (EC)
      return false;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().write((const char *)&Zero, sizeof(unsigned char));
    for (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().write((const char *)&I, sizeof(uint64_t));
    CoverageFile->os().write((const char *)&InvZero, sizeof(uint64_t));

    CoverageFile->keep();
  }

  return true;
}

void CodeGenCoverage::reset() { RuleCoverage.resize(0); }
