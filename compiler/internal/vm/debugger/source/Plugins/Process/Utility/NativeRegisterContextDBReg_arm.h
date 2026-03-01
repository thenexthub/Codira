//===-- NativeRegisterContextDBReg_arm.h ------------------------*- C++ -*-===//
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

#ifndef lldb_NativeRegisterContextDBReg_arm_h
#define lldb_NativeRegisterContextDBReg_arm_h

#include "Plugins/Process/Utility/NativeRegisterContextDBReg.h"

namespace lldb_private {

class NativeRegisterContextDBReg_arm : public NativeRegisterContextDBReg {
public:
  NativeRegisterContextDBReg_arm()
      : NativeRegisterContextDBReg(/*enable_bit=*/0x1U) {}

private:
  uint32_t GetWatchpointSize(uint32_t wp_index) override;

  std::optional<WatchpointDetails>
  AdjustWatchpoint(const WatchpointDetails &details) override;

  BreakpointDetails AdjustBreakpoint(const BreakpointDetails &details) override;

  uint32_t MakeBreakControlValue(size_t size) override;

  uint32_t MakeWatchControlValue(size_t size, uint32_t watch_flags) override;

  bool ValidateBreakpoint(size_t size,
                          [[maybe_unused]] lldb::addr_t addr) override {
    // Break on 4 or 2 byte instructions.
    return size == 4 || size == 2;
  }
};

} // namespace lldb_private

#endif // #ifndef lldb_NativeRegisterContextDBReg_arm_h
