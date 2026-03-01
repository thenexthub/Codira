//===-- RegisterContextPOSIXCore_powerpc.h ----------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_POWERPC_H
#define LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_POWERPC_H

#include "Plugins/Process/Utility/RegisterContextPOSIX_powerpc.h"
#include "Plugins/Process/elf-core/RegisterUtilities.h"
#include "lldb/Utility/DataExtractor.h"

class RegisterContextCorePOSIX_powerpc : public RegisterContextPOSIX_powerpc {
public:
  RegisterContextCorePOSIX_powerpc(
      lldb_private::Thread &thread,
      lldb_private::RegisterInfoInterface *register_info,
      const lldb_private::DataExtractor &gpregset,
      llvm::ArrayRef<lldb_private::CoreNote> notes);

  ~RegisterContextCorePOSIX_powerpc() override;

  bool ReadRegister(const lldb_private::RegisterInfo *reg_info,
                    lldb_private::RegisterValue &value) override;

  bool WriteRegister(const lldb_private::RegisterInfo *reg_info,
                     const lldb_private::RegisterValue &value) override;

  bool ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  bool WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  bool HardwareSingleStep(bool enable) override;

protected:
  bool ReadGPR() override;

  bool ReadFPR() override;

  bool ReadVMX() override;

  bool WriteGPR() override;

  bool WriteFPR() override;

  bool WriteVMX() override;

private:
  lldb::DataBufferSP m_gpr_buffer;
  lldb::DataBufferSP m_fpr_buffer;
  lldb::DataBufferSP m_vec_buffer;
  lldb_private::DataExtractor m_gpr;
  lldb_private::DataExtractor m_fpr;
  lldb_private::DataExtractor m_vec;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_POWERPC_H
