//===-- ThreadMinidump.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_MINIDUMP_THREADMINIDUMP_H
#define LLDB_SOURCE_PLUGINS_PROCESS_MINIDUMP_THREADMINIDUMP_H

#include "MinidumpTypes.h"

#include "lldb/Target/Thread.h"


namespace lldb_private {

namespace minidump {

class ThreadMinidump : public Thread {
public:
  ThreadMinidump(Process &process, const minidump::Thread &td,
                 llvm::ArrayRef<uint8_t> gpregset_data);

  ~ThreadMinidump() override;

  void RefreshStateAfterStop() override;

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(StackFrame *frame) override;

protected:
  lldb::RegisterContextSP m_thread_reg_ctx_sp;
  llvm::ArrayRef<uint8_t> m_gpregset_data;

  bool CalculateStopInfo() override;
};

} // namespace minidump
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_MINIDUMP_THREADMINIDUMP_H
