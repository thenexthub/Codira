//===------ DebuggerSupport.cpp - Utils for enabling debugger support -----===//
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

#include "vm/core/ExecutionEngine/Orc/Debugging/DebuggerSupport.h"
#include "vm/core/ExecutionEngine/Orc/Debugging/DebuggerSupportPlugin.h"
#include "vm/core/ExecutionEngine/Orc/Debugging/ELFDebugObjectPlugin.h"
#include "vm/core/ExecutionEngine/Orc/LLJIT.h"

#define DEBUG_TYPE "orc"

using namespace vm::core;
using namespace vm::core::orc;

namespace vm::core::orc {

Error enableDebuggerSupport(LLJIT &J) {
  auto *ObjLinkingLayer = dyn_cast<ObjectLinkingLayer>(&J.getObjLinkingLayer());
  if (!ObjLinkingLayer)
    return make_error<StringError>("Cannot enable LLJIT debugger support: "
                                   "Debugger support requires JITLink",
                                   inconvertibleErrorCode());
  auto ProcessSymsJD = J.getProcessSymbolsJITDylib();
  if (!ProcessSymsJD)
    return make_error<StringError>("Cannot enable LLJIT debugger support: "
                                   "Process symbols are not available",
                                   inconvertibleErrorCode());

  auto &ES = J.getExecutionSession();
  const auto &TT = J.getTargetTriple();

  switch (TT.getObjectFormat()) {
  case Triple::ELF: {
    Error TargetSymErr = Error::success();
    ObjLinkingLayer->addPlugin(
        std::make_unique<ELFDebugObjectPlugin>(ES, false, true, TargetSymErr));
    return TargetSymErr;
  }
  case Triple::MachO: {
    auto DS = GDBJITDebugInfoRegistrationPlugin::Create(ES, *ProcessSymsJD, TT);
    if (!DS)
      return DS.takeError();
    ObjLinkingLayer->addPlugin(std::move(*DS));
    return Error::success();
  }
  default:
    return make_error<StringError>(
        "Cannot enable LLJIT debugger support: " +
            Triple::getObjectFormatTypeName(TT.getObjectFormat()) +
            " is not supported",
        inconvertibleErrorCode());
  }
}

} // namespace vm::core::orc
