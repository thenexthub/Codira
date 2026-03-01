//===-- Support.cpp -------------------------------------------------------===//
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

#include "lldb/Host/aix/Support.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "llvm/Support/MemoryBuffer.h"

llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
lldb_private::getProcFile(::pid_t pid, ::pid_t tid, const llvm::Twine &file) {
  Log *log = GetLog(LLDBLog::Host);
  std::string File =
      ("/proc/" + llvm::Twine(pid) + "/lwp/" + llvm::Twine(tid) + "/" + file)
          .str();
  auto Ret = llvm::MemoryBuffer::getFileAsStream(File);
  if (!Ret)
    LLDB_LOG(log, "Failed to open {0}: {1}", File, Ret.getError().message());
  return Ret;
}
