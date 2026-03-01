//===------ PGOOptions.cpp -- PGO option tunables --------------*- C++ -*--===//
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

#include "vm/core/Support/PGOOptions.h"
#include "vm/core/Support/VirtualFileSystem.h"

using namespace vm::core;

PGOOptions::PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
                       std::string ProfileRemappingFile,
                       std::string MemoryProfile, PGOAction Action,
                       CSPGOAction CSAction, ColdFuncOpt ColdType,
                       bool DebugInfoForProfiling, bool PseudoProbeForProfiling,
                       bool AtomicCounterUpdate)
    : ProfileFile(ProfileFile), CSProfileGenFile(CSProfileGenFile),
      ProfileRemappingFile(ProfileRemappingFile), MemoryProfile(MemoryProfile),
      Action(Action), CSAction(CSAction), ColdOptType(ColdType),
      DebugInfoForProfiling(DebugInfoForProfiling ||
                            (Action == SampleUse && !PseudoProbeForProfiling)),
      PseudoProbeForProfiling(PseudoProbeForProfiling),
      AtomicCounterUpdate(AtomicCounterUpdate) {
  // Note, we do allow ProfileFile.empty() for Action=IRUse LTO can
  // callback with IRUse action without ProfileFile.

  // If there is a CSAction, PGOAction cannot be IRInstr or SampleUse.
  assert(this->CSAction == NoCSAction ||
         (this->Action != IRInstr && this->Action != SampleUse));

  // For CSIRInstr, CSProfileGenFile also needs to be nonempty.
  assert(this->CSAction != CSIRInstr || !this->CSProfileGenFile.empty());

  // If CSAction is CSIRUse, PGOAction needs to be IRUse as they share
  // a profile.
  assert(this->CSAction != CSIRUse || this->Action == IRUse);

  // Cannot optimize with MemProf profile during IR instrumentation.
  assert(this->MemoryProfile.empty() || this->Action != PGOOptions::IRInstr);

  // If neither Action nor CSAction nor MemoryProfile are set,
  // DebugInfoForProfiling or PseudoProbeForProfiling needs to be true.
  assert(this->Action != NoAction || this->CSAction != NoCSAction ||
         !this->MemoryProfile.empty() || this->DebugInfoForProfiling ||
         this->PseudoProbeForProfiling);
}

PGOOptions::PGOOptions(const PGOOptions &) = default;

PGOOptions &PGOOptions::operator=(const PGOOptions &O) = default;

PGOOptions::~PGOOptions() = default;
