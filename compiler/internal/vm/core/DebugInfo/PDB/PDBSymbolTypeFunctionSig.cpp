//===- PDBSymbolTypeFunctionSig.cpp - --------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFunctionSig.h"

#include "vm/core/DebugInfo/PDB/ConcreteSymbolEnumerator.h"
#include "vm/core/DebugInfo/PDB/IPDBEnumChildren.h"
#include "vm/core/DebugInfo/PDB/IPDBSession.h"
#include "vm/core/DebugInfo/PDB/PDBSymDumper.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeBuiltin.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolTypeFunctionArg.h"

#include <utility>

using namespace vm::core;
using namespace vm::core::pdb;

namespace {
class FunctionArgEnumerator : public IPDBEnumSymbols {
public:
  typedef ConcreteSymbolEnumerator<PDBSymbolTypeFunctionArg> ArgEnumeratorType;

  FunctionArgEnumerator(const IPDBSession &PDBSession,
                        const PDBSymbolTypeFunctionSig &Sig)
      : Session(PDBSession),
        Enumerator(Sig.findAllChildren<PDBSymbolTypeFunctionArg>()) {}

  FunctionArgEnumerator(const IPDBSession &PDBSession,
                        std::unique_ptr<ArgEnumeratorType> ArgEnumerator)
      : Session(PDBSession), Enumerator(std::move(ArgEnumerator)) {}

  uint32_t getChildCount() const override {
    return Enumerator->getChildCount();
  }

  std::unique_ptr<PDBSymbol> getChildAtIndex(uint32_t Index) const override {
    auto FunctionArgSymbol = Enumerator->getChildAtIndex(Index);
    if (!FunctionArgSymbol)
      return nullptr;
    return Session.getSymbolById(FunctionArgSymbol->getTypeId());
  }

  std::unique_ptr<PDBSymbol> getNext() override {
    auto FunctionArgSymbol = Enumerator->getNext();
    if (!FunctionArgSymbol)
      return nullptr;
    return Session.getSymbolById(FunctionArgSymbol->getTypeId());
  }

  void reset() override { Enumerator->reset(); }

private:
  const IPDBSession &Session;
  std::unique_ptr<ArgEnumeratorType> Enumerator;
};
}

std::unique_ptr<IPDBEnumSymbols>
PDBSymbolTypeFunctionSig::getArguments() const {
  return std::make_unique<FunctionArgEnumerator>(Session, *this);
}

void PDBSymbolTypeFunctionSig::dump(PDBSymDumper &Dumper) const {
  Dumper.dump(*this);
}

void PDBSymbolTypeFunctionSig::dumpRight(PDBSymDumper &Dumper) const {
  Dumper.dumpRight(*this);
}

bool PDBSymbolTypeFunctionSig::isCVarArgs() const {
  auto SigArguments = getArguments();
  if (!SigArguments)
    return false;
  uint32_t NumArgs = SigArguments->getChildCount();
  if (NumArgs == 0)
    return false;
  auto Last = SigArguments->getChildAtIndex(NumArgs - 1);
  if (auto Builtin = toolchain::dyn_cast_or_null<PDBSymbolTypeBuiltin>(Last.get())) {
    if (Builtin->getBuiltinType() == PDB_BuiltinType::None)
      return true;
  }

  // Note that for a variadic template signature, this method always returns
  // false since the parameters of the template are specialized.
  return false;
}
