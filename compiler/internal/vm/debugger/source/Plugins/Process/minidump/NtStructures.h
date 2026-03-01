#ifndef LLDB_SOURCE_PLUGINS_PROCESS_MINIDUMP_NTSTRUCTURES_H

#define LLDB_SOURCE_PLUGINS_PROCESS_MINIDUMP_NTSTRUCTURES_H

//===-- NtStructures.h ------------------------------------------*- C++ -*-===//
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

#ifndef liblldb_Plugins_Process_Minidump_NtStructures_h_
#define liblldb_Plugins_Process_Minidump_NtStructures_h_

#include "llvm/Support/Endian.h"

namespace lldb_private {

namespace minidump {

// This describes the layout of a TEB (Thread Environment Block) for a 64-bit
// process.  It's adapted from the 32-bit TEB in winternl.h.  Currently, we care
// only about the position of the tls_slots.
struct TEB64 {
  llvm::support::ulittle64_t reserved1[12];
  llvm::support::ulittle64_t process_environment_block;
  llvm::support::ulittle64_t reserved2[399];
  uint8_t reserved3[1952];
  llvm::support::ulittle64_t tls_slots[64];
  uint8_t reserved4[8];
  llvm::support::ulittle64_t reserved5[26];
  llvm::support::ulittle64_t reserved_for_ole; // Windows 2000 only
  llvm::support::ulittle64_t reserved6[4];
  llvm::support::ulittle64_t tls_expansion_slots;
};

#endif // liblldb_Plugins_Process_Minidump_NtStructures_h_
} // namespace minidump
} // namespace lldb_private

#endif
