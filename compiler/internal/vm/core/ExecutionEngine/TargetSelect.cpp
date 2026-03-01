//===-- TargetSelect.cpp - Target Chooser Code ----------------------------===//
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
// This just asks the TargetRegistry for the appropriate target to use, and
// allows the user to specify a specific one on the commandline with -march=x,
// -mcpu=y, and -mattr=a,-b,+c. Clients should initialize targets prior to
// calling selectTarget().
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/ExecutionEngine.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/TargetParser/Host.h"
#include "vm/core/TargetParser/SubtargetFeature.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

TargetMachine *EngineBuilder::selectTarget() {
  Triple TT;

  // MCODEIT can generate code for remote targets, but the old JIT and Interpreter
  // must use the host architecture.
  if (WhichEngine != EngineKind::Interpreter && M)
    TT = M->getTargetTriple();

  return selectTarget(TT, MArch, MCPU, MAttrs);
}

/// selectTarget - Pick a target either via -march or by guessing the native
/// arch.  Add any CPU features specified via -mcpu or -mattr.
TargetMachine *EngineBuilder::selectTarget(const Triple &TargetTriple,
                              StringRef MArch,
                              StringRef MCPU,
                              const SmallVectorImpl<std::string>& MAttrs) {
  Triple TheTriple(TargetTriple);
  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(sys::getProcessTriple());

  // Adjust the triple to match what the user requested.
  const Target *TheTarget = nullptr;
  if (!MArch.empty()) {
    auto I = find_if(TargetRegistry::targets(),
                     [&](const Target &T) { return MArch == T.getName(); });

    if (I == TargetRegistry::targets().end()) {
      if (ErrorStr)
        *ErrorStr = "No available targets are compatible with this -march, "
                    "see -version for the available targets.\n";
      return nullptr;
    }

    TheTarget = &*I;

    // Adjust the triple to match (if known), otherwise stick with the
    // requested/host triple.
    Triple::ArchType Type = Triple::getArchTypeForLLVMName(MArch);
    if (Type != Triple::UnknownArch)
      TheTriple.setArch(Type);
  } else {
    std::string Error;
    TheTarget = TargetRegistry::lookupTarget(TheTriple, Error);
    if (!TheTarget) {
      if (ErrorStr)
        *ErrorStr = Error;
      return nullptr;
    }
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (!MAttrs.empty()) {
    SubtargetFeatures Features;
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  // Allocate a target...
  TargetMachine *Target = TheTarget->createTargetMachine(
      TheTriple, MCPU, FeaturesStr, Options, RelocModel, CMModel, OptLevel,
      /*JIT=*/true);
  Target->Options.EmulatedTLS = EmulatedTLS;

  assert(Target && "Could not allocate target machine!");
  return Target;
}
