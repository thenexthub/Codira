//===-- AuxVector.cpp -----------------------------------------------------===//
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

#include "AuxVector.h"
#include <optional>

AuxVector::AuxVector(const lldb_private::DataExtractor &data) {
  ParseAuxv(data);
}

void AuxVector::ParseAuxv(const lldb_private::DataExtractor &data) {
  lldb::offset_t offset = 0;
  const size_t value_type_size = data.GetAddressByteSize() * 2;
  while (data.ValidOffsetForDataOfSize(offset, value_type_size)) {
    // We're not reading an address but an int that could be 32 or 64 bit
    // depending on the address size, which is what GetAddress does.
    const uint64_t type = data.GetAddress(&offset);
    const uint64_t value = data.GetAddress(&offset);
    if (type == AUXV_AT_NULL)
      break;
    if (type == AUXV_AT_IGNORE)
      continue;

    m_auxv_entries[type] = value;
  }
}

std::optional<uint64_t>
AuxVector::GetAuxValue(enum EntryType entry_type) const {
  auto it = m_auxv_entries.find(static_cast<uint64_t>(entry_type));
  if (it != m_auxv_entries.end())
    return it->second;
  return std::nullopt;
}

void AuxVector::DumpToLog(lldb_private::Log *log) const {
  if (!log)
    return;

  log->PutCString("AuxVector: ");
  for (auto entry : m_auxv_entries) {
    LLDB_LOGF(log, "   %s [%" PRIu64 "]: %" PRIx64,
              GetEntryName(static_cast<EntryType>(entry.first)), entry.first,
              entry.second);
  }
}

const char *AuxVector::GetEntryName(EntryType type) const {
  const char *name = "AT_???";

#define ENTRY_NAME(_type)                                                      \
  _type:                                                                       \
  name = &#_type[5]
  switch (type) {
    case ENTRY_NAME(AUXV_AT_NULL);           break;
    case ENTRY_NAME(AUXV_AT_IGNORE);         break;
    case ENTRY_NAME(AUXV_AT_EXECFD);         break;
    case ENTRY_NAME(AUXV_AT_PHDR);           break;
    case ENTRY_NAME(AUXV_AT_PHENT);          break;
    case ENTRY_NAME(AUXV_AT_PHNUM);          break;
    case ENTRY_NAME(AUXV_AT_PAGESZ);         break;
    case ENTRY_NAME(AUXV_AT_BASE);           break;
    case ENTRY_NAME(AUXV_AT_FLAGS);          break;
    case ENTRY_NAME(AUXV_AT_ENTRY);          break;
    case ENTRY_NAME(AUXV_AT_NOTELF);         break;
    case ENTRY_NAME(AUXV_AT_UID);            break;
    case ENTRY_NAME(AUXV_AT_EUID);           break;
    case ENTRY_NAME(AUXV_AT_GID);            break;
    case ENTRY_NAME(AUXV_AT_EGID);           break;
    case ENTRY_NAME(AUXV_AT_CLKTCK);         break;
    case ENTRY_NAME(AUXV_AT_PLATFORM);       break;
    case ENTRY_NAME(AUXV_AT_HWCAP);          break;
    case ENTRY_NAME(AUXV_AT_FPUCW);          break;
    case ENTRY_NAME(AUXV_AT_DCACHEBSIZE);    break;
    case ENTRY_NAME(AUXV_AT_ICACHEBSIZE);    break;
    case ENTRY_NAME(AUXV_AT_UCACHEBSIZE);    break;
    case ENTRY_NAME(AUXV_AT_IGNOREPPC);      break;
    case ENTRY_NAME(AUXV_AT_SECURE);         break;
    case ENTRY_NAME(AUXV_AT_BASE_PLATFORM);  break;
    case ENTRY_NAME(AUXV_AT_RANDOM);         break;
    case ENTRY_NAME(AUXV_AT_HWCAP2);         break;
    case ENTRY_NAME(AUXV_AT_HWCAP3);         break;
    case ENTRY_NAME(AUXV_AT_EXECFN);         break;
    case ENTRY_NAME(AUXV_AT_SYSINFO);        break;
    case ENTRY_NAME(AUXV_AT_SYSINFO_EHDR);   break;
    case ENTRY_NAME(AUXV_AT_L1I_CACHESHAPE); break;
    case ENTRY_NAME(AUXV_AT_L1D_CACHESHAPE); break;
    case ENTRY_NAME(AUXV_AT_L2_CACHESHAPE);  break;
    case ENTRY_NAME(AUXV_AT_L3_CACHESHAPE);  break;
    case ENTRY_NAME(AUXV_FREEBSD_AT_HWCAP3);  break;
    case ENTRY_NAME(AUXV_FREEBSD_AT_HWCAP4);  break;
    }
#undef ENTRY_NAME

    return name;
}
