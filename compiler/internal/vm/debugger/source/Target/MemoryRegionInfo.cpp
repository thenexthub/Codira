//===-- MemoryRegionInfo.cpp ----------------------------------------------===//
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

#include "lldb/Target/MemoryRegionInfo.h"

using namespace lldb_private;

llvm::raw_ostream &lldb_private::operator<<(llvm::raw_ostream &OS,
                                            const MemoryRegionInfo &Info) {
  return OS << llvm::formatv("MemoryRegionInfo([{0}, {1}), {2:r}{3:w}{4:x}, "
                             "{5}, `{6}`, {7}, {8}, {9}, {10}, {11})",
                             Info.GetRange().GetRangeBase(),
                             Info.GetRange().GetRangeEnd(), Info.GetReadable(),
                             Info.GetWritable(), Info.GetExecutable(),
                             Info.GetMapped(), Info.GetName(), Info.GetFlash(),
                             Info.GetBlocksize(), Info.GetMemoryTagged(),
                             Info.IsStackMemory(), Info.IsShadowStack());
}

void llvm::format_provider<MemoryRegionInfo::OptionalBool>::format(
    const MemoryRegionInfo::OptionalBool &B, raw_ostream &OS,
    StringRef Options) {
  assert(Options.size() <= 1);
  bool Empty = Options.empty();
  switch (B) {
  case lldb_private::MemoryRegionInfo::eNo:
    OS << (Empty ? "no" : "-");
    return;
  case lldb_private::MemoryRegionInfo::eYes:
    OS << (Empty ? "yes" : Options);
    return;
  case lldb_private::MemoryRegionInfo::eDontKnow:
    OS << (Empty ? "don't know" : "?");
    return;
  }
}
