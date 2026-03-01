//===- RecordStreamer.h - Record asm defined and used symbols ---*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJECT_RECORDSTREAMER_H
#define LLVM_LIB_OBJECT_RECORDSTREAMER_H

#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/MapVector.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/MC/MCDirectives.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/SMLoc.h"
#include <vector>

namespace vm::core {

class MCSymbol;
class Module;

class RecordStreamer : public MCStreamer {
public:
  enum State { NeverSeen, Global, Defined, DefinedGlobal, DefinedWeak, Used,
               UndefinedWeak};

private:
  const Module &M;
  StringMap<State> Symbols;
  // Map of aliases created by .symver directives, saved so we can update
  // their symbol binding after parsing complete. This maps from each
  // aliasee to its list of aliases.
  MapVector<const MCSymbol *, std::vector<StringRef>> SymverAliasMap;

  /// Get the state recorded for the given symbol.
  State getSymbolState(const MCSymbol *Sym);

  void markDefined(const MCSymbol &Symbol);
  void markGlobal(const MCSymbol &Symbol, MCSymbolAttr Attribute);
  void markUsed(const MCSymbol &Symbol);
  void visitUsedSymbol(const MCSymbol &Sym) override;

public:
  RecordStreamer(MCContext &Context, const Module &M);

  void emitLabel(MCSymbol *Symbol, SMLoc Loc = SMLoc()) override;
  void emitAssignment(MCSymbol *Symbol, const MCExpr *Value) override;
  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override;
  void emitZerofill(MCSection *Section, MCSymbol *Symbol, uint64_t Size,
                    Align ByteAlignment, SMLoc Loc = SMLoc()) override;
  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        Align ByteAlignment) override;

  // Ignore format-specific directives; we do not need any information from
  // them, but the default implementation of these methods crashes, so we
  // override them with versions that do nothing.
  void emitSubsectionsViaSymbols() override {};
  void beginCOFFSymbolDef(const MCSymbol *Symbol) override {}
  void emitCOFFSymbolStorageClass(int StorageClass) override {}
  void emitCOFFSymbolType(int Type) override {}
  void endCOFFSymbolDef() override {}

  /// Record .symver aliases for later processing.
  void emitELFSymverDirective(const MCSymbol *OriginalSym, StringRef Name,
                              bool KeepOriginalSym) override;

  // Emit ELF .symver aliases and ensure they have the same binding as the
  // defined symbol they alias with.
  void flushSymverDirectives();

  // Symbols iterators
  using const_iterator = StringMap<State>::const_iterator;
  const_iterator begin();
  const_iterator end();

  // SymverAliasMap iterators
  using const_symver_iterator = decltype(SymverAliasMap)::const_iterator;
  iterator_range<const_symver_iterator> symverAliases();
};

} // end namespace vm::core

#endif // LLVM_LIB_OBJECT_RECORDSTREAMER_H
