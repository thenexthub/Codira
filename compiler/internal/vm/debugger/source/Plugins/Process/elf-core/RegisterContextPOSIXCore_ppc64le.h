//===-- RegisterContextPOSIXCore_ppc64le.h ----------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_PPC64LE_H
#define LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_PPC64LE_H

#include "Plugins/Process/Utility/RegisterContextPOSIX_ppc64le.h"
#include "Plugins/Process/elf-core/RegisterUtilities.h"
#include "lldb/Utility/DataExtractor.h"

class RegisterContextCorePOSIX_ppc64le : public RegisterContextPOSIX_ppc64le {
public:
  RegisterContextCorePOSIX_ppc64le(
      lldb_private::Thread &thread,
      lldb_private::RegisterInfoInterface *register_info,
      const lldb_private::DataExtractor &gpregset,
      llvm::ArrayRef<lldb_private::CoreNote> notes);

  bool ReadRegister(const lldb_private::RegisterInfo *reg_info,
                    lldb_private::RegisterValue &value) override;

  bool WriteRegister(const lldb_private::RegisterInfo *reg_info,
                     const lldb_private::RegisterValue &value) override;

protected:
  size_t GetFPRSize() const;

  size_t GetVMXSize() const;

  size_t GetVSXSize() const;

private:
  lldb::DataBufferSP m_gpr_buffer;
  lldb::DataBufferSP m_fpr_buffer;
  lldb::DataBufferSP m_vmx_buffer;
  lldb::DataBufferSP m_vsx_buffer;
  lldb_private::DataExtractor m_gpr;
  lldb_private::DataExtractor m_fpr;
  lldb_private::DataExtractor m_vmx;
  lldb_private::DataExtractor m_vsx;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_ELF_CORE_REGISTERCONTEXTPOSIXCORE_PPC64LE_H
