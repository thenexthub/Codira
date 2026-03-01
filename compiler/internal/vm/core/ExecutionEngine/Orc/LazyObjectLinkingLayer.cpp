//===---------- LazyReexports.cpp - Utilities for lazy reexports ----------===//
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

#include "vm/core/ExecutionEngine/Orc/LazyObjectLinkingLayer.h"

#include "vm/core/ExecutionEngine/Orc/LazyReexports.h"
#include "vm/core/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "vm/core/ExecutionEngine/Orc/RedirectionManager.h"

using namespace vm::core;
using namespace vm::core::jitlink;

namespace {

constexpr StringRef FnBodySuffix = "$orc_fnbody";

} // anonymous namespace

namespace vm::core::orc {

class LazyObjectLinkingLayer::RenamerPlugin
    : public ObjectLinkingLayer::Plugin {
public:
  void modifyPassConfig(MaterializationResponsibility &MR,
                        jitlink::LinkGraph &LG,
                        jitlink::PassConfiguration &Config) override {
    // We need to insert this before the mark-live pass to ensure that we don't
    // delete the bodies (their names won't match the responsibility set until
    // after this pass completes.
    Config.PrePrunePasses.insert(
        Config.PrePrunePasses.begin(),
        [&MR](LinkGraph &G) { return renameFunctionBodies(G, MR); });
  }

  Error notifyFailed(MaterializationResponsibility &MR) override {
    return Error::success();
  }

  Error notifyRemovingResources(JITDylib &JD, ResourceKey K) override {
    return Error::success();
  }

  void notifyTransferringResources(JITDylib &JD, ResourceKey DstKey,
                                   ResourceKey SrcKey) override {}

private:
  static Error renameFunctionBodies(LinkGraph &G,
                                    MaterializationResponsibility &MR) {
    DenseMap<StringRef, NonOwningSymbolStringPtr> SymsToRename;
    for (auto &[Name, Flags] : MR.getSymbols())
      if ((*Name).ends_with(FnBodySuffix))
        SymsToRename[(*Name).drop_back(FnBodySuffix.size())] =
            NonOwningSymbolStringPtr(Name);

    for (auto *Sym : G.defined_symbols()) {
      if (!Sym->hasName())
        continue;
      auto I = SymsToRename.find(*Sym->getName());
      if (I == SymsToRename.end())
        continue;
      Sym->setName(G.intern(G.allocateName(*I->second)));
    }

    return Error::success();
  }
};

LazyObjectLinkingLayer::LazyObjectLinkingLayer(ObjectLinkingLayer &BaseLayer,
                                               LazyReexportsManager &LRMgr)
    : ObjectLayer(BaseLayer.getExecutionSession()), BaseLayer(BaseLayer),
      LRMgr(LRMgr) {
  BaseLayer.addPlugin(std::make_unique<RenamerPlugin>());
}

Error LazyObjectLinkingLayer::add(ResourceTrackerSP RT,
                                  std::unique_ptr<MemoryBuffer> O,
                                  MaterializationUnit::Interface I) {

  // Object files with initializer symbols can't be lazy.
  if (I.InitSymbol)
    return BaseLayer.add(std::move(RT), std::move(O), std::move(I));

  auto &ES = getExecutionSession();
  SymbolAliasMap LazySymbols;
  for (auto &[Name, Flags] : I.SymbolFlags)
    if (Flags.isCallable())
      LazySymbols[Name] = {ES.intern((*Name + FnBodySuffix).str()), Flags};

  for (auto &[Name, AI] : LazySymbols) {
    I.SymbolFlags.erase(Name);
    I.SymbolFlags[AI.Aliasee] = AI.AliasFlags;
  }

  if (auto Err = BaseLayer.add(RT, std::move(O), std::move(I)))
    return Err;

  auto &JD = RT->getJITDylib();
  return JD.define(lazyReexports(LRMgr, std::move(LazySymbols)), std::move(RT));
}

void LazyObjectLinkingLayer::emit(
    std::unique_ptr<MaterializationResponsibility> MR,
    std::unique_ptr<MemoryBuffer> Obj) {
  return BaseLayer.emit(std::move(MR), std::move(Obj));
}

} // namespace vm::core::orc
