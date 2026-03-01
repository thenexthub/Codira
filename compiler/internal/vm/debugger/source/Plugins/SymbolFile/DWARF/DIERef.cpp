//===-- DIERef.cpp --------------------------------------------------------===//
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

#include "DIERef.h"
#include "lldb/Utility/DataEncoder.h"
#include "lldb/Utility/DataExtractor.h"
#include "llvm/Support/Format.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

void llvm::format_provider<DIERef>::format(const DIERef &ref, raw_ostream &OS,
                                           StringRef Style) {
  if (ref.file_index())
    OS << format_hex_no_prefix(*ref.file_index(), 8) << "/";
  OS << (ref.section() == DIERef::DebugInfo ? "INFO" : "TYPE");
  OS << "/" << format_hex_no_prefix(ref.die_offset(), 8);
}

std::optional<DIERef> DIERef::Decode(const DataExtractor &data,
                                     lldb::offset_t *offset_ptr) {
  DIERef die_ref(data.GetU64(offset_ptr));

  // DIE offsets can't be zero and if we fail to decode something from data,
  // it will return 0
  if (!die_ref.die_offset())
    return std::nullopt;

  return die_ref;
}

void DIERef::Encode(DataEncoder &encoder) const { encoder.AppendU64(get_id()); }
