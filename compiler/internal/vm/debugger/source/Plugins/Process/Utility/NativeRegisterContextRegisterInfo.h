//===-- NativeRegisterContextRegisterInfo.h ---------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_NATIVEREGISTERCONTEXTREGISTERINFO_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_NATIVEREGISTERCONTEXTREGISTERINFO_H

#include <memory>

#include "RegisterInfoInterface.h"
#include "lldb/Host/common/NativeRegisterContext.h"

namespace lldb_private {
class NativeRegisterContextRegisterInfo : public NativeRegisterContext {
public:
  ///
  /// Construct a NativeRegisterContextRegisterInfo, taking ownership
  /// of the register_info_interface pointer.
  ///
  NativeRegisterContextRegisterInfo(
      NativeThreadProtocol &thread,
      RegisterInfoInterface *register_info_interface);

  uint32_t GetRegisterCount() const override;

  uint32_t GetUserRegisterCount() const override;

  const RegisterInfo *GetRegisterInfoAtIndex(uint32_t reg_index) const override;

  const RegisterInfoInterface &GetRegisterInfoInterface() const;

protected:
  std::unique_ptr<RegisterInfoInterface> m_register_info_interface_up;
};
}
#endif
