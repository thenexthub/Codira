//===- NativeExeSymbol.cpp - native impl for PDBSymbolExe -------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeExeSymbol.h"

#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/PDB/Native/DbiStream.h"
#include "vm/core/DebugInfo/PDB/Native/InfoStream.h"
#include "vm/core/DebugInfo/PDB/Native/NativeEnumModules.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"
#include "vm/core/DebugInfo/PDB/Native/PDBFile.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"

using namespace vm::core;
using namespace vm::core::pdb;

static DbiStream *getDbiStreamPtr(NativeSession &Session) {
  Expected<DbiStream &> DbiS = Session.getPDBFile().getPDBDbiStream();
  if (DbiS)
    return &DbiS.get();

  consumeError(DbiS.takeError());
  return nullptr;
}

NativeExeSymbol::NativeExeSymbol(NativeSession &Session, SymIndexId SymbolId)
    : NativeRawSymbol(Session, PDB_SymType::Exe, SymbolId),
      Dbi(getDbiStreamPtr(Session)) {}

std::unique_ptr<IPDBEnumSymbols>
NativeExeSymbol::findChildren(PDB_SymType Type) const {
  switch (Type) {
  case PDB_SymType::Compiland: {
    return std::unique_ptr<IPDBEnumSymbols>(new NativeEnumModules(Session));
    break;
  }
  case PDB_SymType::ArrayType:
    return Session.getSymbolCache().createTypeEnumerator(codeview::LF_ARRAY);
  case PDB_SymType::Enum:
    return Session.getSymbolCache().createTypeEnumerator(codeview::LF_ENUM);
  case PDB_SymType::PointerType:
    return Session.getSymbolCache().createTypeEnumerator(codeview::LF_POINTER);
  case PDB_SymType::UDT:
    return Session.getSymbolCache().createTypeEnumerator(
        {codeview::LF_STRUCTURE, codeview::LF_CLASS, codeview::LF_UNION,
         codeview::LF_INTERFACE});
  case PDB_SymType::VTableShape:
    return Session.getSymbolCache().createTypeEnumerator(codeview::LF_VTSHAPE);
  case PDB_SymType::FunctionSig:
    return Session.getSymbolCache().createTypeEnumerator(
        {codeview::LF_PROCEDURE, codeview::LF_MFUNCTION});
  case PDB_SymType::Typedef:
    return Session.getSymbolCache().createGlobalsEnumerator(codeview::S_UDT);

  default:
    break;
  }
  return nullptr;
}

uint32_t NativeExeSymbol::getAge() const {
  auto IS = Session.getPDBFile().getPDBInfoStream();
  if (IS)
    return IS->getAge();
  consumeError(IS.takeError());
  return 0;
}

std::string NativeExeSymbol::getSymbolsFileName() const {
  return std::string(Session.getPDBFile().getFilePath());
}

codeview::GUID NativeExeSymbol::getGuid() const {
  auto IS = Session.getPDBFile().getPDBInfoStream();
  if (IS)
    return IS->getGuid();
  consumeError(IS.takeError());
  return codeview::GUID{{0}};
}

bool NativeExeSymbol::hasCTypes() const {
  auto Dbi = Session.getPDBFile().getPDBDbiStream();
  if (Dbi)
    return Dbi->hasCTypes();
  consumeError(Dbi.takeError());
  return false;
}

bool NativeExeSymbol::hasPrivateSymbols() const {
  auto Dbi = Session.getPDBFile().getPDBDbiStream();
  if (Dbi)
    return !Dbi->isStripped();
  consumeError(Dbi.takeError());
  return false;
}
