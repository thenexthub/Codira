//===- PDBSymbolFunc.cpp - --------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/PDBSymbolFunc.h"

#include "vm/core/DebugInfo/PDB/ConcreteSymbolEnumerator.h"
#include "vm/core/DebugInfo/PDB/IPDBEnumChildren.h"
#include "vm/core/DebugInfo/PDB/IPDBLineNumber.h"
#include "vm/core/DebugInfo/PDB/IPDBSession.h"
#include "vm/core/DebugInfo/PDB/PDBSymDumper.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolData.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFunctionSig.h"
#include "vm/core/DebugInfo/PDB/PDBTypes.h"

#include <unordered_set>
#include <utility>
#include <vector>

using namespace vm::core;
using namespace vm::core::pdb;

namespace {
class FunctionArgEnumerator : public IPDBEnumChildren<PDBSymbolData> {
public:
  typedef ConcreteSymbolEnumerator<PDBSymbolData> ArgEnumeratorType;

  FunctionArgEnumerator(const IPDBSession &PDBSession,
                        const PDBSymbolFunc &PDBFunc)
      : Session(PDBSession), Func(PDBFunc) {
    // Arguments can appear multiple times if they have live range
    // information, so we only take the first occurrence.
    std::unordered_set<std::string> SeenNames;
    auto DataChildren = Func.findAllChildren<PDBSymbolData>();
    while (auto Child = DataChildren->getNext()) {
      if (Child->getDataKind() == PDB_DataKind::Param) {
        if (SeenNames.insert(Child->getName()).second)
          Args.push_back(std::move(Child));
      }
    }
    reset();
  }

  uint32_t getChildCount() const override { return Args.size(); }

  std::unique_ptr<PDBSymbolData>
  getChildAtIndex(uint32_t Index) const override {
    if (Index >= Args.size())
      return nullptr;

    return Session.getConcreteSymbolById<PDBSymbolData>(
        Args[Index]->getSymIndexId());
  }

  std::unique_ptr<PDBSymbolData> getNext() override {
    if (CurIter == Args.end())
      return nullptr;
    const auto &Result = **CurIter;
    ++CurIter;
    return Session.getConcreteSymbolById<PDBSymbolData>(Result.getSymIndexId());
  }

  void reset() override { CurIter = Args.empty() ? Args.end() : Args.begin(); }

private:
  typedef std::vector<std::unique_ptr<PDBSymbolData>> ArgListType;
  const IPDBSession &Session;
  const PDBSymbolFunc &Func;
  ArgListType Args;
  ArgListType::const_iterator CurIter;
};
}

std::unique_ptr<IPDBEnumChildren<PDBSymbolData>>
PDBSymbolFunc::getArguments() const {
  return std::make_unique<FunctionArgEnumerator>(Session, *this);
}

void PDBSymbolFunc::dump(PDBSymDumper &Dumper) const { Dumper.dump(*this); }

bool PDBSymbolFunc::isDestructor() const {
  std::string Name = getName();
  if (Name.empty())
    return false;
  if (Name[0] == '~')
    return true;
  if (Name == "__vecDelDtor")
    return true;
  return false;
}

std::unique_ptr<IPDBEnumLineNumbers> PDBSymbolFunc::getLineNumbers() const {
  auto Len = RawSymbol->getLength();
  return Session.findLineNumbersByAddress(RawSymbol->getVirtualAddress(),
                                          Len ? Len : 1);
}

uint32_t PDBSymbolFunc::getCompilandId() const {
  if (auto Lines = getLineNumbers()) {
    if (auto FirstLine = Lines->getNext()) {
      return FirstLine->getCompilandId();
    }
  }
  return 0;
}
