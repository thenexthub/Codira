//===- Formatters.cpp -----------------------------------------------------===//
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

#include "vm/core/DebugInfo/CodeView/Formatters.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/CodeView/GUID.h"
#include "vm/core/Support/Endian.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::codeview::detail;

GuidAdapter::GuidAdapter(StringRef Guid)
    : FormatAdapter(ArrayRef(Guid.bytes_begin(), Guid.bytes_end())) {}

GuidAdapter::GuidAdapter(ArrayRef<uint8_t> Guid)
    : FormatAdapter(std::move(Guid)) {}

// From https://docs.microsoft.com/en-us/windows/win32/msi/guid documentation:
// The GUID data type is a text string representing a Class identifier (ID).
// All GUIDs must be authored in uppercase.
// The valid format for a GUID is {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} where
// X is a hex digit (0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F).
//
// The individual string components must be padded to comply with the specific
// lengths of {8-4-4-4-12} characters.
// The toolchain-yaml2obj tool checks that a GUID follow that format:
// - the total length to be 38 (including the curly braces.
// - there is a dash at the positions: 8, 13, 18 and 23.
void GuidAdapter::format(raw_ostream &Stream, StringRef Style) {
  assert(Item.size() == 16 && "Expected 16-byte GUID");
  struct MSGuid {
    support::ulittle32_t Data1;
    support::ulittle16_t Data2;
    support::ulittle16_t Data3;
    support::ubig64_t Data4;
  };
  const MSGuid *G = reinterpret_cast<const MSGuid *>(Item.data());
  Stream
      << '{' << format_hex_no_prefix(G->Data1, 8, /*Upper=*/true)
      << '-' << format_hex_no_prefix(G->Data2, 4, /*Upper=*/true)
      << '-' << format_hex_no_prefix(G->Data3, 4, /*Upper=*/true)
      << '-' << format_hex_no_prefix(G->Data4 >> 48, 4, /*Upper=*/true) << '-'
      << format_hex_no_prefix(G->Data4 & ((1ULL << 48) - 1), 12, /*Upper=*/true)
      << '}';
}

raw_ostream &toolchain::codeview::operator<<(raw_ostream &OS, const GUID &Guid) {
  codeview::detail::GuidAdapter A(Guid.Guid);
  A.format(OS, "");
  return OS;
}
