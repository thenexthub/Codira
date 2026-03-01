//===-- RegisterContextMach_arm.cpp ---------------------------------------===//
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

#if defined(__APPLE__)

#include "RegisterContextMach_arm.h"

#include <mach/mach_types.h>
#include <mach/thread_act.h>


using namespace lldb;
using namespace lldb_private;

RegisterContextMach_arm::RegisterContextMach_arm(Thread &thread,
                                                 uint32_t concrete_frame_idx)
    : RegisterContextDarwin_arm(thread, concrete_frame_idx) {}

RegisterContextMach_arm::~RegisterContextMach_arm() = default;

int RegisterContextMach_arm::DoReadGPR(lldb::tid_t tid, int flavor, GPR &gpr) {
  mach_msg_type_number_t count = GPRWordCount;
  return ::thread_get_state(tid, flavor, (thread_state_t)&gpr, &count);
}

int RegisterContextMach_arm::DoReadFPU(lldb::tid_t tid, int flavor, FPU &fpu) {
  mach_msg_type_number_t count = FPUWordCount;
  return ::thread_get_state(tid, flavor, (thread_state_t)&fpu, &count);
}

int RegisterContextMach_arm::DoReadEXC(lldb::tid_t tid, int flavor, EXC &exc) {
  mach_msg_type_number_t count = EXCWordCount;
  return ::thread_get_state(tid, flavor, (thread_state_t)&exc, &count);
}

int RegisterContextMach_arm::DoReadDBG(lldb::tid_t tid, int flavor, DBG &dbg) {
  mach_msg_type_number_t count = DBGWordCount;
  return ::thread_get_state(tid, flavor, (thread_state_t)&dbg, &count);
}

int RegisterContextMach_arm::DoWriteGPR(lldb::tid_t tid, int flavor,
                                        const GPR &gpr) {
  return ::thread_set_state(
      tid, flavor, reinterpret_cast<thread_state_t>(const_cast<GPR *>(&gpr)),
      GPRWordCount);
}

int RegisterContextMach_arm::DoWriteFPU(lldb::tid_t tid, int flavor,
                                        const FPU &fpu) {
  return ::thread_set_state(
      tid, flavor, reinterpret_cast<thread_state_t>(const_cast<FPU *>(&fpu)),
      FPUWordCount);
}

int RegisterContextMach_arm::DoWriteEXC(lldb::tid_t tid, int flavor,
                                        const EXC &exc) {
  return ::thread_set_state(
      tid, flavor, reinterpret_cast<thread_state_t>(const_cast<EXC *>(&exc)),
      EXCWordCount);
}

int RegisterContextMach_arm::DoWriteDBG(lldb::tid_t tid, int flavor,
                                        const DBG &dbg) {
  return ::thread_set_state(
      tid, flavor, reinterpret_cast<thread_state_t>(const_cast<DBG *>(&dbg)),
      DBGWordCount);
}

#endif
