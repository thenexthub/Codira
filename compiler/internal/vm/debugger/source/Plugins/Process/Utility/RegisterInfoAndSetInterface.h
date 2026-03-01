//===-- RegisterInfoAndSetInterface.h ---------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOANDSETINTERFACE_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_REGISTERINFOANDSETINTERFACE_H

#include "RegisterInfoInterface.h"

#include "lldb/Utility/ArchSpec.h"
#include "lldb/lldb-private-types.h"
#include <vector>

namespace lldb_private {

class RegisterInfoAndSetInterface : public RegisterInfoInterface {
public:
  RegisterInfoAndSetInterface(const lldb_private::ArchSpec &target_arch)
      : RegisterInfoInterface(target_arch) {}

  virtual size_t GetFPRSize() const = 0;

  virtual const lldb_private::RegisterSet *
  GetRegisterSet(size_t reg_set) const = 0;

  virtual size_t GetRegisterSetCount() const = 0;

  virtual size_t GetRegisterSetFromRegisterIndex(uint32_t reg_index) const = 0;
};
} // namespace lldb_private

#endif
