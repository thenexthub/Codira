//===-- RegisterContextKDP_arm64.h --------------------------------*- C++
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_MACOSX_KERNEL_REGISTERCONTEXTKDP_ARM64_H
#define LLDB_SOURCE_PLUGINS_PROCESS_MACOSX_KERNEL_REGISTERCONTEXTKDP_ARM64_H

#include "Plugins/Process/Utility/RegisterContextDarwin_arm64.h"

class ThreadKDP;

class RegisterContextKDP_arm64 : public RegisterContextDarwin_arm64 {
public:
  RegisterContextKDP_arm64(ThreadKDP &thread, uint32_t concrete_frame_idx);

  ~RegisterContextKDP_arm64() override;

protected:
  int DoReadGPR(lldb::tid_t tid, int flavor, GPR &gpr) override;

  int DoReadFPU(lldb::tid_t tid, int flavor, FPU &fpu) override;

  int DoReadEXC(lldb::tid_t tid, int flavor, EXC &exc) override;

  int DoReadDBG(lldb::tid_t tid, int flavor, DBG &dbg) override;

  int DoWriteGPR(lldb::tid_t tid, int flavor, const GPR &gpr) override;

  int DoWriteFPU(lldb::tid_t tid, int flavor, const FPU &fpu) override;

  int DoWriteEXC(lldb::tid_t tid, int flavor, const EXC &exc) override;

  int DoWriteDBG(lldb::tid_t tid, int flavor, const DBG &dbg) override;

  ThreadKDP &m_kdp_thread;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_MACOSX_KERNEL_REGISTERCONTEXTKDP_ARM64_H
