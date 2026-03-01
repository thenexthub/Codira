//===-- NativeRegisterContextRegisterInfo.cpp -----------------------------===//
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

#include "NativeRegisterContextRegisterInfo.h"
#include "lldb/lldb-private-forward.h"
#include "lldb/lldb-types.h"

using namespace lldb_private;

NativeRegisterContextRegisterInfo::NativeRegisterContextRegisterInfo(
    NativeThreadProtocol &thread,
    RegisterInfoInterface *register_info_interface)
    : NativeRegisterContext(thread),
      m_register_info_interface_up(register_info_interface) {
  assert(register_info_interface && "null register_info_interface");
}

uint32_t NativeRegisterContextRegisterInfo::GetRegisterCount() const {
  return m_register_info_interface_up->GetRegisterCount();
}

uint32_t NativeRegisterContextRegisterInfo::GetUserRegisterCount() const {
  return m_register_info_interface_up->GetUserRegisterCount();
}

const RegisterInfo *NativeRegisterContextRegisterInfo::GetRegisterInfoAtIndex(
    uint32_t reg_index) const {
  if (reg_index <= GetRegisterCount())
    return m_register_info_interface_up->GetRegisterInfo() + reg_index;
  else
    return nullptr;
}

const RegisterInfoInterface &
NativeRegisterContextRegisterInfo::GetRegisterInfoInterface() const {
  return *m_register_info_interface_up;
}
