//===-- ValueObjectUpdater.cpp --------------------------------------------===//
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

#include "lldb/ValueObject/ValueObjectUpdater.h"

using namespace lldb_private;

ValueObjectUpdater::ValueObjectUpdater(lldb::ValueObjectSP in_valobj_sp) {
  if (!in_valobj_sp)
    return;
  // If the user passes in a value object that is dynamic or synthetic, then
  // water it down to the static type.
  m_root_valobj_sp = in_valobj_sp->GetQualifiedRepresentationIfAvailable(
      lldb::eNoDynamicValues, false);
}

lldb::ValueObjectSP ValueObjectUpdater::GetSP() {
  lldb::ProcessSP process_sp = GetProcessSP();
  if (!process_sp)
    return lldb::ValueObjectSP();

  const uint32_t current_stop_id = process_sp->GetLastNaturalStopID();
  if (current_stop_id == m_stop_id)
    return m_user_valobj_sp;

  m_stop_id = current_stop_id;

  if (!m_root_valobj_sp) {
    m_user_valobj_sp.reset();
    return m_root_valobj_sp;
  }

  m_user_valobj_sp = m_root_valobj_sp;

  lldb::ValueObjectSP dynamic_sp =
      m_user_valobj_sp->GetDynamicValue(lldb::eDynamicDontRunTarget);
  if (dynamic_sp)
    m_user_valobj_sp = dynamic_sp;

  lldb::ValueObjectSP synthetic_sp = m_user_valobj_sp->GetSyntheticValue();
  if (synthetic_sp)
    m_user_valobj_sp = synthetic_sp;

  return m_user_valobj_sp;
}

lldb::ProcessSP ValueObjectUpdater::GetProcessSP() const {
  if (m_root_valobj_sp)
    return m_root_valobj_sp->GetProcessSP();
  return lldb::ProcessSP();
}
