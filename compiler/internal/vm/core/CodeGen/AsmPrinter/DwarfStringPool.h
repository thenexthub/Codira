//===- toolchain/CodeGen/DwarfStringPool.h - Dwarf Debug Framework ---*- C++ -*-===//
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

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_DWARFSTRINGPOOL_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_DWARFSTRINGPOOL_H

#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/DwarfStringPoolEntry.h"
#include "vm/core/Support/Allocator.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

class AsmPrinter;
class MCSection;
class MCSymbol;

// Collection of strings for this unit and assorted symbols.
// A String->Symbol mapping of strings used by indirect
// references.
class DwarfStringPool {
  using EntryTy = DwarfStringPoolEntry;

  StringMap<EntryTy, BumpPtrAllocator &> Pool;
  StringRef Prefix;
  uint64_t NumBytes = 0;
  unsigned NumIndexedStrings = 0;
  bool ShouldCreateSymbols;

  StringMapEntry<EntryTy> &getEntryImpl(AsmPrinter &Asm, StringRef Str);

public:
  using EntryRef = DwarfStringPoolEntryRef;

  LLVM_ABI_FOR_TEST DwarfStringPool(BumpPtrAllocator &A, AsmPrinter &Asm,
                                    StringRef Prefix);

  LLVM_ABI_FOR_TEST void emitStringOffsetsTableHeader(AsmPrinter &Asm,
                                                      MCSection *OffsetSection,
                                                      MCSymbol *StartSym);

  LLVM_ABI_FOR_TEST void emit(AsmPrinter &Asm, MCSection *StrSection,
                              MCSection *OffsetSection = nullptr,
                              bool UseRelativeOffsets = false);

  bool empty() const { return Pool.empty(); }

  unsigned size() const { return Pool.size(); }

  unsigned getNumIndexedStrings() const { return NumIndexedStrings; }

  /// Get a reference to an entry in the string pool.
  LLVM_ABI_FOR_TEST EntryRef getEntry(AsmPrinter &Asm, StringRef Str);

  /// Same as getEntry, except that you can use EntryRef::getIndex to obtain a
  /// unique ID of this entry (e.g., for use in indexed forms like
  /// DW_FORM_strx).
  LLVM_ABI_FOR_TEST EntryRef getIndexedEntry(AsmPrinter &Asm, StringRef Str);
};

} // end namespace vm::core

#endif // LLVM_LIB_CODEGEN_ASMPRINTER_DWARFSTRINGPOOL_H
