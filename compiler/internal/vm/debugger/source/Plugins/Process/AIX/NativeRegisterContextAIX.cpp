//===---- NativeRegisterContextAIX.cpp ------------------------------------===//
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

#include "NativeRegisterContextAIX.h"
#include "Plugins/Process/AIX/NativeProcessAIX.h"

using namespace lldb_private;
using namespace lldb_private::process_aix;

lldb::ByteOrder NativeRegisterContextAIX::GetByteOrder() const {
  return lldb::eByteOrderInvalid;
}

Status NativeRegisterContextAIX::ReadRegisterRaw(uint32_t reg_index,
                                                 RegisterValue &reg_value) {
  return Status("unimplemented");
}

Status
NativeRegisterContextAIX::WriteRegisterRaw(uint32_t reg_index,
                                           const RegisterValue &reg_value) {
  return Status("unimplemented");
}

Status NativeRegisterContextAIX::ReadGPR() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::WriteGPR() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::ReadFPR() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::WriteFPR() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::ReadVMX() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::WriteVMX() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::ReadVSX() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::WriteVSX() { return Status("unimplemented"); }

Status NativeRegisterContextAIX::ReadRegisterSet(void *buf, size_t buf_size,
                                                 unsigned int regset) {
  return Status("unimplemented");
}

Status NativeRegisterContextAIX::WriteRegisterSet(void *buf, size_t buf_size,
                                                  unsigned int regset) {
  return Status("unimplemented");
}
