//===-- ArmUnwindInfo.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_ARMUNWINDINFO_H
#define LLDB_SYMBOL_ARMUNWINDINFO_H

#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/RangeMap.h"
#include "lldb/lldb-private.h"
#include <vector>

/*
 * Unwind information reader and parser for the ARM exception handling ABI
 *
 * Implemented based on:
 *     Exception Handling ABI for the ARM Architecture
 *     Document number: ARM IHI 0038A (current through ABI r2.09)
 *     Date of Issue: 25th January 2007, reissued 30th November 2012
 *     http://infocenter.arm.com/help/topic/com.arm.doc.ihi0038a/IHI0038A_ehabi.pdf
 */

namespace lldb_private {

class ArmUnwindInfo {
public:
  ArmUnwindInfo(ObjectFile &objfile, lldb::SectionSP &arm_exidx,
                lldb::SectionSP &arm_extab);

  ~ArmUnwindInfo();

  bool GetUnwindPlan(Target &target, const Address &addr,
                     UnwindPlan &unwind_plan);

private:
  struct ArmExidxEntry {
    ArmExidxEntry(uint32_t f, lldb::addr_t a, uint32_t d);

    bool operator<(const ArmExidxEntry &other) const;

    uint32_t file_address;
    lldb::addr_t address;
    uint32_t data;
  };

  const uint8_t *GetExceptionHandlingTableEntry(const Address &addr);

  uint8_t GetByteAtOffset(const uint32_t *data, uint16_t offset) const;

  uint64_t GetULEB128(const uint32_t *data, uint16_t &offset,
                      uint16_t max_offset) const;

  const lldb::ByteOrder m_byte_order;
  lldb::SectionSP m_arm_exidx_sp; // .ARM.exidx section
  lldb::SectionSP m_arm_extab_sp; // .ARM.extab section
  DataExtractor m_arm_exidx_data; // .ARM.exidx section data
  DataExtractor m_arm_extab_data; // .ARM.extab section data
  std::vector<ArmExidxEntry> m_exidx_entries;
};

} // namespace lldb_private

#endif // LLDB_SYMBOL_ARMUNWINDINFO_H
