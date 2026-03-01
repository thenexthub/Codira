//===- SymbolTable.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLD_MACHO_SYMBOL_TABLE_H
#define LLD_MACHO_SYMBOL_TABLE_H

#include "Symbols.h"

#include "lld/Common/LLVM.h"
#include "llvm/ADT/CachedHashString.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Object/Archive.h"

namespace lld::macho {

class ArchiveFile;
class DylibFile;
class InputFile;
class ObjFile;
class InputSection;
class MachHeaderSection;
class Symbol;
class Defined;
class Undefined;

/*
 * Note that the SymbolTable handles name collisions by calling
 * replaceSymbol(), which does an in-place update of the Symbol via `placement
 * new`. Therefore, there is no need to update any relocations that hold
 * pointers the "old" Symbol -- they will automatically point to the new one.
 */
class SymbolTable {
public:
  Defined *addDefined(StringRef name, InputFile *, InputSection *,
                      uint64_t value, uint64_t size, bool isWeakDef,
                      bool isPrivateExtern, bool isReferencedDynamically,
                      bool noDeadStrip, bool isWeakDefCanBeHidden);

  Defined *aliasDefined(Defined *src, StringRef target, InputFile *newFile,
                        bool makePrivateExtern = false);

  Symbol *addUndefined(StringRef name, InputFile *, bool isWeakRef);

  Symbol *addCommon(StringRef name, InputFile *, uint64_t size, uint32_t align,
                    bool isPrivateExtern);

  Symbol *addDylib(StringRef name, DylibFile *file, bool isWeakDef, bool isTlv);
  Symbol *addDynamicLookup(StringRef name);

  Symbol *addLazyArchive(StringRef name, ArchiveFile *file,
                         const llvm::object::Archive::Symbol &sym);
  Symbol *addLazyObject(StringRef name, InputFile &file);

  Defined *addSynthetic(StringRef name, InputSection *, uint64_t value,
                        bool isPrivateExtern, bool includeInSymtab,
                        bool referencedDynamically);

  ArrayRef<Symbol *> getSymbols() const { return symVector; }
  Symbol *find(llvm::CachedHashStringRef name);
  Symbol *find(StringRef name) { return find(llvm::CachedHashStringRef(name)); }

private:
  std::pair<Symbol *, bool> insert(StringRef name, const InputFile *);
  llvm::DenseMap<llvm::CachedHashStringRef, int> symMap;
  std::vector<Symbol *> symVector;
};

void reportPendingUndefinedSymbols();
void reportPendingDuplicateSymbols();

// Call reportPendingUndefinedSymbols() to emit diagnostics.
void treatUndefinedSymbol(const Undefined &, StringRef source);
void treatUndefinedSymbol(const Undefined &, const InputSection *,
                          uint64_t offset);

extern std::unique_ptr<SymbolTable> symtab;

} // namespace lld::macho

#endif
