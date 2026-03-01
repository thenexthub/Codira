//===----- LinkGraphLayer.cpp - Add LinkGraphs to an ExecutionSession -----===//
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

#include "vm/core/ExecutionEngine/Orc/LinkGraphLayer.h"

#include "vm/core/ExecutionEngine/JITLink/JITLink.h"
#include "vm/core/ExecutionEngine/Orc/Shared/MachOObjectFormat.h"
#include "vm/core/ExecutionEngine/Orc/Shared/ObjectFormats.h"

#define DEBUG_TYPE "orc"

using namespace vm::core;
using namespace vm::core::jitlink;
using namespace vm::core::orc;

namespace {

bool hasInitializerSection(LinkGraph &G) {
  bool IsMachO = G.getTargetTriple().isOSBinFormatMachO();
  bool IsElf = G.getTargetTriple().isOSBinFormatELF();
  if (!IsMachO && !IsElf)
    return false;

  for (auto &Sec : G.sections()) {
    if (IsMachO && isMachOInitializerSection(Sec.getName()))
      return true;
    if (IsElf && isELFInitializerSection(Sec.getName()))
      return true;
  }

  return false;
}

} // end anonymous namespace

namespace vm::core::orc {

LinkGraphLayer::~LinkGraphLayer() = default;

MaterializationUnit::Interface LinkGraphLayer::getInterface(LinkGraph &G) {

  MaterializationUnit::Interface LGI;

  auto AddSymbol = [&](Symbol *Sym) {
    // Skip local symbols.
    if (Sym->getScope() == Scope::Local)
      return;
    assert(Sym->hasName() && "Anonymous non-local symbol?");

    LGI.SymbolFlags[Sym->getName()] = getJITSymbolFlagsForSymbol(*Sym);
  };

  for (auto *Sym : G.defined_symbols())
    AddSymbol(Sym);
  for (auto *Sym : G.absolute_symbols())
    AddSymbol(Sym);

  if (hasInitializerSection(G)) {
    std::string InitSymString;
    {
      raw_string_ostream(InitSymString)
          << "$." << G.getName() << ".__inits" << Counter++;
    }
    LGI.InitSymbol = ES.intern(InitSymString);
  }

  return LGI;
}

JITSymbolFlags LinkGraphLayer::getJITSymbolFlagsForSymbol(Symbol &Sym) {
  JITSymbolFlags Flags;

  if (Sym.getLinkage() == Linkage::Weak)
    Flags |= JITSymbolFlags::Weak;

  if (Sym.getScope() == Scope::Default)
    Flags |= JITSymbolFlags::Exported;
  else if (Sym.getScope() == Scope::SideEffectsOnly)
    Flags |= JITSymbolFlags::MaterializationSideEffectsOnly;

  if (Sym.isCallable())
    Flags |= JITSymbolFlags::Callable;

  return Flags;
}

StringRef LinkGraphMaterializationUnit::getName() const { return G->getName(); }

void LinkGraphMaterializationUnit::discard(const JITDylib &JD,
                                           const SymbolStringPtr &Name) {
  for (auto *Sym : G->defined_symbols())
    if (Sym->getName() == Name) {
      assert(Sym->getLinkage() == Linkage::Weak &&
             "Discarding non-weak definition");
      G->makeExternal(*Sym);
      break;
    }
}

} // namespace vm::core::orc
