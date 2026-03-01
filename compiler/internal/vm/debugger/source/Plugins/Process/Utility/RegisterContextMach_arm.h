//===-- RegisterContextMach_arm.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTMACH_ARM_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTMACH_ARM_H

#include "RegisterContextDarwin_arm.h"

class RegisterContextMach_arm : public RegisterContextDarwin_arm {
public:
  RegisterContextMach_arm(lldb_private::Thread &thread,
                          uint32_t concrete_frame_idx);

  ~RegisterContextMach_arm() override;

protected:
  int DoReadGPR(lldb::tid_t tid, int flavor, GPR &gpr) override;

  int DoReadFPU(lldb::tid_t tid, int flavor, FPU &fpu) override;

  int DoReadEXC(lldb::tid_t tid, int flavor, EXC &exc) override;

  int DoReadDBG(lldb::tid_t tid, int flavor, DBG &dbg) override;

  int DoWriteGPR(lldb::tid_t tid, int flavor, const GPR &gpr) override;

  int DoWriteFPU(lldb::tid_t tid, int flavor, const FPU &fpu) override;

  int DoWriteEXC(lldb::tid_t tid, int flavor, const EXC &exc) override;

  int DoWriteDBG(lldb::tid_t tid, int flavor, const DBG &dbg) override;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTMACH_ARM_H
