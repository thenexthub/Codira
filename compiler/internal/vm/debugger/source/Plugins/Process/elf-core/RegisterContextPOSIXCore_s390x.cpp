//===-- RegisterContextPOSIXCore_s390x.cpp --------------------------------===//
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

#include "RegisterContextPOSIXCore_s390x.h"

#include "lldb/Target/Thread.h"
#include "lldb/Utility/DataBufferHeap.h"
#include "lldb/Utility/RegisterValue.h"

#include <memory>

using namespace lldb_private;

RegisterContextCorePOSIX_s390x::RegisterContextCorePOSIX_s390x(
    Thread &thread, RegisterInfoInterface *register_info,
    const DataExtractor &gpregset, llvm::ArrayRef<CoreNote> notes)
    : RegisterContextPOSIX_s390x(thread, 0, register_info) {
  m_gpr_buffer = std::make_shared<DataBufferHeap>(gpregset.GetDataStart(),
                                                  gpregset.GetByteSize());
  m_gpr.SetData(m_gpr_buffer);
  m_gpr.SetByteOrder(gpregset.GetByteOrder());

  DataExtractor fpregset = getRegset(
      notes, register_info->GetTargetArchitecture().GetTriple(), FPR_Desc);
  m_fpr_buffer = std::make_shared<DataBufferHeap>(fpregset.GetDataStart(),
                                                  fpregset.GetByteSize());
  m_fpr.SetData(m_fpr_buffer);
  m_fpr.SetByteOrder(fpregset.GetByteOrder());
}

RegisterContextCorePOSIX_s390x::~RegisterContextCorePOSIX_s390x() = default;

bool RegisterContextCorePOSIX_s390x::ReadGPR() { return true; }

bool RegisterContextCorePOSIX_s390x::ReadFPR() { return true; }

bool RegisterContextCorePOSIX_s390x::WriteGPR() {
  assert(0);
  return false;
}

bool RegisterContextCorePOSIX_s390x::WriteFPR() {
  assert(0);
  return false;
}

bool RegisterContextCorePOSIX_s390x::ReadRegister(const RegisterInfo *reg_info,
                                                  RegisterValue &value) {
  const uint32_t reg = reg_info->kinds[lldb::eRegisterKindLLDB];
  if (reg == LLDB_INVALID_REGNUM)
    return false;

  if (IsGPR(reg)) {
    lldb::offset_t offset = reg_info->byte_offset;
    uint64_t v = m_gpr.GetMaxU64(&offset, reg_info->byte_size);
    if (offset == reg_info->byte_offset + reg_info->byte_size) {
      value.SetUInt(v, reg_info->byte_size);
      return true;
    }
  }

  if (IsFPR(reg)) {
    lldb::offset_t offset = reg_info->byte_offset;
    uint64_t v = m_fpr.GetMaxU64(&offset, reg_info->byte_size);
    if (offset == reg_info->byte_offset + reg_info->byte_size) {
      value.SetUInt(v, reg_info->byte_size);
      return true;
    }
  }

  return false;
}

bool RegisterContextCorePOSIX_s390x::ReadAllRegisterValues(
    lldb::WritableDataBufferSP &data_sp) {
  return false;
}

bool RegisterContextCorePOSIX_s390x::WriteRegister(const RegisterInfo *reg_info,
                                                   const RegisterValue &value) {
  return false;
}

bool RegisterContextCorePOSIX_s390x::WriteAllRegisterValues(
    const lldb::DataBufferSP &data_sp) {
  return false;
}

bool RegisterContextCorePOSIX_s390x::HardwareSingleStep(bool enable) {
  return false;
}
