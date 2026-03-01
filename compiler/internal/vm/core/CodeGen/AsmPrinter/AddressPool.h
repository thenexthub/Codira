//===- toolchain/CodeGen/AddressPool.h - Dwarf Debug Framework -------*- C++ -*-===//
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

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_ADDRESSPOOL_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_ADDRESSPOOL_H

#include "vm/core/ADT/DenseMap.h"

namespace vm::core {

class AsmPrinter;
class MCSection;
class MCSymbol;

// Collection of addresses for this unit and assorted labels.
// A Symbol->unsigned mapping of addresses used by indirect
// references.
class AddressPool {
  struct AddressPoolEntry {
    unsigned Number;
    bool TLS;

    AddressPoolEntry(unsigned Number, bool TLS) : Number(Number), TLS(TLS) {}
  };
  DenseMap<const MCSymbol *, AddressPoolEntry> Pool;

  /// Record whether the AddressPool has been queried for an address index since
  /// the last "resetUsedFlag" call. Used to implement type unit fallback - a
  /// type that references addresses cannot be placed in a type unit when using
  /// fission.
  bool HasBeenUsed = false;

public:
  AddressPool() = default;

  /// Returns the index into the address pool with the given
  /// label/symbol.
  unsigned getIndex(const MCSymbol *Sym, bool TLS = false);

  void emit(AsmPrinter &Asm, MCSection *AddrSection);

  bool isEmpty() { return Pool.empty(); }

  bool hasBeenUsed() const { return HasBeenUsed; }

  void resetUsedFlag(bool HasBeenUsed = false) { this->HasBeenUsed = HasBeenUsed; }

  MCSymbol *getLabel() { return AddressTableBaseSym; }
  void setLabel(MCSymbol *Sym) { AddressTableBaseSym = Sym; }

private:
  MCSymbol *emitHeader(AsmPrinter &Asm, MCSection *Section);

  /// Symbol designates the start of the contribution to the address table.
  MCSymbol *AddressTableBaseSym = nullptr;
};

} // end namespace vm::core

#endif // LLVM_LIB_CODEGEN_ASMPRINTER_ADDRESSPOOL_H
