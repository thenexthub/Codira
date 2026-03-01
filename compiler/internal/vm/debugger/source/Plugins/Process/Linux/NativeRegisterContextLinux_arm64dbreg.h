//===-- NativeRegisterContextLinux_arm64dbreg.h -----------------*- C++ -*-===//
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

// When debugging 32-bit processes, Arm64 lldb-server should use 64-bit ptrace
// interfaces. 32-bit ptrace interfaces should only be used by 32-bit server.
// These functions are split out to be reused in both 32-bit and 64-bit register
// context for 64-bit server.

#include "Plugins/Process/Linux/NativeProcessLinux.h"
#include "Plugins/Process/Utility/NativeRegisterContextDBReg.h"
#include "lldb/Utility/Status.h"

namespace lldb_private {
namespace process_linux {
namespace arm64 {

Status ReadHardwareDebugInfo(::pid_t tid, uint32_t &max_hwp_supported,
                             uint32_t &max_hbp_supported);

Status WriteHardwareDebugRegs(
    int hwbType, ::pid_t tid, uint32_t max_supported,
    const std::array<NativeRegisterContextDBReg::DREG, 16> &regs);

} // namespace arm64
} // namespace process_linux
} // namespace lldb_private
