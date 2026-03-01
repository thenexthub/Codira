//===-- lib/CodeGen/GlobalISel/GISelChangeObserver.cpp --------------------===//
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
// This file constains common code to combine machine functions at generic
// level.
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/GlobalISel/GISelChangeObserver.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"

using namespace vm::core;

void GISelChangeObserver::changingAllUsesOfReg(
    const MachineRegisterInfo &MRI, Register Reg) {
  for (auto &ChangingMI : MRI.use_instructions(Reg)) {
    changingInstr(ChangingMI);
    ChangingAllUsesOfReg.insert(&ChangingMI);
  }
}

void GISelChangeObserver::finishedChangingAllUsesOfReg() {
  for (auto *ChangedMI : ChangingAllUsesOfReg)
    changedInstr(*ChangedMI);
  ChangingAllUsesOfReg.clear();
}

RAIIDelegateInstaller::RAIIDelegateInstaller(MachineFunction &MF,
                                             MachineFunction::Delegate *Del)
    : MF(MF), Delegate(Del) {
  // Register this as the delegate for handling insertions and deletions of
  // instructions.
  MF.setDelegate(Del);
}

RAIIDelegateInstaller::~RAIIDelegateInstaller() { MF.resetDelegate(Delegate); }

RAIIMFObserverInstaller::RAIIMFObserverInstaller(MachineFunction &MF,
                                                 GISelChangeObserver &Observer)
    : MF(MF) {
  MF.setObserver(&Observer);
}

RAIIMFObserverInstaller::~RAIIMFObserverInstaller() { MF.setObserver(nullptr); }

RAIITemporaryObserverInstaller::RAIITemporaryObserverInstaller(
    GISelObserverWrapper &Observers, GISelChangeObserver &TemporaryObserver)
    : Observers(Observers), TemporaryObserver(TemporaryObserver) {
  Observers.addObserver(&TemporaryObserver);
}

RAIITemporaryObserverInstaller::~RAIITemporaryObserverInstaller() {
  Observers.removeObserver(&TemporaryObserver);
}
