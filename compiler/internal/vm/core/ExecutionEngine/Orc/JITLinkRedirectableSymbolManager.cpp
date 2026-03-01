//===-- JITLinkRedirectableSymbolManager.cpp - JITLink redirection in Orc -===//
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

#include "vm/core/ExecutionEngine/Orc/JITLinkRedirectableSymbolManager.h"
#include "vm/core/ExecutionEngine/Orc/Core.h"

#define DEBUG_TYPE "orc"

using namespace vm::core;
using namespace vm::core::orc;

namespace {
constexpr StringRef JumpStubSectionName = "__orc_stubs";
constexpr StringRef StubPtrSectionName = "__orc_stub_ptrs";
constexpr StringRef StubSuffix = "$__stub_ptr";
} // namespace

void JITLinkRedirectableSymbolManager::emitRedirectableSymbols(
    std::unique_ptr<MaterializationResponsibility> R, SymbolMap InitialDests) {

  auto &ES = ObjLinkingLayer.getExecutionSession();
  auto G = std::make_unique<jitlink::LinkGraph>(
      ("<indirect stubs graph #" + Twine(++StubGraphIdx) + ">").str(),
      ES.getSymbolStringPool(), ES.getTargetTriple(), SubtargetFeatures(),
      jitlink::getGenericEdgeKindName);
  auto &PointerSection =
      G->createSection(StubPtrSectionName, MemProt::Write | MemProt::Read);
  auto &StubsSection =
      G->createSection(JumpStubSectionName, MemProt::Exec | MemProt::Read);

  SymbolFlagsMap NewSymbols;
  for (auto &[Name, Def] : InitialDests) {
    jitlink::Symbol *TargetSym = nullptr;
    if (Def.getAddress())
      TargetSym = &G->addAbsoluteSymbol(
          G->allocateName(*Name + "$__init_tgt"), Def.getAddress(), 0,
          jitlink::Linkage::Strong, jitlink::Scope::Local, false);

    auto PtrName = ES.intern((*Name + StubSuffix).str());
    auto &Ptr = AnonymousPtrCreator(*G, PointerSection, TargetSym, 0);
    Ptr.setName(PtrName);
    Ptr.setScope(jitlink::Scope::Hidden);
    auto &Stub = PtrJumpStubCreator(*G, StubsSection, Ptr);
    Stub.setName(Name);
    Stub.setScope(Def.getFlags().isExported() ? jitlink::Scope::Default
                                              : jitlink::Scope::Hidden);
    Stub.setLinkage(!Def.getFlags().isWeak() ? jitlink::Linkage::Strong
                                             : jitlink::Linkage::Weak);
    NewSymbols[std::move(PtrName)] = JITSymbolFlags();
  }

  // Try to claim responsibility for the new stub symbols.
  if (auto Err = R->defineMaterializing(std::move(NewSymbols))) {
    ES.reportError(std::move(Err));
    return R->failMaterialization();
  }

  ObjLinkingLayer.emit(std::move(R), std::move(G));
}

Error JITLinkRedirectableSymbolManager::redirect(JITDylib &JD,
                                                 const SymbolMap &NewDests) {
  auto &ES = ObjLinkingLayer.getExecutionSession();
  SymbolLookupSet LS;
  DenseMap<NonOwningSymbolStringPtr, SymbolStringPtr> PtrToStub;
  for (auto &[StubName, Sym] : NewDests) {
    auto PtrName = ES.intern((*StubName + StubSuffix).str());
    PtrToStub[NonOwningSymbolStringPtr(PtrName)] = StubName;
    LS.add(std::move(PtrName));
  }
  auto PtrSyms =
      ES.lookup({{&JD, JITDylibLookupFlags::MatchAllSymbols}}, std::move(LS));
  if (!PtrSyms)
    return PtrSyms.takeError();

  std::vector<tpctypes::PointerWrite> PtrWrites;
  for (auto &[PtrName, PtrSym] : *PtrSyms) {
    auto DestSymI = NewDests.find(PtrToStub[NonOwningSymbolStringPtr(PtrName)]);
    assert(DestSymI != NewDests.end() && "Bad ptr -> stub mapping");
    auto &DestSym = DestSymI->second;
    PtrWrites.push_back({PtrSym.getAddress(), DestSym.getAddress()});
  }

  return ObjLinkingLayer.getExecutionSession()
      .getExecutorProcessControl()
      .getMemoryAccess()
      .writePointers(PtrWrites);
}
