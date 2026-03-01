//===----- UnwindInfoRegistrationPlugin.cpp - libunwind registration ------===//
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

#include "vm/core/ExecutionEngine/Orc/UnwindInfoRegistrationPlugin.h"

#include "vm/core/ExecutionEngine/Orc/Shared/MachOObjectFormat.h"
#include "vm/core/ExecutionEngine/Orc/Shared/OrcRTBridge.h"
#include "vm/core/IR/Module.h"

#define DEBUG_TYPE "orc"

using namespace vm::core::jitlink;

namespace vm::core::orc {

Expected<std::shared_ptr<UnwindInfoRegistrationPlugin>>
UnwindInfoRegistrationPlugin::Create(ExecutionSession &ES) {

  ExecutorAddr Register, Deregister;

  auto &EPC = ES.getExecutorProcessControl();
  if (auto Err = EPC.getBootstrapSymbols(
          {{Register, rt_alt::UnwindInfoManagerRegisterActionName},
           {Deregister, rt_alt::UnwindInfoManagerDeregisterActionName}}))
    return std::move(Err);

  return std::make_shared<UnwindInfoRegistrationPlugin>(ES, Register,
                                                        Deregister);
}

void UnwindInfoRegistrationPlugin::modifyPassConfig(
    MaterializationResponsibility &MR, LinkGraph &G,
    PassConfiguration &PassConfig) {

  PassConfig.PostFixupPasses.push_back(
      [this](LinkGraph &G) { return addUnwindInfoRegistrationActions(G); });
}

Error UnwindInfoRegistrationPlugin::addUnwindInfoRegistrationActions(
    LinkGraph &G) {
  ExecutorAddrRange EHFrameRange, UnwindInfoRange;

  std::vector<Block *> CodeBlocks;

  auto ScanUnwindInfoSection = [&](Section &Sec, ExecutorAddrRange &SecRange) {
    if (Sec.empty())
      return;

    SecRange.Start = (*Sec.blocks().begin())->getAddress();
    for (auto *B : Sec.blocks()) {
      auto R = B->getRange();
      SecRange.Start = std::min(SecRange.Start, R.Start);
      SecRange.End = std::max(SecRange.End, R.End);
      for (auto &E : B->edges()) {
        if (E.getKind() != Edge::KeepAlive || !E.getTarget().isDefined())
          continue;
        auto &TargetBlock = E.getTarget().getBlock();
        auto &TargetSection = TargetBlock.getSection();
        if ((TargetSection.getMemProt() & MemProt::Exec) == MemProt::Exec)
          CodeBlocks.push_back(&TargetBlock);
      }
    }
  };

  if (auto *EHFrame = G.findSectionByName(MachOEHFrameSectionName))
    ScanUnwindInfoSection(*EHFrame, EHFrameRange);

  if (auto *UnwindInfo = G.findSectionByName(MachOUnwindInfoSectionName))
    ScanUnwindInfoSection(*UnwindInfo, UnwindInfoRange);

  if (CodeBlocks.empty())
    return Error::success();

  if ((EHFrameRange == ExecutorAddrRange() &&
       UnwindInfoRange == ExecutorAddrRange()))
    return Error::success();

  toolchain::sort(CodeBlocks, [](const Block *LHS, const Block *RHS) {
    return LHS->getAddress() < RHS->getAddress();
  });

  SmallVector<ExecutorAddrRange> CodeRanges;
  for (auto *B : CodeBlocks) {
    if (CodeRanges.empty() || CodeRanges.back().End != B->getAddress())
      CodeRanges.push_back(B->getRange());
    else
      CodeRanges.back().End = B->getRange().End;
  }

  ExecutorAddr DSOBase;
  if (auto *DSOBaseSym = G.findAbsoluteSymbolByName(DSOBaseName))
    DSOBase = DSOBaseSym->getAddress();
  else if (auto *DSOBaseSym = G.findExternalSymbolByName(DSOBaseName))
    DSOBase = DSOBaseSym->getAddress();
  else if (auto *DSOBaseSym = G.findDefinedSymbolByName(DSOBaseName))
    DSOBase = DSOBaseSym->getAddress();
  else
    return make_error<StringError>("In " + G.getName() +
                                       " could not find dso base symbol",
                                   inconvertibleErrorCode());

  using namespace shared;
  using SPSRegisterArgs =
      SPSArgList<SPSSequence<SPSExecutorAddrRange>, SPSExecutorAddr,
                 SPSExecutorAddrRange, SPSExecutorAddrRange>;
  using SPSDeregisterArgs = SPSArgList<SPSSequence<SPSExecutorAddrRange>>;

  G.allocActions().push_back(
      {cantFail(WrapperFunctionCall::Create<SPSRegisterArgs>(
           Register, CodeRanges, DSOBase, EHFrameRange, UnwindInfoRange)),
       cantFail(WrapperFunctionCall::Create<SPSDeregisterArgs>(Deregister,
                                                               CodeRanges))});

  return Error::success();
}

} // namespace vm::core::orc
