//===-- MachVMMemory.h ------------------------------------------*- C++ -*-===//
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
//
//  Created by Greg Clayton on 6/26/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_MACHVMMEMORY_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_MACHVMMEMORY_H

#include "DNBDefs.h"
#include "DNBError.h"
#include <mach/mach.h>

class MachVMMemory {
public:
  MachVMMemory();
  ~MachVMMemory();
  nub_size_t Read(task_t task, nub_addr_t address, void *data,
                  nub_size_t data_count);
  nub_size_t Write(task_t task, nub_addr_t address, const void *data,
                   nub_size_t data_count);
  nub_size_t PageSize(task_t task);
  nub_bool_t GetMemoryRegionInfo(task_t task, nub_addr_t address,
                                 DNBRegionInfo *region_info);
  nub_bool_t GetMemoryTags(task_t task, nub_addr_t address, nub_size_t size,
                           std::vector<uint8_t> &tags);
  nub_bool_t GetMemoryProfile(DNBProfileDataScanType scanType, task_t task,
                              struct task_basic_info ti, cpu_type_t cputype,
                              nub_process_t pid, vm_statistics64_data_t &vminfo,
                              uint64_t &physical_memory, uint64_t &anonymous,
                              uint64_t &phys_footprint, uint64_t &memory_cap);

protected:
  nub_size_t MaxBytesLeftInPage(task_t task, nub_addr_t addr, nub_size_t count);

  nub_size_t WriteRegion(task_t task, const nub_addr_t address,
                         const void *data, const nub_size_t data_count);

  vm_size_t m_page_size;
  DNBError m_err;
};

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_MACHVMMEMORY_H
