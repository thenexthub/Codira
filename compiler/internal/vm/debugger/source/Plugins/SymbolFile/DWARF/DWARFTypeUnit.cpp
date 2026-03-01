//===-- DWARFTypeUnit.cpp -------------------------------------------------===//
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

#include "DWARFTypeUnit.h"

#include "SymbolFileDWARF.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

void DWARFTypeUnit::Dump(Stream *s) const {
  s->Format("{0:x16}: Type Unit: length = {1:x8}, version = {2:x4}, "
            "abbr_offset = {3:x8}, addr_size = {4:x2} (next CU at "
            "[{5:x16}])\n",
            GetOffset(), (uint32_t)GetLength(), GetVersion(),
            (uint32_t)GetAbbrevOffset(), GetAddressByteSize(),
            GetNextUnitOffset());
}
