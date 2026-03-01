//===-- RegisterContextPOSIXCore_riscv32.h ----------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_RISCV32_H
#define LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_RISCV32_H

#include "Plugins/Process/Utility/RegisterContextPOSIX_riscv32.h"
#include "Plugins/Process/Utility/RegisterInfoPOSIX_riscv32.h"
#include "Plugins/Process/elf-core/RegisterUtilities.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/RegisterValue.h"

#include <memory>

class RegisterContextCorePOSIX_riscv32 : public RegisterContextPOSIX_riscv32 {
public:
  static std::unique_ptr<RegisterContextCorePOSIX_riscv32>
  Create(lldb_private::Thread &thread, const lldb_private::ArchSpec &arch,
         const lldb_private::DataExtractor &gpregset,
         llvm::ArrayRef<lldb_private::CoreNote> notes);

  ~RegisterContextCorePOSIX_riscv32() override;

  bool ReadRegister(const lldb_private::RegisterInfo *reg_info,
                    lldb_private::RegisterValue &value) override;

  bool WriteRegister(const lldb_private::RegisterInfo *reg_info,
                     const lldb_private::RegisterValue &value) override;

protected:
  RegisterContextCorePOSIX_riscv32(
      lldb_private::Thread &thread,
      std::unique_ptr<RegisterInfoPOSIX_riscv32> register_info,
      const lldb_private::DataExtractor &gpregset,
      llvm::ArrayRef<lldb_private::CoreNote> notes);

  bool ReadGPR() override;

  bool ReadFPR() override;

  bool WriteGPR() override;

  bool WriteFPR() override;

private:
  lldb_private::DataExtractor m_gpr;
  lldb_private::DataExtractor m_fpr;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_RISCV32_H
