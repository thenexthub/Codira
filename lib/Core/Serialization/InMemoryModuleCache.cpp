//===- InMemoryModuleCache.cpp - Cache for loaded memory buffers ----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Core/Serialization/InMemoryModuleCache.h"
#include "toolchain/Support/MemoryBuffer.h"

using namespace language::Core;

InMemoryModuleCache::State
InMemoryModuleCache::getPCMState(toolchain::StringRef Filename) const {
  auto I = PCMs.find(Filename);
  if (I == PCMs.end())
    return Unknown;
  if (I->second.IsFinal)
    return Final;
  return I->second.Buffer ? Tentative : ToBuild;
}

toolchain::MemoryBuffer &
InMemoryModuleCache::addPCM(toolchain::StringRef Filename,
                            std::unique_ptr<toolchain::MemoryBuffer> Buffer) {
  auto Insertion = PCMs.insert(std::make_pair(Filename, std::move(Buffer)));
  assert(Insertion.second && "Already has a PCM");
  return *Insertion.first->second.Buffer;
}

toolchain::MemoryBuffer &
InMemoryModuleCache::addBuiltPCM(toolchain::StringRef Filename,
                                 std::unique_ptr<toolchain::MemoryBuffer> Buffer) {
  auto &PCM = PCMs[Filename];
  assert(!PCM.IsFinal && "Trying to override finalized PCM?");
  assert(!PCM.Buffer && "Trying to override tentative PCM?");
  PCM.Buffer = std::move(Buffer);
  PCM.IsFinal = true;
  return *PCM.Buffer;
}

toolchain::MemoryBuffer *
InMemoryModuleCache::lookupPCM(toolchain::StringRef Filename) const {
  auto I = PCMs.find(Filename);
  if (I == PCMs.end())
    return nullptr;
  return I->second.Buffer.get();
}

bool InMemoryModuleCache::isPCMFinal(toolchain::StringRef Filename) const {
  return getPCMState(Filename) == Final;
}

bool InMemoryModuleCache::shouldBuildPCM(toolchain::StringRef Filename) const {
  return getPCMState(Filename) == ToBuild;
}

bool InMemoryModuleCache::tryToDropPCM(toolchain::StringRef Filename) {
  auto I = PCMs.find(Filename);
  assert(I != PCMs.end() && "PCM to remove is unknown...");

  auto &PCM = I->second;
  assert(PCM.Buffer && "PCM to remove is scheduled to be built...");

  if (PCM.IsFinal)
    return true;

  PCM.Buffer.reset();
  return false;
}

void InMemoryModuleCache::finalizePCM(toolchain::StringRef Filename) {
  auto I = PCMs.find(Filename);
  assert(I != PCMs.end() && "PCM to finalize is unknown...");

  auto &PCM = I->second;
  assert(PCM.Buffer && "Trying to finalize a dropped PCM...");
  PCM.IsFinal = true;
}
