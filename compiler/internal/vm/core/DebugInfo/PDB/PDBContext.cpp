//===-- PDBContext.cpp ------------------------------------------*- C++ -*-===//
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
//===----------------------------------------------------------------------===/

#include "vm/core/DebugInfo/PDB/PDBContext.h"
#include "vm/core/DebugInfo/PDB/IPDBEnumChildren.h"
#include "vm/core/DebugInfo/PDB/IPDBLineNumber.h"
#include "vm/core/DebugInfo/PDB/IPDBSourceFile.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolData.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolFunc.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolPublicSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFunctionSig.h"
#include "vm/core/DebugInfo/PDB/PDBTypes.h"
#include "vm/core/Object/COFF.h"

using namespace vm::core;
using namespace vm::core::object;
using namespace vm::core::pdb;

PDBContext::PDBContext(const COFFObjectFile &Object,
                       std::unique_ptr<IPDBSession> PDBSession)
    : DIContext(CK_PDB), Session(std::move(PDBSession)) {
  ErrorOr<uint64_t> ImageBase = Object.getImageBase();
  if (ImageBase)
    Session->setLoadAddress(ImageBase.get());
}

void PDBContext::dump(raw_ostream &OS, DIDumpOptions DumpOpts){}

std::optional<DILineInfo>
PDBContext::getLineInfoForAddress(object::SectionedAddress Address,
                                  DILineInfoSpecifier Specifier) {
  DILineInfo Result;
  Result.FunctionName = getFunctionName(Address.Address, Specifier.FNKind);

  uint32_t Length = 1;
  std::unique_ptr<PDBSymbol> Symbol =
      Session->findSymbolByAddress(Address.Address, PDB_SymType::None);
  if (auto Func = dyn_cast_or_null<PDBSymbolFunc>(Symbol.get())) {
    Length = Func->getLength();
  } else if (auto Data = dyn_cast_or_null<PDBSymbolData>(Symbol.get())) {
    Length = Data->getLength();
  }

  // If we couldn't find a symbol, then just assume 1 byte, so that we get
  // only the line number of the first instruction.
  auto LineNumbers = Session->findLineNumbersByAddress(Address.Address, Length);
  if (!LineNumbers || LineNumbers->getChildCount() == 0)
    return Result;

  auto LineInfo = LineNumbers->getNext();
  assert(LineInfo);
  auto SourceFile = Session->getSourceFileById(LineInfo->getSourceFileId());

  if (SourceFile &&
      Specifier.FLIKind != DILineInfoSpecifier::FileLineInfoKind::None)
    Result.FileName = SourceFile->getFileName();
  Result.Column = LineInfo->getColumnNumber();
  Result.Line = LineInfo->getLineNumber();
  return Result;
}

std::optional<DILineInfo>
PDBContext::getLineInfoForDataAddress(object::SectionedAddress Address) {
  // Unimplemented. S_GDATA and S_LDATA in CodeView (used to describe global
  // variables) aren't capable of carrying line information.
  return DILineInfo();
}

DILineInfoTable
PDBContext::getLineInfoForAddressRange(object::SectionedAddress Address,
                                       uint64_t Size,
                                       DILineInfoSpecifier Specifier) {
  if (Size == 0)
    return DILineInfoTable();

  DILineInfoTable Table;
  auto LineNumbers = Session->findLineNumbersByAddress(Address.Address, Size);
  if (!LineNumbers || LineNumbers->getChildCount() == 0)
    return Table;

  while (auto LineInfo = LineNumbers->getNext()) {
    if (std::optional<DILineInfo> LineEntry = getLineInfoForAddress(
            {LineInfo->getVirtualAddress(), Address.SectionIndex}, Specifier))
      Table.push_back(
          std::make_pair(LineInfo->getVirtualAddress(), *LineEntry));
  }
  return Table;
}

DIInliningInfo
PDBContext::getInliningInfoForAddress(object::SectionedAddress Address,
                                      DILineInfoSpecifier Specifier) {
  DIInliningInfo InlineInfo;
  DILineInfo CurrentLine =
      getLineInfoForAddress(Address, Specifier).value_or(DILineInfo());

  // Find the function at this address.
  std::unique_ptr<PDBSymbol> ParentFunc =
      Session->findSymbolByAddress(Address.Address, PDB_SymType::Function);
  if (!ParentFunc) {
    InlineInfo.addFrame(CurrentLine);
    return InlineInfo;
  }

  auto Frames = ParentFunc->findInlineFramesByVA(Address.Address);
  if (!Frames || Frames->getChildCount() == 0) {
    InlineInfo.addFrame(CurrentLine);
    return InlineInfo;
  }

  while (auto Frame = Frames->getNext()) {
    uint32_t Length = 1;
    auto LineNumbers = Frame->findInlineeLinesByVA(Address.Address, Length);
    if (!LineNumbers || LineNumbers->getChildCount() == 0)
      break;

    std::unique_ptr<IPDBLineNumber> Line = LineNumbers->getNext();
    assert(Line);

    DILineInfo LineInfo;
    LineInfo.FunctionName = Frame->getName();
    auto SourceFile = Session->getSourceFileById(Line->getSourceFileId());
    if (SourceFile &&
        Specifier.FLIKind != DILineInfoSpecifier::FileLineInfoKind::None)
      LineInfo.FileName = SourceFile->getFileName();
    LineInfo.Line = Line->getLineNumber();
    LineInfo.Column = Line->getColumnNumber();
    InlineInfo.addFrame(LineInfo);
  }

  InlineInfo.addFrame(CurrentLine);
  return InlineInfo;
}

std::vector<DILocal>
PDBContext::getLocalsForAddress(object::SectionedAddress Address) {
  return std::vector<DILocal>();
}

std::string PDBContext::getFunctionName(uint64_t Address,
                                        DINameKind NameKind) const {
  if (NameKind == DINameKind::None)
    return std::string();

  std::unique_ptr<PDBSymbol> FuncSymbol =
      Session->findSymbolByAddress(Address, PDB_SymType::Function);
  auto *Func = dyn_cast_or_null<PDBSymbolFunc>(FuncSymbol.get());

  if (NameKind == DINameKind::LinkageName) {
    // It is not possible to get the mangled linkage name through a
    // PDBSymbolFunc.  For that we have to specifically request a
    // PDBSymbolPublicSymbol.
    auto PublicSym =
        Session->findSymbolByAddress(Address, PDB_SymType::PublicSymbol);
    if (auto *PS = dyn_cast_or_null<PDBSymbolPublicSymbol>(PublicSym.get())) {
      // If we also have a function symbol, prefer the use of public symbol name
      // only if it refers to the same address. The public symbol uses the
      // linkage name while the function does not.
      if (!Func || Func->getVirtualAddress() == PS->getVirtualAddress())
        return PS->getName();
    }
  }

  return Func ? Func->getName() : std::string();
}
