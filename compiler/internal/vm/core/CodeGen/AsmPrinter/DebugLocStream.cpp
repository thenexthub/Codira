//===- DebugLocStream.cpp - DWARF debug_loc stream --------------*- C++ -*-===//
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

#include "DebugLocStream.h"
#include "DwarfDebug.h"
#include "vm/core/CodeGen/AsmPrinter.h"

using namespace vm::core;

bool DebugLocStream::finalizeList(AsmPrinter &Asm) {
  if (Lists.back().EntryOffset == Entries.size()) {
    // Empty list.  Delete it.
    Lists.pop_back();
    return false;
  }

  // Real list.  Generate a label for it.
  Lists.back().Label = Asm.createTempSymbol("debug_loc");
  return true;
}

void DebugLocStream::finalizeEntry() {
  if (Entries.back().ByteOffset != DWARFBytes.size())
    return;

  // The last entry was empty.  Delete it.
  Comments.erase(Comments.begin() + Entries.back().CommentOffset,
                 Comments.end());
  Entries.pop_back();

  assert(Lists.back().EntryOffset <= Entries.size() &&
         "Popped off more entries than are in the list");
}

DebugLocStream::ListBuilder::~ListBuilder() {
  if (!Locs.finalizeList(Asm))
    return;
  V.emplace<Loc::Multi>(ListIndex, TagOffset);
}
