//===-- RegisterContextKDP_x86_64.cpp -------------------------------------===//
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

#include "RegisterContextKDP_x86_64.h"
#include "ProcessKDP.h"
#include "ThreadKDP.h"

using namespace lldb;
using namespace lldb_private;

RegisterContextKDP_x86_64::RegisterContextKDP_x86_64(
    ThreadKDP &thread, uint32_t concrete_frame_idx)
    : RegisterContextDarwin_x86_64(thread, concrete_frame_idx),
      m_kdp_thread(thread) {}

RegisterContextKDP_x86_64::~RegisterContextKDP_x86_64() = default;

int RegisterContextKDP_x86_64::DoReadGPR(lldb::tid_t tid, int flavor,
                                         GPR &gpr) {
  ProcessSP process_sp(CalculateProcess());
  if (process_sp) {
    Status error;
    if (static_cast<ProcessKDP *>(process_sp.get())
            ->GetCommunication()
            .SendRequestReadRegisters(tid, GPRRegSet, &gpr, sizeof(gpr),
                                      error)) {
      if (error.Success())
        return 0;
    }
  }
  return -1;
}

int RegisterContextKDP_x86_64::DoReadFPU(lldb::tid_t tid, int flavor,
                                         FPU &fpu) {
  ProcessSP process_sp(CalculateProcess());
  if (process_sp) {
    Status error;
    if (static_cast<ProcessKDP *>(process_sp.get())
            ->GetCommunication()
            .SendRequestReadRegisters(tid, FPURegSet, &fpu, sizeof(fpu),
                                      error)) {
      if (error.Success())
        return 0;
    }
  }
  return -1;
}

int RegisterContextKDP_x86_64::DoReadEXC(lldb::tid_t tid, int flavor,
                                         EXC &exc) {
  ProcessSP process_sp(CalculateProcess());
  if (process_sp) {
    Status error;
    if (static_cast<ProcessKDP *>(process_sp.get())
            ->GetCommunication()
            .SendRequestReadRegisters(tid, EXCRegSet, &exc, sizeof(exc),
                                      error)) {
      if (error.Success())
        return 0;
    }
  }
  return -1;
}

int RegisterContextKDP_x86_64::DoWriteGPR(lldb::tid_t tid, int flavor,
                                          const GPR &gpr) {
  ProcessSP process_sp(CalculateProcess());
  if (process_sp) {
    Status error;
    if (static_cast<ProcessKDP *>(process_sp.get())
            ->GetCommunication()
            .SendRequestWriteRegisters(tid, GPRRegSet, &gpr, sizeof(gpr),
                                       error)) {
      if (error.Success())
        return 0;
    }
  }
  return -1;
}

int RegisterContextKDP_x86_64::DoWriteFPU(lldb::tid_t tid, int flavor,
                                          const FPU &fpu) {
  ProcessSP process_sp(CalculateProcess());
  if (process_sp) {
    Status error;
    if (static_cast<ProcessKDP *>(process_sp.get())
            ->GetCommunication()
            .SendRequestWriteRegisters(tid, FPURegSet, &fpu, sizeof(fpu),
                                       error)) {
      if (error.Success())
        return 0;
    }
  }
  return -1;
}

int RegisterContextKDP_x86_64::DoWriteEXC(lldb::tid_t tid, int flavor,
                                          const EXC &exc) {
  ProcessSP process_sp(CalculateProcess());
  if (process_sp) {
    Status error;
    if (static_cast<ProcessKDP *>(process_sp.get())
            ->GetCommunication()
            .SendRequestWriteRegisters(tid, EXCRegSet, &exc, sizeof(exc),
                                       error)) {
      if (error.Success())
        return 0;
    }
  }
  return -1;
}
