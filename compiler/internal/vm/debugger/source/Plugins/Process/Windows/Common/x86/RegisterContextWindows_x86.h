//===-- RegisterContextWindows_x86.h ----------------------------*- C++ -*-===//
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

#ifndef liblldb_RegisterContextWindows_x86_H_
#define liblldb_RegisterContextWindows_x86_H_

#if defined(__i386__) || defined(_M_IX86)

#include "RegisterContextWindows.h"
#include "lldb/lldb-forward.h"

namespace lldb_private {

class Thread;

class RegisterContextWindows_x86 : public RegisterContextWindows {
public:
  // Constructors and Destructors
  RegisterContextWindows_x86(Thread &thread, uint32_t concrete_frame_idx);

  virtual ~RegisterContextWindows_x86();

  // Subclasses must override these functions
  size_t GetRegisterCount() override;

  const RegisterInfo *GetRegisterInfoAtIndex(size_t reg) override;

  size_t GetRegisterSetCount() override;

  const RegisterSet *GetRegisterSet(size_t reg_set) override;

  bool ReadRegister(const RegisterInfo *reg_info,
                    RegisterValue &reg_value) override;

  bool WriteRegister(const RegisterInfo *reg_info,
                     const RegisterValue &reg_value) override;

private:
  bool ReadRegisterHelper(DWORD flags_required, const char *reg_name,
                          DWORD value, RegisterValue &reg_value) const;
};
}

#endif // defined(__i386__) || defined(_M_IX86)

#endif // #ifndef liblldb_RegisterContextWindows_x86_H_
