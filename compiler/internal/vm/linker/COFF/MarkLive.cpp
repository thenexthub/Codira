//===- MarkLive.cpp -------------------------------------------------------===//
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

#include "COFFLinkerContext.h"
#include "Chunks.h"
#include "Symbols.h"
#include "lld/Common/Timer.h"
#include "llvm/Support/TimeProfiler.h"

namespace lld::coff {

// Set live bit on for each reachable chunk. Unmarked (unreachable)
// COMDAT chunks will be ignored by Writer, so they will be excluded
// from the final output.
void markLive(COFFLinkerContext &ctx) {
  llvm::TimeTraceScope timeScope("Mark live");
  ScopedTimer t(ctx.gcTimer);

  // We build up a worklist of sections which have been marked as live. We only
  // push into the worklist when we discover an unmarked section, and we mark
  // as we push, so sections never appear twice in the list.
  SmallVector<SectionChunk *, 256> worklist;

  // COMDAT section chunks are dead by default. Add non-COMDAT chunks. Do not
  // traverse DWARF sections. They are live, but they should not keep other
  // sections alive.
  for (Chunk *c : ctx.driver.getChunks())
    if (auto *sc = dyn_cast<SectionChunk>(c))
      if (sc->live && !sc->isDWARF())
        worklist.push_back(sc);

  auto enqueue = [&](SectionChunk *c) {
    if (c->live)
      return;
    c->live = true;
    worklist.push_back(c);
  };

  std::function<void(Symbol *)> addSym;

  auto addImportFile = [&](ImportFile *file) {
    file->live = true;
    if (file->impchkThunk && file->impchkThunk->exitThunk)
      addSym(file->impchkThunk->exitThunk);
  };

  addSym = [&](Symbol *s) {
    Defined *b = s->getDefined();
    if (!b)
      return;
    if (auto *sym = dyn_cast<DefinedRegular>(b)) {
      enqueue(sym->getChunk());
    } else if (auto *sym = dyn_cast<DefinedImportData>(b)) {
      addImportFile(sym->file);
    } else if (auto *sym = dyn_cast<DefinedImportThunk>(b)) {
      addImportFile(sym->wrappedSym->file);
      sym->getChunk()->live = true;
    }
  };

  // Add GC root chunks.
  for (Symbol *b : ctx.config.gcroot)
    addSym(b);

  while (!worklist.empty()) {
    SectionChunk *sc = worklist.pop_back_val();
    assert(sc->live && "We mark as live when pushing onto the worklist!");

    // Mark all symbols listed in the relocation table for this section.
    for (Symbol *b : sc->symbols())
      if (b)
        addSym(b);

    // Mark associative sections if any.
    for (SectionChunk &c : sc->children())
      enqueue(&c);

    // Mark EC entry thunks.
    if (Defined *entryThunk = sc->getEntryThunk())
      addSym(entryThunk);
  }
}
}
