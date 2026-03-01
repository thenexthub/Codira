//===-- RegisterContextFreeBSD_powerpc.h -------------------------*- C++
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTFREEBSD_POWERPC_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTFREEBSD_POWERPC_H

#include "RegisterInfoInterface.h"

class RegisterContextFreeBSD_powerpc
    : public lldb_private::RegisterInfoInterface {
public:
  RegisterContextFreeBSD_powerpc(const lldb_private::ArchSpec &target_arch);
  ~RegisterContextFreeBSD_powerpc() override;

  size_t GetGPRSize() const override;

  const lldb_private::RegisterInfo *GetRegisterInfo() const override;

  uint32_t GetRegisterCount() const override;
};

class RegisterContextFreeBSD_powerpc32 : public RegisterContextFreeBSD_powerpc {
public:
  RegisterContextFreeBSD_powerpc32(const lldb_private::ArchSpec &target_arch);
  ~RegisterContextFreeBSD_powerpc32() override;

  size_t GetGPRSize() const override;

  const lldb_private::RegisterInfo *GetRegisterInfo() const override;

  uint32_t GetRegisterCount() const override;
};

class RegisterContextFreeBSD_powerpc64 : public RegisterContextFreeBSD_powerpc {
public:
  RegisterContextFreeBSD_powerpc64(const lldb_private::ArchSpec &target_arch);
  ~RegisterContextFreeBSD_powerpc64() override;

  size_t GetGPRSize() const override;

  const lldb_private::RegisterInfo *GetRegisterInfo() const override;

  uint32_t GetRegisterCount() const override;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTFREEBSD_POWERPC_H
