//===-- SystemZConstantPoolValue.cpp - SystemZ constant-pool value --------===//
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

#include "SystemZConstantPoolValue.h"
#include "vm/core/ADT/FoldingSet.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

SystemZConstantPoolValue::SystemZConstantPoolValue(
    const GlobalValue *GV, SystemZCP::SystemZCPModifier Modifier)
    : MachineConstantPoolValue(GV->getType()), GV(GV), Modifier(Modifier) {}

SystemZConstantPoolValue *
SystemZConstantPoolValue::Create(const GlobalValue *GV,
                                 SystemZCP::SystemZCPModifier Modifier) {
  return new SystemZConstantPoolValue(GV, Modifier);
}

int SystemZConstantPoolValue::getExistingMachineCPValue(MachineConstantPool *CP,
                                                        Align Alignment) {
  const std::vector<MachineConstantPoolEntry> &Constants = CP->getConstants();
  for (unsigned I = 0, E = Constants.size(); I != E; ++I) {
    if (Constants[I].isMachineConstantPoolEntry() &&
        Constants[I].getAlign() >= Alignment) {
      auto *ZCPV =
        static_cast<SystemZConstantPoolValue *>(Constants[I].Val.MachineCPVal);
      if (ZCPV->GV == GV && ZCPV->Modifier == Modifier)
        return I;
    }
  }
  return -1;
}

void SystemZConstantPoolValue::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  ID.AddPointer(GV);
  ID.AddInteger(Modifier);
}

void SystemZConstantPoolValue::print(raw_ostream &O) const {
  O << GV << "@" << int(Modifier);
}
