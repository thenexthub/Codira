//===-- RegisterContextUnifiedCore.h --------------------------------------===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_REGISTERCONTEXT_UNIFIED_CORE_H
#define LLDB_SOURCE_PLUGINS_PROCESS_REGISTERCONTEXT_UNIFIED_CORE_H

#include <string>
#include <vector>

#include "lldb/Target/RegisterContext.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/StructuredData.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class RegisterContextUnifiedCore : public RegisterContext {
public:
  RegisterContextUnifiedCore(
      Thread &thread, uint32_t concrete_frame_idx,
      lldb::RegisterContextSP core_thread_regctx_sp,
      lldb_private::StructuredData::ObjectSP metadata_thread_registers);

  void InvalidateAllRegisters() override {};

  size_t GetRegisterCount() override;

  const lldb_private::RegisterInfo *GetRegisterInfoAtIndex(size_t reg) override;

  size_t GetRegisterSetCount() override;

  const lldb_private::RegisterSet *GetRegisterSet(size_t set) override;

  bool ReadRegister(const lldb_private::RegisterInfo *reg_info,
                    lldb_private::RegisterValue &value) override;

  bool WriteRegister(const lldb_private::RegisterInfo *reg_info,
                     const lldb_private::RegisterValue &value) override;

private:
  std::vector<lldb_private::RegisterSet> m_register_sets;
  std::vector<lldb_private::RegisterInfo> m_register_infos;
  /// For each register set, an array of register numbers included.
  std::map<size_t, std::vector<uint32_t>> m_regset_regnum_collection;
  /// Bytes of the register contents.
  std::vector<uint8_t> m_register_data;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_REGISTERCONTEXT_UNIFIED_CORE_H
