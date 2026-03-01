//===-- HostInfoAIX.cpp -------------------------------------------------===//
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

#include "lldb/Host/aix/HostInfoAIX.h"
#include "lldb/Host/posix/Support.h"
#include <sys/procfs.h>

using namespace lldb_private;

void HostInfoAIX::Initialize(SharedLibraryDirectoryHelper *helper) {
  HostInfoPosix::Initialize(helper);
}

void HostInfoAIX::Terminate() { HostInfoBase::Terminate(); }

FileSpec HostInfoAIX::GetProgramFileSpec() {
  static FileSpec g_program_filespec;
  struct psinfo psinfoData;
  auto BufferOrError = getProcFile(getpid(), "psinfo");
  if (BufferOrError) {
    std::unique_ptr<llvm::MemoryBuffer> PsinfoBuffer =
        std::move(*BufferOrError);
    memcpy(&psinfoData, PsinfoBuffer->getBufferStart(), sizeof(psinfoData));
    llvm::StringRef exe_path(
        psinfoData.pr_psargs,
        strnlen(psinfoData.pr_psargs, sizeof(psinfoData.pr_psargs)));
    if (!exe_path.empty())
      g_program_filespec.SetFile(exe_path, FileSpec::Style::native);
  }
  return g_program_filespec;
}
