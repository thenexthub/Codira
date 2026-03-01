//===-- ThreadPostMortemTrace.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_THREADPOSTMORTEMTRACE_H
#define LLDB_TARGET_THREADPOSTMORTEMTRACE_H

#include "lldb/Target/Thread.h"
#include <optional>

namespace lldb_private {

/// \class ThreadPostMortemTrace ThreadPostMortemTrace.h
///
/// Thread implementation used for representing threads gotten from trace
/// session files, which are similar to threads from core files.
///
class ThreadPostMortemTrace : public Thread {
public:
  /// \param[in] process
  ///     The process who owns this thread.
  ///
  /// \param[in] tid
  ///     The tid of this thread.
  ///
  /// \param[in] trace_file
  ///     The file that contains the list of instructions that were traced when
  ///     this thread was being executed.
  ThreadPostMortemTrace(Process &process, lldb::tid_t tid,
                        const std::optional<FileSpec> &trace_file)
      : Thread(process, tid), m_trace_file(trace_file) {}

  void RefreshStateAfterStop() override;

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(StackFrame *frame) override;

  /// \return
  ///   The trace file of this thread.
  const std::optional<FileSpec> &GetTraceFile() const;

protected:
  bool CalculateStopInfo() override;

  lldb::RegisterContextSP m_thread_reg_ctx_sp;

private:
  std::optional<FileSpec> m_trace_file;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPOSTMORTEMTRACE_H
