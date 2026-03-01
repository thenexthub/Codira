//===-- NativeRegisterContextDBReg_loongarch.cpp --------------------------===//
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

#include "NativeRegisterContextDBReg_loongarch.h"

#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/RegisterValue.h"

using namespace lldb_private;

uint32_t
NativeRegisterContextDBReg_loongarch::GetWatchpointSize(uint32_t wp_index) {
  Log *log = GetLog(LLDBLog::Watchpoints);
  LLDB_LOG(log, "wp_index: {0}", wp_index);

  switch ((m_hwp_regs[wp_index].control >> 10) & 0x3) {
  case 0x0:
    return 8;
  case 0x1:
    return 4;
  case 0x2:
    return 2;
  case 0x3:
    return 1;
  default:
    return 0;
  }
}

std::optional<NativeRegisterContextDBReg::WatchpointDetails>
NativeRegisterContextDBReg_loongarch::AdjustWatchpoint(
    const WatchpointDetails &details) {
  // LoongArch only needs to check the size; it does not need to check the
  // address.
  size_t size = details.size;
  if (size != 1 && size != 2 && size != 4 && size != 8)
    return std::nullopt;

  return details;
}

uint32_t
NativeRegisterContextDBReg_loongarch::MakeBreakControlValue(size_t size) {
  // Return encoded hardware breakpoint control value.
  return m_hw_dbg_enable_bit;
}

uint32_t NativeRegisterContextDBReg_loongarch::MakeWatchControlValue(
    size_t size, uint32_t watch_flags) {
  // Encoding hardware watchpoint control value.
  // Size encoded:
  // case 1 : 0b11
  // case 2 : 0b10
  // case 4 : 0b01
  // case 8 : 0b00
  size_t encoded_size = (3 - llvm::Log2_32(size)) << 10;

  return m_hw_dbg_enable_bit | encoded_size | (watch_flags << 8);
}
