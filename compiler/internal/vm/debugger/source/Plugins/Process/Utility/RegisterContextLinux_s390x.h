//===-- RegisterContextLinux_s390x.h ----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTLINUX_S390X_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTLINUX_S390X_H

#include "RegisterInfoInterface.h"

class RegisterContextLinux_s390x : public lldb_private::RegisterInfoInterface {
public:
  RegisterContextLinux_s390x(const lldb_private::ArchSpec &target_arch);

  size_t GetGPRSize() const override;

  const lldb_private::RegisterInfo *GetRegisterInfo() const override;

  uint32_t GetRegisterCount() const override;

  uint32_t GetUserRegisterCount() const override;

private:
  const lldb_private::RegisterInfo *m_register_info_p;
  uint32_t m_register_info_count;
  uint32_t m_user_register_count;
};

#endif
