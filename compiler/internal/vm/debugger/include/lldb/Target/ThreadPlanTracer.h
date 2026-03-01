//===-- ThreadPlanTracer.h --------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_TARGET_THREADPLANTRACER_H
#define LLDB_TARGET_THREADPLANTRACER_H

#include "lldb/Symbol/TaggedASTType.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/RegisterValue.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class ThreadPlanTracer {
  friend class ThreadPlan;

public:
  enum ThreadPlanTracerStyle {
    eLocation = 0,
    eStateChange,
    eCheckFrames,
    ePython
  };

  ThreadPlanTracer(Thread &thread, lldb::StreamSP &stream_sp);
  ThreadPlanTracer(Thread &thread);

  virtual ~ThreadPlanTracer() = default;

  virtual void TracingStarted() {}

  virtual void TracingEnded() {}

  bool EnableTracing(bool value) {
    bool old_value = m_enabled;
    m_enabled = value;
    if (old_value == false && value == true)
      TracingStarted();
    else if (old_value == true && value == false)
      TracingEnded();

    return old_value;
  }

  bool TracingEnabled() { return m_enabled; }

  Thread &GetThread();

protected:
  Process &m_process;
  lldb::tid_t m_tid;

  lldb::StreamSP GetLogStreamSP();

  virtual void Log();

private:
  bool TracerExplainsStop();

  bool m_enabled;
  lldb::StreamSP m_stream_sp;
  Thread *m_thread;
};

class ThreadPlanAssemblyTracer : public ThreadPlanTracer {
public:
  ThreadPlanAssemblyTracer(Thread &thread, lldb::StreamSP &stream_sp);
  ThreadPlanAssemblyTracer(Thread &thread);
  ~ThreadPlanAssemblyTracer() override;

  void TracingStarted() override;
  void TracingEnded() override;
  void Log() override;

private:
  Disassembler *GetDisassembler();

  TypeFromUser GetIntPointerType();

  lldb::DisassemblerSP m_disassembler_sp;
  TypeFromUser m_intptr_type;
  std::vector<RegisterValue> m_register_values;
  lldb::DataBufferSP m_buffer_sp;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANTRACER_H
