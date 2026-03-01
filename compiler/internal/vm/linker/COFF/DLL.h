//===- DLL.h ----------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_COFF_DLL_H
#define LLD_COFF_DLL_H

#include "Chunks.h"
#include "Symbols.h"

namespace lld::coff {

// Windows-specific.
// IdataContents creates all chunks for the DLL import table.
// You are supposed to call add() to add symbols and then
// call create() to populate the chunk vectors.
class IdataContents {
public:
  void add(DefinedImportData *sym) { imports.push_back(sym); }
  bool empty() { return imports.empty(); }

  void create(COFFLinkerContext &ctx);

  std::vector<DefinedImportData *> imports;
  std::vector<Chunk *> dirs;
  std::vector<Chunk *> lookups;
  std::vector<Chunk *> addresses;
  std::vector<Chunk *> hints;
  std::vector<Chunk *> dllNames;
  std::vector<Chunk *> auxIat;
  std::vector<Chunk *> auxIatCopy;
};

// Windows-specific.
// DelayLoadContents creates all chunks for the delay-load DLL import table.
class DelayLoadContents {
public:
  DelayLoadContents(COFFLinkerContext &ctx) : ctx(ctx) {}
  void add(DefinedImportData *sym) { imports.push_back(sym); }
  bool empty() { return imports.empty(); }
  void create();
  std::vector<Chunk *> getChunks();
  std::vector<Chunk *> getDataChunks();
  ArrayRef<Chunk *> getCodeChunks() { return thunks; }
  ArrayRef<Chunk *> getCodePData() { return pdata; }
  ArrayRef<Chunk *> getCodeUnwindInfo() { return unwindinfo; }
  ArrayRef<Chunk *> getAuxIat() { return auxIat; }
  ArrayRef<Chunk *> getAuxIatCopy() { return auxIatCopy; }

  uint64_t getDirRVA() { return dirs[0]->getRVA(); }
  uint64_t getDirSize();

private:
  Chunk *newThunkChunk(DefinedImportData *s, Chunk *tailMerge);
  Chunk *newTailMergeChunk(SymbolTable &symtab, Chunk *dir);
  Chunk *newTailMergePDataChunk(SymbolTable &symtab, Chunk *tm);

  std::vector<DefinedImportData *> imports;
  std::vector<Chunk *> dirs;
  std::vector<Chunk *> moduleHandles;
  std::vector<Chunk *> addresses;
  std::vector<Chunk *> names;
  std::vector<Chunk *> hintNames;
  std::vector<Chunk *> thunks;
  std::vector<Chunk *> pdata;
  std::vector<Chunk *> unwindinfo;
  std::vector<Chunk *> dllNames;
  std::vector<Chunk *> auxIat;
  std::vector<Chunk *> auxIatCopy;

  COFFLinkerContext &ctx;
};

// Create all chunks for the DLL export table.
void createEdataChunks(SymbolTable &symtab, std::vector<Chunk *> &chunks);

} // namespace lld::coff

#endif
