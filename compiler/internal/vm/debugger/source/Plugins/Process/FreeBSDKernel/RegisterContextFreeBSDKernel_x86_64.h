//===-- RegisterContextFreeBSDKernel_x86_64.h -------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_REGISTERCONTEXTFREEBSDKERNEL_X86_64_H
#define LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_REGISTERCONTEXTFREEBSDKERNEL_X86_64_H

#include "Plugins/Process/Utility/RegisterContextPOSIX_x86.h"
#include "Plugins/Process/elf-core/RegisterUtilities.h"

class RegisterContextFreeBSDKernel_x86_64 : public RegisterContextPOSIX_x86 {
public:
  RegisterContextFreeBSDKernel_x86_64(
      lldb_private::Thread &thread,
      lldb_private::RegisterInfoInterface *register_info,
      lldb::addr_t pcb_addr);

  bool ReadRegister(const lldb_private::RegisterInfo *reg_info,
                    lldb_private::RegisterValue &value) override;

  bool WriteRegister(const lldb_private::RegisterInfo *reg_info,
                     const lldb_private::RegisterValue &value) override;

protected:
  bool ReadGPR() override;

  bool ReadFPR() override;

  bool WriteGPR() override;

  bool WriteFPR() override;

private:
  lldb::addr_t m_pcb_addr;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_REGISTERCONTEXTFREEBSDKERNEL_X86_64_H
