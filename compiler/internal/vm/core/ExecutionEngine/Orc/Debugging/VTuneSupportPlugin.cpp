//===--- VTuneSupportPlugin.cpp -- Support for VTune profiler --*- C++ -*--===//
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
//
// Handles support for registering code with VIntel Tune's Amplfiier JIT API.
//
//===----------------------------------------------------------------------===//
#include "vm/core/ExecutionEngine/Orc/Debugging/VTuneSupportPlugin.h"
#include "vm/core/DebugInfo/DWARF/DWARFContext.h"
#include "vm/core/ExecutionEngine/Orc/Debugging/DebugInfoSupport.h"

using namespace vm::core;
using namespace vm::core::orc;
using namespace vm::core::jitlink;

static constexpr StringRef RegisterVTuneImplName = "llvm_orc_registerVTuneImpl";
static constexpr StringRef UnregisterVTuneImplName =
    "llvm_orc_unregisterVTuneImpl";
static constexpr StringRef RegisterTestVTuneImplName =
    "llvm_orc_test_registerVTuneImpl";

static VTuneMethodBatch getMethodBatch(LinkGraph &G, bool EmitDebugInfo) {
  VTuneMethodBatch Batch;
  std::unique_ptr<DWARFContext> DC;
  StringMap<std::unique_ptr<MemoryBuffer>> DCBacking;
  if (EmitDebugInfo) {
    auto EDC = createDWARFContext(G);
    if (!EDC) {
      EmitDebugInfo = false;
    } else {
      DC = std::move(EDC->first);
      DCBacking = std::move(EDC->second);
    }
  }

  auto GetStringIdx = [Deduplicator = StringMap<uint32_t>(),
                       &Batch](StringRef S) mutable {
    auto [I, Inserted] = Deduplicator.try_emplace(S);
    if (Inserted) {
      Batch.Strings.push_back(S.str());
      I->second = Batch.Strings.size();
    }
    return I->second;
  };
  for (auto Sym : G.defined_symbols()) {
    if (!Sym->isCallable())
      continue;

    Batch.Methods.push_back(VTuneMethodInfo());
    auto &Method = Batch.Methods.back();
    Method.MethodID = 0;
    Method.ParentMI = 0;
    Method.LoadAddr = Sym->getAddress();
    Method.LoadSize = Sym->getSize();
    Method.NameSI = GetStringIdx(*Sym->getName());
    Method.ClassFileSI = 0;
    Method.SourceFileSI = 0;

    if (!EmitDebugInfo)
      continue;

    auto &Section = Sym->getSection();
    auto Addr = Sym->getAddress();
    auto SAddr =
        object::SectionedAddress{Addr.getValue(), Section.getOrdinal()};
    DILineInfoTable LinesInfo = DC->getLineInfoForAddressRange(
        SAddr, Sym->getSize(),
        DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath);
    Method.SourceFileSI = Batch.Strings.size();
    Batch.Strings.push_back(
        DC->getLineInfoForAddress(SAddr).value_or(DILineInfo()).FileName);
    for (auto &LInfo : LinesInfo) {
      Method.LineTable.push_back(
          std::pair<unsigned, unsigned>{/*unsigned*/ Sym->getOffset(),
                                        /*DILineInfo*/ LInfo.second.Line});
    }
  }
  return Batch;
}

void VTuneSupportPlugin::modifyPassConfig(MaterializationResponsibility &MR,
                                          LinkGraph &G,
                                          PassConfiguration &Config) {
  Config.PostFixupPasses.push_back([this, MR = &MR](LinkGraph &G) {
    // the object file is generated but not linked yet
    auto Batch = getMethodBatch(G, EmitDebugInfo);
    if (Batch.Methods.empty()) {
      return Error::success();
    }
    {
      std::lock_guard<std::mutex> Lock(PluginMutex);
      uint64_t Allocated = Batch.Methods.size();
      uint64_t Start = NextMethodID;
      NextMethodID += Allocated;
      for (size_t i = Start; i < NextMethodID; ++i) {
        Batch.Methods[i - Start].MethodID = i;
      }
      this->PendingMethodIDs[MR] = {Start, Allocated};
    }
    G.allocActions().push_back(
        {cantFail(shared::WrapperFunctionCall::Create<
                  shared::SPSArgList<shared::SPSVTuneMethodBatch>>(
             RegisterVTuneImplAddr, Batch)),
         {}});
    return Error::success();
  });
}

Error VTuneSupportPlugin::notifyEmitted(MaterializationResponsibility &MR) {
  if (auto Err = MR.withResourceKeyDo([this, MR = &MR](ResourceKey K) {
        std::lock_guard<std::mutex> Lock(PluginMutex);
        auto I = PendingMethodIDs.find(MR);
        if (I == PendingMethodIDs.end())
          return;

        LoadedMethodIDs[K].push_back(I->second);
        PendingMethodIDs.erase(I);
      })) {
    return Err;
  }
  return Error::success();
}

Error VTuneSupportPlugin::notifyFailed(MaterializationResponsibility &MR) {
  std::lock_guard<std::mutex> Lock(PluginMutex);
  PendingMethodIDs.erase(&MR);
  return Error::success();
}

Error VTuneSupportPlugin::notifyRemovingResources(JITDylib &JD, ResourceKey K) {
  // Unregistration not required if not provided
  if (!UnregisterVTuneImplAddr) {
    return Error::success();
  }
  VTuneUnloadedMethodIDs UnloadedIDs;
  {
    std::lock_guard<std::mutex> Lock(PluginMutex);
    auto I = LoadedMethodIDs.find(K);
    if (I == LoadedMethodIDs.end())
      return Error::success();

    UnloadedIDs = std::move(I->second);
    LoadedMethodIDs.erase(I);
  }
  if (auto Err = EPC.callSPSWrapper<void(shared::SPSVTuneUnloadedMethodIDs)>(
          UnregisterVTuneImplAddr, UnloadedIDs))
    return Err;

  return Error::success();
}

void VTuneSupportPlugin::notifyTransferringResources(JITDylib &JD,
                                                     ResourceKey DstKey,
                                                     ResourceKey SrcKey) {
  std::lock_guard<std::mutex> Lock(PluginMutex);
  auto I = LoadedMethodIDs.find(SrcKey);
  if (I == LoadedMethodIDs.end())
    return;

  auto &Dest = LoadedMethodIDs[DstKey];
  toolchain::append_range(Dest, I->second);
  LoadedMethodIDs.erase(SrcKey);
}

Expected<std::unique_ptr<VTuneSupportPlugin>>
VTuneSupportPlugin::Create(ExecutorProcessControl &EPC, JITDylib &JD,
                           bool EmitDebugInfo, bool TestMode) {
  auto &ES = EPC.getExecutionSession();
  auto RegisterImplName =
      ES.intern(TestMode ? RegisterTestVTuneImplName : RegisterVTuneImplName);
  auto UnregisterImplName = ES.intern(UnregisterVTuneImplName);
  SymbolLookupSet SLS{RegisterImplName, UnregisterImplName};
  auto Res = ES.lookup(makeJITDylibSearchOrder({&JD}), std::move(SLS));
  if (!Res)
    return Res.takeError();
  ExecutorAddr RegisterImplAddr(
      Res->find(RegisterImplName)->second.getAddress());
  ExecutorAddr UnregisterImplAddr(
      Res->find(UnregisterImplName)->second.getAddress());
  return std::make_unique<VTuneSupportPlugin>(
      EPC, RegisterImplAddr, UnregisterImplAddr, EmitDebugInfo);
}
