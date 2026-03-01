//===----- CompileOnDemandLayer.cpp - Lazily emit IR on first call --------===//
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

#include "vm/core/ExecutionEngine/Orc/CompileOnDemandLayer.h"
#include "vm/core/ExecutionEngine/Orc/Layer.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;
using namespace vm::core::orc;

CompileOnDemandLayer::CompileOnDemandLayer(
    ExecutionSession &ES, IRLayer &BaseLayer, LazyCallThroughManager &LCTMgr,
    IndirectStubsManagerBuilder BuildIndirectStubsManager)
    : IRLayer(ES, BaseLayer.getManglingOptions()), BaseLayer(BaseLayer),
      LCTMgr(LCTMgr),
      BuildIndirectStubsManager(std::move(BuildIndirectStubsManager)) {}

void CompileOnDemandLayer::setImplMap(ImplSymbolMap *Imp) {
  this->AliaseeImpls = Imp;
}

void CompileOnDemandLayer::emit(
    std::unique_ptr<MaterializationResponsibility> R, ThreadSafeModule TSM) {
  assert(TSM && "Null module");

  auto &ES = getExecutionSession();

  // Sort the callables and non-callables, build re-exports and lodge the
  // actual module with the implementation dylib.
  auto &PDR = getPerDylibResources(R->getTargetJITDylib());

  SymbolAliasMap NonCallables;
  SymbolAliasMap Callables;

  for (auto &KV : R->getSymbols()) {
    auto &Name = KV.first;
    auto &Flags = KV.second;
    if (Flags.isCallable())
      Callables[Name] = SymbolAliasMapEntry(Name, Flags);
    else
      NonCallables[Name] = SymbolAliasMapEntry(Name, Flags);
  }

  // Lodge symbols with the implementation dylib.
  if (auto Err = PDR.getImplDylib().define(
          std::make_unique<BasicIRLayerMaterializationUnit>(
              BaseLayer, *getManglingOptions(), std::move(TSM)))) {
    ES.reportError(std::move(Err));
    R->failMaterialization();
    return;
  }

  if (!NonCallables.empty())
    if (auto Err =
            R->replace(reexports(PDR.getImplDylib(), std::move(NonCallables),
                                 JITDylibLookupFlags::MatchAllSymbols))) {
      getExecutionSession().reportError(std::move(Err));
      R->failMaterialization();
      return;
    }
  if (!Callables.empty()) {
    if (auto Err = R->replace(
            lazyReexports(LCTMgr, PDR.getISManager(), PDR.getImplDylib(),
                          std::move(Callables), AliaseeImpls))) {
      getExecutionSession().reportError(std::move(Err));
      R->failMaterialization();
      return;
    }
  }
}

CompileOnDemandLayer::PerDylibResources &
CompileOnDemandLayer::getPerDylibResources(JITDylib &TargetD) {
  std::lock_guard<std::mutex> Lock(CODLayerMutex);

  auto I = DylibResources.find(&TargetD);
  if (I == DylibResources.end()) {
    auto &ImplD =
        getExecutionSession().createBareJITDylib(TargetD.getName() + ".impl");
    JITDylibSearchOrder NewLinkOrder;
    TargetD.withLinkOrderDo([&](const JITDylibSearchOrder &TargetLinkOrder) {
      NewLinkOrder = TargetLinkOrder;
    });

    assert(!NewLinkOrder.empty() && NewLinkOrder.front().first == &TargetD &&
           NewLinkOrder.front().second ==
               JITDylibLookupFlags::MatchAllSymbols &&
           "TargetD must be at the front of its own search order and match "
           "non-exported symbol");
    NewLinkOrder.insert(std::next(NewLinkOrder.begin()),
                        {&ImplD, JITDylibLookupFlags::MatchAllSymbols});
    ImplD.setLinkOrder(NewLinkOrder, false);
    TargetD.setLinkOrder(std::move(NewLinkOrder), false);

    PerDylibResources PDR(ImplD, BuildIndirectStubsManager());
    I = DylibResources.insert(std::make_pair(&TargetD, std::move(PDR))).first;
  }

  return I->second;
}
