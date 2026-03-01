//===-- RegisterInfoInterface.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOINTERFACE_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOINTERFACE_H

#include "lldb/Utility/ArchSpec.h"
#include "lldb/lldb-private-types.h"
#include <vector>

namespace lldb_private {

/// \class RegisterInfoInterface
///
/// RegisterInfo interface to patch RegisterInfo structure for archs.
class RegisterInfoInterface {
public:
  RegisterInfoInterface(const lldb_private::ArchSpec &target_arch)
      : m_target_arch(target_arch) {}
  virtual ~RegisterInfoInterface() = default;

  virtual size_t GetGPRSize() const = 0;

  virtual const lldb_private::RegisterInfo *GetRegisterInfo() const = 0;

  // Returns the number of registers including the user registers and the
  // lldb internal registers also
  virtual uint32_t GetRegisterCount() const = 0;

  // Returns the number of the user registers (excluding the registers
  // kept for lldb internal use only). Subclasses should override it if
  // they belongs to an architecture with lldb internal registers.
  virtual uint32_t GetUserRegisterCount() const { return GetRegisterCount(); }

  const lldb_private::ArchSpec &GetTargetArchitecture() const {
    return m_target_arch;
  }

private:
  lldb_private::ArchSpec m_target_arch;
};
} // namespace lldb_private

#endif
