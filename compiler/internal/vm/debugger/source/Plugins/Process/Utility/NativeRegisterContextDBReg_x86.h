//===-- NativeRegisterContextDBReg_x86.h ------------------------*- C++ -*-===//
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

#ifndef lldb_NativeRegisterContextDBReg_x86_h
#define lldb_NativeRegisterContextDBReg_x86_h

#include "Plugins/Process/Utility/NativeRegisterContextRegisterInfo.h"

namespace lldb_private {

class NativeRegisterContextDBReg_x86
    : public virtual NativeRegisterContextRegisterInfo {
public:
  // NB: This constructor is here only because gcc<=6.5 requires a virtual base
  // class initializer on abstract class (even though it is never used). It can
  // be deleted once we move to gcc>=7.0.
  NativeRegisterContextDBReg_x86(NativeThreadProtocol &thread)
      : NativeRegisterContextRegisterInfo(thread, nullptr) {}

  Status IsWatchpointHit(uint32_t wp_index, bool &is_hit) override;

  Status GetWatchpointHitIndex(uint32_t &wp_index,
                               lldb::addr_t trap_addr) override;

  Status IsWatchpointVacant(uint32_t wp_index, bool &is_vacant) override;

  bool ClearHardwareWatchpoint(uint32_t wp_index) override;

  Status ClearWatchpointHit(uint32_t wp_index) override;

  Status ClearAllHardwareWatchpoints() override;

  Status SetHardwareWatchpointWithIndex(lldb::addr_t addr, size_t size,
                                        uint32_t watch_flags,
                                        uint32_t wp_index);

  uint32_t SetHardwareWatchpoint(lldb::addr_t addr, size_t size,
                                 uint32_t watch_flags) override;

  lldb::addr_t GetWatchpointAddress(uint32_t wp_index) override;

  uint32_t NumSupportedHardwareWatchpoints() override;

  virtual const RegisterInfo *GetDR(int num) const;
};

} // namespace lldb_private

#endif // #ifndef lldb_NativeRegisterContextDBReg_x86_h
