//===- NativeTypeArray.cpp - info about arrays ------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeTypeArray.h"

#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/PDBExtras.h"

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeTypeArray::NativeTypeArray(NativeSession &Session, SymIndexId Id,
                                 codeview::TypeIndex TI,
                                 codeview::ArrayRecord Record)
    : NativeRawSymbol(Session, PDB_SymType::ArrayType, Id), Record(Record),
      Index(TI) {}
NativeTypeArray::~NativeTypeArray() = default;

void NativeTypeArray::dump(raw_ostream &OS, int Indent,
                           PdbSymbolIdField ShowIdFields,
                           PdbSymbolIdField RecurseIdFields) const {
  NativeRawSymbol::dump(OS, Indent, ShowIdFields, RecurseIdFields);

  dumpSymbolField(OS, "arrayIndexTypeId", getArrayIndexTypeId(), Indent);
  dumpSymbolIdField(OS, "elementTypeId", getTypeId(), Indent, Session,
                    PdbSymbolIdField::Type, ShowIdFields, RecurseIdFields);

  dumpSymbolIdField(OS, "lexicalParentId", 0, Indent, Session,
                    PdbSymbolIdField::LexicalParent, ShowIdFields,
                    RecurseIdFields);
  dumpSymbolField(OS, "length", getLength(), Indent);
  dumpSymbolField(OS, "count", getCount(), Indent);
  dumpSymbolField(OS, "constType", isConstType(), Indent);
  dumpSymbolField(OS, "unalignedType", isUnalignedType(), Indent);
  dumpSymbolField(OS, "volatileType", isVolatileType(), Indent);
}

SymIndexId NativeTypeArray::getArrayIndexTypeId() const {
  return Session.getSymbolCache().findSymbolByTypeIndex(Record.getIndexType());
}

bool NativeTypeArray::isConstType() const { return false; }

bool NativeTypeArray::isUnalignedType() const { return false; }

bool NativeTypeArray::isVolatileType() const { return false; }

uint32_t NativeTypeArray::getCount() const {
  NativeRawSymbol &Element =
      Session.getSymbolCache().getNativeSymbolById(getTypeId());
  return getLength() / Element.getLength();
}

SymIndexId NativeTypeArray::getTypeId() const {
  return Session.getSymbolCache().findSymbolByTypeIndex(
      Record.getElementType());
}

uint64_t NativeTypeArray::getLength() const { return Record.Size; }
