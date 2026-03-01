//===- NativeTypePointer.cpp - info about pointer type ----------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeTypePointer.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"

#include "vm/core/DebugInfo/CodeView/CodeView.h"

#include <cassert>

using namespace vm::core;
using namespace vm::core::codeview;
using namespace vm::core::pdb;

NativeTypePointer::NativeTypePointer(NativeSession &Session, SymIndexId Id,
                                     codeview::TypeIndex TI)
    : NativeRawSymbol(Session, PDB_SymType::PointerType, Id), TI(TI) {
  assert(TI.isSimple());
  assert(TI.getSimpleMode() != SimpleTypeMode::Direct);
}

NativeTypePointer::NativeTypePointer(NativeSession &Session, SymIndexId Id,
                                     codeview::TypeIndex TI,
                                     codeview::PointerRecord Record)
    : NativeRawSymbol(Session, PDB_SymType::PointerType, Id), TI(TI),
      Record(std::move(Record)) {}

NativeTypePointer::~NativeTypePointer() = default;

void NativeTypePointer::dump(raw_ostream &OS, int Indent,
                             PdbSymbolIdField ShowIdFields,
                             PdbSymbolIdField RecurseIdFields) const {
  NativeRawSymbol::dump(OS, Indent, ShowIdFields, RecurseIdFields);

  if (isMemberPointer()) {
    dumpSymbolIdField(OS, "classParentId", getClassParentId(), Indent, Session,
                      PdbSymbolIdField::ClassParent, ShowIdFields,
                      RecurseIdFields);
  }
  dumpSymbolIdField(OS, "lexicalParentId", 0, Indent, Session,
                    PdbSymbolIdField::LexicalParent, ShowIdFields,
                    RecurseIdFields);
  dumpSymbolIdField(OS, "typeId", getTypeId(), Indent, Session,
                    PdbSymbolIdField::Type, ShowIdFields, RecurseIdFields);
  dumpSymbolField(OS, "length", getLength(), Indent);
  dumpSymbolField(OS, "constType", isConstType(), Indent);
  dumpSymbolField(OS, "isPointerToDataMember", isPointerToDataMember(), Indent);
  dumpSymbolField(OS, "isPointerToMemberFunction", isPointerToMemberFunction(),
                  Indent);
  dumpSymbolField(OS, "RValueReference", isRValueReference(), Indent);
  dumpSymbolField(OS, "reference", isReference(), Indent);
  dumpSymbolField(OS, "restrictedType", isRestrictedType(), Indent);
  if (isMemberPointer()) {
    if (isSingleInheritance())
      dumpSymbolField(OS, "isSingleInheritance", 1, Indent);
    else if (isMultipleInheritance())
      dumpSymbolField(OS, "isMultipleInheritance", 1, Indent);
    else if (isVirtualInheritance())
      dumpSymbolField(OS, "isVirtualInheritance", 1, Indent);
  }
  dumpSymbolField(OS, "unalignedType", isUnalignedType(), Indent);
  dumpSymbolField(OS, "volatileType", isVolatileType(), Indent);
}

SymIndexId NativeTypePointer::getClassParentId() const {
  if (!isMemberPointer())
    return 0;

  assert(Record);
  const MemberPointerInfo &MPI = Record->getMemberInfo();
  return Session.getSymbolCache().findSymbolByTypeIndex(MPI.ContainingType);
}

uint64_t NativeTypePointer::getLength() const {
  if (Record)
    return Record->getSize();

  switch (TI.getSimpleMode()) {
  case SimpleTypeMode::NearPointer:
  case SimpleTypeMode::FarPointer:
  case SimpleTypeMode::HugePointer:
    return 2;
  case SimpleTypeMode::NearPointer32:
  case SimpleTypeMode::FarPointer32:
    return 4;
  case SimpleTypeMode::NearPointer64:
    return 8;
  case SimpleTypeMode::NearPointer128:
    return 16;
  default:
    assert(false && "invalid simple type mode!");
  }
  return 0;
}

SymIndexId NativeTypePointer::getTypeId() const {
  // This is the pointee SymIndexId.
  TypeIndex Referent = Record ? Record->ReferentType : TI.makeDirect();

  return Session.getSymbolCache().findSymbolByTypeIndex(Referent);
}

bool NativeTypePointer::isReference() const {
  if (!Record)
    return false;
  return Record->getMode() == PointerMode::LValueReference;
}

bool NativeTypePointer::isRValueReference() const {
  if (!Record)
    return false;
  return Record->getMode() == PointerMode::RValueReference;
}

bool NativeTypePointer::isPointerToDataMember() const {
  if (!Record)
    return false;
  return Record->getMode() == PointerMode::PointerToDataMember;
}

bool NativeTypePointer::isPointerToMemberFunction() const {
  if (!Record)
    return false;
  return Record->getMode() == PointerMode::PointerToMemberFunction;
}

bool NativeTypePointer::isConstType() const {
  if (!Record)
    return false;
  return (Record->getOptions() & PointerOptions::Const) != PointerOptions::None;
}

bool NativeTypePointer::isRestrictedType() const {
  if (!Record)
    return false;
  return (Record->getOptions() & PointerOptions::Restrict) !=
         PointerOptions::None;
}

bool NativeTypePointer::isVolatileType() const {
  if (!Record)
    return false;
  return (Record->getOptions() & PointerOptions::Volatile) !=
         PointerOptions::None;
}

bool NativeTypePointer::isUnalignedType() const {
  if (!Record)
    return false;
  return (Record->getOptions() & PointerOptions::Unaligned) !=
         PointerOptions::None;
}

static inline bool isInheritanceKind(const MemberPointerInfo &MPI,
                                     PointerToMemberRepresentation P1,
                                     PointerToMemberRepresentation P2) {
  return (MPI.getRepresentation() == P1 || MPI.getRepresentation() == P2);
}

bool NativeTypePointer::isSingleInheritance() const {
  if (!isMemberPointer())
    return false;
  return isInheritanceKind(
      Record->getMemberInfo(),
      PointerToMemberRepresentation::SingleInheritanceData,
      PointerToMemberRepresentation::SingleInheritanceFunction);
}

bool NativeTypePointer::isMultipleInheritance() const {
  if (!isMemberPointer())
    return false;
  return isInheritanceKind(
      Record->getMemberInfo(),
      PointerToMemberRepresentation::MultipleInheritanceData,
      PointerToMemberRepresentation::MultipleInheritanceFunction);
}

bool NativeTypePointer::isVirtualInheritance() const {
  if (!isMemberPointer())
    return false;
  return isInheritanceKind(
      Record->getMemberInfo(),
      PointerToMemberRepresentation::VirtualInheritanceData,
      PointerToMemberRepresentation::VirtualInheritanceFunction);
}

bool NativeTypePointer::isMemberPointer() const {
  return isPointerToDataMember() || isPointerToMemberFunction();
}
