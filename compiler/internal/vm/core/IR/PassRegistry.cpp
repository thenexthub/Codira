//===- PassRegistry.cpp - Pass Registration Implementation ----------------===//
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
// This file implements the PassRegistry, with which passes are registered on
// initialization, and supports the PassManager in dependency resolution.
//
//===----------------------------------------------------------------------===//

#include "vm/core/PassRegistry.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Pass.h"
#include "vm/core/PassInfo.h"
#include <cassert>
#include <memory>

using namespace vm::core;

PassRegistry *PassRegistry::getPassRegistry() {
  static PassRegistry PassRegistryObj;
  return &PassRegistryObj;
}

//===----------------------------------------------------------------------===//
// Accessors
//

PassRegistry::~PassRegistry() = default;

const PassInfo *PassRegistry::getPassInfo(const void *TI) const {
  sys::SmartScopedReader<true> Guard(Lock);
  return PassInfoMap.lookup(TI);
}

const PassInfo *PassRegistry::getPassInfo(StringRef Arg) const {
  sys::SmartScopedReader<true> Guard(Lock);
  return PassInfoStringMap.lookup(Arg);
}

//===----------------------------------------------------------------------===//
// Pass Registration mechanism
//

void PassRegistry::registerPass(const PassInfo &PI, bool ShouldFree) {
  sys::SmartScopedWriter<true> Guard(Lock);
  bool Inserted =
      PassInfoMap.insert(std::make_pair(PI.getTypeInfo(), &PI)).second;
  assert(Inserted && "Pass registered multiple times!");
  (void)Inserted;
  PassInfoStringMap[PI.getPassArgument()] = &PI;

  // Notify any listeners.
  for (auto *Listener : Listeners)
    Listener->passRegistered(&PI);

  if (ShouldFree)
    ToFree.push_back(std::unique_ptr<const PassInfo>(&PI));
}

void PassRegistry::enumerateWith(PassRegistrationListener *L) {
  sys::SmartScopedReader<true> Guard(Lock);
  for (auto PassInfoPair : PassInfoMap)
    L->passEnumerate(PassInfoPair.second);
}

void PassRegistry::addRegistrationListener(PassRegistrationListener *L) {
  sys::SmartScopedWriter<true> Guard(Lock);
  Listeners.push_back(L);
}

void PassRegistry::removeRegistrationListener(PassRegistrationListener *L) {
  sys::SmartScopedWriter<true> Guard(Lock);

  auto I = toolchain::find(Listeners, L);
  Listeners.erase(I);
}
