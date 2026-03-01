//===----------------------------------------------------------------------===//
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
/// \file
/// Functions to emit the LC_SEGMENT load command, and to provide the bytes
/// that appear later in the corefile.
//===----------------------------------------------------------------------===//

#ifndef YAML2MACHOCOREFILE_MEMORYWRITER_H
#define YAML2MACHOCOREFILE_MEMORYWRITER_H

#include "CoreSpec.h"

#include <vector>

void create_lc_segment_cmd(const CoreSpec &spec, std::vector<uint8_t> &cmds,
                           const MemoryRegion &memory, off_t data_offset);

void create_memory_bytes(const CoreSpec &spec, const MemoryRegion &memory,
                         std::vector<uint8_t> &buf);

#endif
