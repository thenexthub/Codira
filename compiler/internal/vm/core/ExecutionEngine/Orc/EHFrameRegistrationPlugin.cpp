//===--------- EHFrameRegistrationPlugin.cpp - Register eh-frames ---------===//
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

#include "vm/core/ExecutionEngine/Orc/EHFrameRegistrationPlugin.h"

#include "vm/core/ExecutionEngine/JITLink/EHFrameSupport.h"
#include "vm/core/ExecutionEngine/Orc/Shared/MachOObjectFormat.h"
#include "vm/core/ExecutionEngine/Orc/Shared/OrcRTBridge.h"

#define DEBUG_TYPE "orc"

using namespace vm::core::jitlink;

namespace vm::core::orc {

Expected<std::unique_ptr<EHFrameRegistrationPlugin>>
EHFrameRegistrationPlugin::Create(ExecutionSession &ES) {
  // Lookup addresseses of the registration/deregistration functions in the
  // bootstrap map.
  ExecutorAddr RegisterEHFrameSectionAllocAction;
  ExecutorAddr DeregisterEHFrameSectionAllocAction;
  if (auto Err = ES.getExecutorProcessControl().getBootstrapSymbols(
          {{RegisterEHFrameSectionAllocAction,
            rt::RegisterEHFrameSectionAllocActionName},
           {DeregisterEHFrameSectionAllocAction,
            rt::DeregisterEHFrameSectionAllocActionName}}))
    return std::move(Err);

  return std::make_unique<EHFrameRegistrationPlugin>(
      RegisterEHFrameSectionAllocAction, DeregisterEHFrameSectionAllocAction);
}

void EHFrameRegistrationPlugin::modifyPassConfig(
    MaterializationResponsibility &MR, LinkGraph &LG,
    PassConfiguration &PassConfig) {
  if (LG.getTargetTriple().isOSBinFormatMachO())
    PassConfig.PrePrunePasses.insert(
        PassConfig.PrePrunePasses.begin(), [](LinkGraph &G) {
          if (auto *CUSec = G.findSectionByName(MachOCompactUnwindSectionName))
            G.removeSection(*CUSec);
          return Error::success();
        });

  PassConfig.PostFixupPasses.push_back([this](LinkGraph &G) -> Error {
    if (auto *EHFrame = getEHFrameSection(G)) {
      using namespace shared;
      auto R = SectionRange(*EHFrame).getRange();
      G.allocActions().push_back(
          {cantFail(
               WrapperFunctionCall::Create<SPSArgList<SPSExecutorAddrRange>>(
                   RegisterEHFrame, R)),
           cantFail(
               WrapperFunctionCall::Create<SPSArgList<SPSExecutorAddrRange>>(
                   DeregisterEHFrame, R))});
    }
    return Error::success();
  });
}

} // namespace vm::core::orc
