//===-- RegisterContextDummy.h ----------------------------------------*- C++
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTDUMMY_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTDUMMY_H

#include <vector>

#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class RegisterContextDummy : public lldb_private::RegisterContext {
public:
  typedef std::shared_ptr<RegisterContextDummy> SharedPtr;

  RegisterContextDummy(Thread &thread, uint32_t concrete_frame_idx,
                       uint32_t address_byte_size);

  ~RegisterContextDummy() override;

  void InvalidateAllRegisters() override;

  size_t GetRegisterCount() override;

  const lldb_private::RegisterInfo *GetRegisterInfoAtIndex(size_t reg) override;

  size_t GetRegisterSetCount() override;

  const lldb_private::RegisterSet *GetRegisterSet(size_t reg_set) override;

  bool ReadRegister(const lldb_private::RegisterInfo *reg_info,
                    lldb_private::RegisterValue &value) override;

  bool WriteRegister(const lldb_private::RegisterInfo *reg_info,
                     const lldb_private::RegisterValue &value) override;

  bool ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  bool WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  uint32_t ConvertRegisterKindToRegisterNumber(lldb::RegisterKind kind,
                                               uint32_t num) override;

private:
  // For RegisterContextLLDB only

  lldb_private::RegisterSet m_reg_set0; // register set 0 (PC only)
  lldb_private::RegisterInfo m_pc_reg_info;

  RegisterContextDummy(const RegisterContextDummy &) = delete;
  const RegisterContextDummy &operator=(const RegisterContextDummy &) = delete;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTDUMMY_H
