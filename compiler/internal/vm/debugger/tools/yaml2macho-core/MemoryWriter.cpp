//===-- MemoryWriter.cpp --------------------------------------------------===//
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

#include "MemoryWriter.h"
#include "CoreSpec.h"
#include "Utility.h"
#include "llvm/BinaryFormat/MachO.h"

void create_lc_segment_cmd(const CoreSpec &spec, std::vector<uint8_t> &cmds,
                           const MemoryRegion &memory, off_t data_offset) {
  if (spec.wordsize == 8) {
    // Add the bytes for a segment_command_64 from <mach-o/loader.h>
    add_uint32(cmds, llvm::MachO::LC_SEGMENT_64);
    add_uint32(cmds, sizeof(struct llvm::MachO::segment_command_64));
    for (int i = 0; i < 16; i++)
      cmds.push_back(0);
    add_uint64(cmds, memory.addr); // segment_command_64.vmaddr
    add_uint64(cmds, memory.size); // segment_command_64.vmsize
    add_uint64(cmds, data_offset); // segment_command_64.fileoff
    add_uint64(cmds, memory.size); // segment_command_64.filesize
  } else {
    // Add the bytes for a segment_command from <mach-o/loader.h>
    add_uint32(cmds, llvm::MachO::LC_SEGMENT);
    add_uint32(cmds, sizeof(struct llvm::MachO::segment_command));
    for (int i = 0; i < 16; i++)
      cmds.push_back(0);
    add_uint32(cmds, memory.addr); // segment_command_64.vmaddr
    add_uint32(cmds, memory.size); // segment_command_64.vmsize
    add_uint32(cmds, data_offset); // segment_command_64.fileoff
    add_uint32(cmds, memory.size); // segment_command_64.filesize
  }
  add_uint32(cmds, 3); // segment_command_64.maxprot
  add_uint32(cmds, 3); // segment_command_64.initprot
  add_uint32(cmds, 0); // segment_command_64.nsects
  add_uint32(cmds, 0); // segment_command_64.flags
}

void create_memory_bytes(const CoreSpec &spec, const MemoryRegion &memory,
                         std::vector<uint8_t> &buf) {
  if (memory.type == MemoryType::UInt8)
    for (uint8_t byte : memory.bytes)
      buf.push_back(byte);

  if (memory.type == MemoryType::UInt32)
    for (uint32_t word : memory.words)
      add_uint32(buf, word);

  if (memory.type == MemoryType::UInt64)
    for (uint64_t word : memory.doublewords)
      add_uint64(buf, word);
}
