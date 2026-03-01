//===- NativePublicSymbol.cpp - info about public symbols -------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativePublicSymbol.h"

#include "vm/core/DebugInfo/CodeView/SymbolRecord.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativePublicSymbol::NativePublicSymbol(NativeSession &Session, SymIndexId Id,
                                       const codeview::PublicSym32 &Sym)
    : NativeRawSymbol(Session, PDB_SymType::PublicSymbol, Id), Sym(Sym) {}

NativePublicSymbol::~NativePublicSymbol() = default;

void NativePublicSymbol::dump(raw_ostream &OS, int Indent,
                              PdbSymbolIdField ShowIdFields,
                              PdbSymbolIdField RecurseIdFields) const {
  NativeRawSymbol::dump(OS, Indent, ShowIdFields, RecurseIdFields);
  dumpSymbolField(OS, "name", getName(), Indent);
  dumpSymbolField(OS, "offset", getAddressOffset(), Indent);
  dumpSymbolField(OS, "section", getAddressSection(), Indent);
}

uint32_t NativePublicSymbol::getAddressOffset() const { return Sym.Offset; }

uint32_t NativePublicSymbol::getAddressSection() const { return Sym.Segment; }

std::string NativePublicSymbol::getName() const {
  return std::string(Sym.Name);
}

uint32_t NativePublicSymbol::getRelativeVirtualAddress() const {
  return Session.getRVAFromSectOffset(Sym.Segment, Sym.Offset);
}

uint64_t NativePublicSymbol::getVirtualAddress() const {
  return Session.getVAFromSectOffset(Sym.Segment, Sym.Offset);
}
