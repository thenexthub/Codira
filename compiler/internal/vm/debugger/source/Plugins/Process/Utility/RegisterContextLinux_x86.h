//===-- RegisterContextLinux_i386.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTLINUX_X86_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERCONTEXTLINUX_X86_H

#include "RegisterInfoInterface.h"

namespace lldb_private {

class RegisterContextLinux_x86 : public RegisterInfoInterface {
public:
  RegisterContextLinux_x86(const ArchSpec &target_arch,
                           RegisterInfo orig_ax_info)
      : RegisterInfoInterface(target_arch), m_orig_ax_info(orig_ax_info) {}

  const RegisterInfo &GetOrigAxInfo() const { return m_orig_ax_info; }

private:
  lldb_private::RegisterInfo m_orig_ax_info;
};

} // namespace lldb_private

#endif
