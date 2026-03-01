//===-- PostMortemProcess.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_POSTMORTEMPROCESS_H
#define LLDB_TARGET_POSTMORTEMPROCESS_H

#include "lldb/Target/Process.h"

namespace lldb_private {

/// \class PostMortemProcess
/// Base class for all processes that don't represent a live process, such as
/// coredumps or processes traced in the past.
///
/// \a lldb_private::Process virtual functions overrides that are common
/// between these kinds of processes can have default implementations in this
/// class.
class PostMortemProcess : public Process {
  using Process::Process;

public:
  PostMortemProcess(lldb::TargetSP target_sp, lldb::ListenerSP listener_sp,
                    const FileSpec &core_file)
      : Process(target_sp, listener_sp), m_core_file(core_file) {}

  bool IsLiveDebugSession() const override { return false; }

  FileSpec GetCoreFile() const override { return m_core_file; }

protected:
  FileSpec m_core_file;
};

} // namespace lldb_private

#endif // LLDB_TARGET_POSTMORTEMPROCESS_H
