//===-- RegisterContextLinuxCore_x86_64.h -----------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTLINUXCORE_X86_64_H
#define LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTLINUXCORE_X86_64_H

#include "Plugins/Process/elf-core/RegisterUtilities.h"
#include "RegisterContextPOSIXCore_x86_64.h"

class RegisterContextLinuxCore_x86_64 : public RegisterContextCorePOSIX_x86_64 {
public:
  RegisterContextLinuxCore_x86_64(
      lldb_private::Thread &thread,
      lldb_private::RegisterInfoInterface *register_info,
      const lldb_private::DataExtractor &gpregset,
      llvm::ArrayRef<lldb_private::CoreNote> notes);

  const lldb_private::RegisterSet *GetRegisterSet(size_t set) override;

  lldb_private::RegInfo &GetRegInfo() override;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTLINUXCORE_X86_64_H
