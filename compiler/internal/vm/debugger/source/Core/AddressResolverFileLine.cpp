//===-- AddressResolverFileLine.cpp ---------------------------------------===//
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

#include "lldb/Core/AddressResolverFileLine.h"

#include "lldb/Core/Address.h"
#include "lldb/Core/AddressRange.h"
#include "lldb/Symbol/CompileUnit.h"
#include "lldb/Symbol/LineEntry.h"
#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StreamString.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-types.h"

#include <cinttypes>
#include <vector>

using namespace lldb;
using namespace lldb_private;

// AddressResolverFileLine:
AddressResolverFileLine::AddressResolverFileLine(
    SourceLocationSpec location_spec)
    : AddressResolver(), m_src_location_spec(location_spec) {}

AddressResolverFileLine::~AddressResolverFileLine() = default;

Searcher::CallbackReturn
AddressResolverFileLine::SearchCallback(SearchFilter &filter,
                                        SymbolContext &context, Address *addr) {
  SymbolContextList sc_list;
  CompileUnit *cu = context.comp_unit;

  Log *log = GetLog(LLDBLog::Breakpoints);

  // TODO: Handle SourceLocationSpec column information
  cu->ResolveSymbolContext(m_src_location_spec, eSymbolContextEverything,
                           sc_list);
  for (const SymbolContext &sc : sc_list) {
    Address line_start = sc.line_entry.range.GetBaseAddress();
    addr_t byte_size = sc.line_entry.range.GetByteSize();
    if (line_start.IsValid()) {
      AddressRange new_range(line_start, byte_size);
      m_address_ranges.push_back(new_range);
    } else {
      LLDB_LOGF(log,
                "error: Unable to resolve address at file address 0x%" PRIx64
                " for %s:%d\n",
                line_start.GetFileAddress(),
                m_src_location_spec.GetFileSpec().GetFilename().AsCString(
                    "<Unknown>"),
                m_src_location_spec.GetLine().value_or(0));
    }
  }
  return Searcher::eCallbackReturnContinue;
}

lldb::SearchDepth AddressResolverFileLine::GetDepth() {
  return lldb::eSearchDepthCompUnit;
}

void AddressResolverFileLine::GetDescription(Stream *s) {
  s->Printf(
      "File and line address - file: \"%s\" line: %u",
      m_src_location_spec.GetFileSpec().GetFilename().AsCString("<Unknown>"),
      m_src_location_spec.GetLine().value_or(0));
}
