//===- PDBSymbol.cpp - base class for user-facing symbol types --*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/IPDBEnumChildren.h"
#include "vm/core/DebugInfo/PDB/IPDBLineNumber.h"
#include "vm/core/DebugInfo/PDB/IPDBRawSymbol.h"
#include "vm/core/DebugInfo/PDB/IPDBSession.h"
#include "vm/core/DebugInfo/PDB/PDBExtras.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolAnnotation.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolBlock.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompiland.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompilandDetails.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompilandEnv.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCustom.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolData.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolExe.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolFunc.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolFuncDebugEnd.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolFuncDebugStart.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolLabel.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolPublicSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolThunk.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeArray.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeBaseClass.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeBuiltin.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeCustom.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeDimension.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeEnum.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFriend.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFunctionArg.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFunctionSig.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeManaged.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypePointer.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeTypedef.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeUDT.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeVTable.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeVTableShape.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolUnknown.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolUsingNamespace.h"
#include "vm/core/DebugInfo/PDB/PDBTypes.h"
#include <memory>

using namespace vm::core;
using namespace vm::core::pdb;

PDBSymbol::PDBSymbol(const IPDBSession &PDBSession) : Session(PDBSession) {}

PDBSymbol::PDBSymbol(PDBSymbol &&Other)
    : Session(Other.Session), RawSymbol(std::move(Other.RawSymbol)) {}

PDBSymbol::~PDBSymbol() = default;

#define FACTORY_SYMTAG_CASE(Tag, Type)                                         \
  case PDB_SymType::Tag:                                                       \
    return std::unique_ptr<PDBSymbol>(new Type(PDBSession));

std::unique_ptr<PDBSymbol>
PDBSymbol::createSymbol(const IPDBSession &PDBSession, PDB_SymType Tag) {
  switch (Tag) {
    FACTORY_SYMTAG_CASE(Exe, PDBSymbolExe)
    FACTORY_SYMTAG_CASE(Compiland, PDBSymbolCompiland)
    FACTORY_SYMTAG_CASE(CompilandDetails, PDBSymbolCompilandDetails)
    FACTORY_SYMTAG_CASE(CompilandEnv, PDBSymbolCompilandEnv)
    FACTORY_SYMTAG_CASE(Function, PDBSymbolFunc)
    FACTORY_SYMTAG_CASE(Block, PDBSymbolBlock)
    FACTORY_SYMTAG_CASE(Data, PDBSymbolData)
    FACTORY_SYMTAG_CASE(Annotation, PDBSymbolAnnotation)
    FACTORY_SYMTAG_CASE(Label, PDBSymbolLabel)
    FACTORY_SYMTAG_CASE(PublicSymbol, PDBSymbolPublicSymbol)
    FACTORY_SYMTAG_CASE(UDT, PDBSymbolTypeUDT)
    FACTORY_SYMTAG_CASE(Enum, PDBSymbolTypeEnum)
    FACTORY_SYMTAG_CASE(FunctionSig, PDBSymbolTypeFunctionSig)
    FACTORY_SYMTAG_CASE(PointerType, PDBSymbolTypePointer)
    FACTORY_SYMTAG_CASE(ArrayType, PDBSymbolTypeArray)
    FACTORY_SYMTAG_CASE(BuiltinType, PDBSymbolTypeBuiltin)
    FACTORY_SYMTAG_CASE(Typedef, PDBSymbolTypeTypedef)
    FACTORY_SYMTAG_CASE(BaseClass, PDBSymbolTypeBaseClass)
    FACTORY_SYMTAG_CASE(Friend, PDBSymbolTypeFriend)
    FACTORY_SYMTAG_CASE(FunctionArg, PDBSymbolTypeFunctionArg)
    FACTORY_SYMTAG_CASE(FuncDebugStart, PDBSymbolFuncDebugStart)
    FACTORY_SYMTAG_CASE(FuncDebugEnd, PDBSymbolFuncDebugEnd)
    FACTORY_SYMTAG_CASE(UsingNamespace, PDBSymbolUsingNamespace)
    FACTORY_SYMTAG_CASE(VTableShape, PDBSymbolTypeVTableShape)
    FACTORY_SYMTAG_CASE(VTable, PDBSymbolTypeVTable)
    FACTORY_SYMTAG_CASE(Custom, PDBSymbolCustom)
    FACTORY_SYMTAG_CASE(Thunk, PDBSymbolThunk)
    FACTORY_SYMTAG_CASE(CustomType, PDBSymbolTypeCustom)
    FACTORY_SYMTAG_CASE(ManagedType, PDBSymbolTypeManaged)
    FACTORY_SYMTAG_CASE(Dimension, PDBSymbolTypeDimension)
  default:
    return std::unique_ptr<PDBSymbol>(new PDBSymbolUnknown(PDBSession));
  }
}

std::unique_ptr<PDBSymbol>
PDBSymbol::create(const IPDBSession &PDBSession,
                  std::unique_ptr<IPDBRawSymbol> RawSymbol) {
  auto SymbolPtr = createSymbol(PDBSession, RawSymbol->getSymTag());
  SymbolPtr->RawSymbol = RawSymbol.get();
  SymbolPtr->OwnedRawSymbol = std::move(RawSymbol);
  return SymbolPtr;
}

std::unique_ptr<PDBSymbol> PDBSymbol::create(const IPDBSession &PDBSession,
                                             IPDBRawSymbol &RawSymbol) {
  auto SymbolPtr = createSymbol(PDBSession, RawSymbol.getSymTag());
  SymbolPtr->RawSymbol = &RawSymbol;
  return SymbolPtr;
}

void PDBSymbol::defaultDump(raw_ostream &OS, int Indent,
                            PdbSymbolIdField ShowFlags,
                            PdbSymbolIdField RecurseFlags) const {
  RawSymbol->dump(OS, Indent, ShowFlags, RecurseFlags);
}

void PDBSymbol::dumpProperties() const {
  outs() << "\n";
  defaultDump(outs(), 0, PdbSymbolIdField::All, PdbSymbolIdField::None);
  outs().flush();
}

void PDBSymbol::dumpChildStats() const {
  TagStats Stats;
  getChildStats(Stats);
  outs() << "\n";
  for (auto &Stat : Stats) {
    outs() << Stat.first << ": " << Stat.second << "\n";
  }
  outs().flush();
}

PDB_SymType PDBSymbol::getSymTag() const { return RawSymbol->getSymTag(); }
uint32_t PDBSymbol::getSymIndexId() const { return RawSymbol->getSymIndexId(); }

std::unique_ptr<IPDBEnumSymbols> PDBSymbol::findAllChildren() const {
  return findAllChildren(PDB_SymType::None);
}

std::unique_ptr<IPDBEnumSymbols>
PDBSymbol::findAllChildren(PDB_SymType Type) const {
  return RawSymbol->findChildren(Type);
}

std::unique_ptr<IPDBEnumSymbols>
PDBSymbol::findChildren(PDB_SymType Type, StringRef Name,
                        PDB_NameSearchFlags Flags) const {
  return RawSymbol->findChildren(Type, Name, Flags);
}

std::unique_ptr<IPDBEnumSymbols>
PDBSymbol::findChildrenByRVA(PDB_SymType Type, StringRef Name,
                             PDB_NameSearchFlags Flags, uint32_t RVA) const {
  return RawSymbol->findChildrenByRVA(Type, Name, Flags, RVA);
}

std::unique_ptr<IPDBEnumSymbols>
PDBSymbol::findInlineFramesByVA(uint64_t VA) const {
  return RawSymbol->findInlineFramesByVA(VA);
}

std::unique_ptr<IPDBEnumSymbols>
PDBSymbol::findInlineFramesByRVA(uint32_t RVA) const {
  return RawSymbol->findInlineFramesByRVA(RVA);
}

std::unique_ptr<IPDBEnumLineNumbers>
PDBSymbol::findInlineeLinesByVA(uint64_t VA, uint32_t Length) const {
  return RawSymbol->findInlineeLinesByVA(VA, Length);
}

std::unique_ptr<IPDBEnumLineNumbers>
PDBSymbol::findInlineeLinesByRVA(uint32_t RVA, uint32_t Length) const {
  return RawSymbol->findInlineeLinesByRVA(RVA, Length);
}

std::string PDBSymbol::getName() const { return RawSymbol->getName(); }

std::unique_ptr<IPDBEnumSymbols>
PDBSymbol::getChildStats(TagStats &Stats) const {
  std::unique_ptr<IPDBEnumSymbols> Result(findAllChildren());
  if (!Result)
    return nullptr;
  Stats.clear();
  while (auto Child = Result->getNext()) {
    ++Stats[Child->getSymTag()];
  }
  Result->reset();
  return Result;
}

std::unique_ptr<PDBSymbol> PDBSymbol::getSymbolByIdHelper(uint32_t Id) const {
  return Session.getSymbolById(Id);
}

void toolchain::pdb::dumpSymbolIdField(raw_ostream &OS, StringRef Name,
                                  SymIndexId Value, int Indent,
                                  const IPDBSession &Session,
                                  PdbSymbolIdField FieldId,
                                  PdbSymbolIdField ShowFlags,
                                  PdbSymbolIdField RecurseFlags) {
  if ((FieldId & ShowFlags) == PdbSymbolIdField::None)
    return;

  OS << "\n";
  OS.indent(Indent);
  OS << Name << ": " << Value;
  // Don't recurse unless the user requested it.
  if ((FieldId & RecurseFlags) == PdbSymbolIdField::None)
    return;
  // And obviously don't recurse on the symbol itself.
  if (FieldId == PdbSymbolIdField::SymIndexId)
    return;

  auto Child = Session.getSymbolById(Value);

  // It could have been a placeholder symbol for a type we don't yet support,
  // so just exit in that case.
  if (!Child)
    return;

  // Don't recurse more than once, so pass PdbSymbolIdField::None) for the
  // recurse flags.
  Child->defaultDump(OS, Indent + 2, ShowFlags, PdbSymbolIdField::None);
}
