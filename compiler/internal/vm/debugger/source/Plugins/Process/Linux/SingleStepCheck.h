//===-- SingleStepCheck.h ------------------------------------- -*- C++ -*-===//
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

#ifndef liblldb_SingleStepCheck_H_
#define liblldb_SingleStepCheck_H_

#include <memory>
#include <sched.h>
#include <sys/types.h>

namespace lldb_private {
namespace process_linux {

// arm64 linux had a bug which prevented single-stepping and watchpoints from
// working on non-boot cpus, due to them being incorrectly initialized after
// coming out of suspend.  This issue is particularly affecting android M, which
// uses suspend ("doze mode") quite aggressively. This code detects that
// situation and makes single-stepping work by doing all the step operations on
// the boot cpu.
//
// The underlying issue has been fixed in android N and linux 4.4. This code can
// be removed once these systems become obsolete.

#if defined(__arm64__) || defined(__aarch64__)
class SingleStepWorkaround {
  ::pid_t m_tid;
  cpu_set_t m_original_set;

  SingleStepWorkaround(const SingleStepWorkaround &) = delete;
  void operator=(const SingleStepWorkaround &) = delete;

public:
  SingleStepWorkaround(::pid_t tid, cpu_set_t original_set)
      : m_tid(tid), m_original_set(original_set) {}
  ~SingleStepWorkaround();

  static std::unique_ptr<SingleStepWorkaround> Get(::pid_t tid);
};
#else
class SingleStepWorkaround {
public:
  static std::unique_ptr<SingleStepWorkaround> Get(::pid_t tid) {
    return nullptr;
  }
};
#endif

} // end namespace process_linux
} // end namespace lldb_private

#endif // #ifndef liblldb_SingleStepCheck_H_
