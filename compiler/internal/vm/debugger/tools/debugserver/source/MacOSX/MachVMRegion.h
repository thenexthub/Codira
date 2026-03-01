//===-- MachVMRegion.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_MACHVMREGION_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_MACHVMREGION_H

#include "DNBDefs.h"
#include "DNBError.h"
#include <mach/mach.h>

class MachVMRegion {
public:
  MachVMRegion(task_t task);
  ~MachVMRegion();

  void Clear();
  mach_vm_address_t StartAddress() const { return m_start; }
  mach_vm_address_t EndAddress() const { return m_start + m_size; }
  mach_vm_size_t GetByteSize() const { return m_size; }
  mach_vm_address_t BytesRemaining(mach_vm_address_t addr) const {
    if (ContainsAddress(addr))
      return m_size - (addr - m_start);
    else
      return 0;
  }
  bool ContainsAddress(mach_vm_address_t addr) const {
    return addr >= StartAddress() && addr < EndAddress();
  }

  bool SetProtections(mach_vm_address_t addr, mach_vm_size_t size,
                      vm_prot_t prot);
  bool RestoreProtections();
  bool GetRegionForAddress(nub_addr_t addr);

  uint32_t GetDNBPermissions() const;
  std::vector<std::string> GetFlags() const;
  std::vector<std::string> GetMemoryTypes() const;

  const DNBError &GetError() { return m_err; }

protected:
#if defined(VM_REGION_SUBMAP_SHORT_INFO_COUNT_64)
  typedef vm_region_submap_short_info_data_64_t RegionInfo;
  enum { kRegionInfoSize = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64 };
#else
  typedef vm_region_submap_info_data_64_t RegionInfo;
  enum { kRegionInfoSize = VM_REGION_SUBMAP_INFO_COUNT_64 };
#endif

  task_t m_task;
  mach_vm_address_t m_addr;
  DNBError m_err;
  mach_vm_address_t m_start;
  mach_vm_size_t m_size;
  natural_t m_depth;
  RegionInfo m_data;
  vm_prot_t m_curr_protection; // The current, possibly modified protections.
                               // Original value is saved in m_data.protections.
  mach_vm_address_t
      m_protection_addr; // The start address at which protections were changed
  mach_vm_size_t
      m_protection_size; // The size of memory that had its protections changed
};

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_MACOSX_MACHVMREGION_H
