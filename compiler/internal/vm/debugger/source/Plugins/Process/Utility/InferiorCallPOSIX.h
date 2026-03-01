//===-- InferiorCallPOSIX.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_INFERIORCALLPOSIX_H
#define LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_INFERIORCALLPOSIX_H

// Inferior execution of POSIX functions.

#include "lldb/lldb-types.h"

namespace lldb_private {

class Process;

enum MmapProt {
  eMmapProtNone = 0,
  eMmapProtExec = 1,
  eMmapProtRead = 2,
  eMmapProtWrite = 4
};

bool InferiorCallMmap(Process *proc, lldb::addr_t &allocated_addr,
                      lldb::addr_t addr, lldb::addr_t length, unsigned prot,
                      unsigned flags, lldb::addr_t fd, lldb::addr_t offset);

bool InferiorCallMunmap(Process *proc, lldb::addr_t addr, lldb::addr_t length);

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_UTILITY_INFERIORCALLPOSIX_H
