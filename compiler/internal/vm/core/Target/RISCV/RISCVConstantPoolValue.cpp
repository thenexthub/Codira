//===------- RISCVConstantPoolValue.cpp - RISC-V constantpool value -------===//
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
// This file implements the RISC-V specific constantpool value class.
//
//===----------------------------------------------------------------------===//

#include "RISCVConstantPoolValue.h"
#include "vm/core/ADT/FoldingSet.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/Type.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

RISCVConstantPoolValue::RISCVConstantPoolValue(Type *Ty, const GlobalValue *GV)
    : MachineConstantPoolValue(Ty), GV(GV), Kind(RISCVCPKind::GlobalValue) {}

RISCVConstantPoolValue::RISCVConstantPoolValue(LLVMContext &C, StringRef S)
    : MachineConstantPoolValue(Type::getInt64Ty(C)), S(S),
      Kind(RISCVCPKind::ExtSymbol) {}

RISCVConstantPoolValue *RISCVConstantPoolValue::Create(const GlobalValue *GV) {
  return new RISCVConstantPoolValue(GV->getType(), GV);
}

RISCVConstantPoolValue *RISCVConstantPoolValue::Create(LLVMContext &C,
                                                       StringRef S) {
  return new RISCVConstantPoolValue(C, S);
}

int RISCVConstantPoolValue::getExistingMachineCPValue(MachineConstantPool *CP,
                                                      Align Alignment) {
  const std::vector<MachineConstantPoolEntry> &Constants = CP->getConstants();
  for (unsigned i = 0, e = Constants.size(); i != e; ++i) {
    if (Constants[i].isMachineConstantPoolEntry() &&
        Constants[i].getAlign() >= Alignment) {
      auto *CPV =
          static_cast<RISCVConstantPoolValue *>(Constants[i].Val.MachineCPVal);
      if (equals(CPV))
        return i;
    }
  }

  return -1;
}

void RISCVConstantPoolValue::addSelectionDAGCSEId(FoldingSetNodeID &ID) {
  if (isGlobalValue())
    ID.AddPointer(GV);
  else {
    assert(isExtSymbol() && "unrecognized constant pool type");
    ID.AddString(S);
  }
}

void RISCVConstantPoolValue::print(raw_ostream &O) const {
  if (isGlobalValue())
    O << GV->getName();
  else {
    assert(isExtSymbol() && "unrecognized constant pool type");
    O << S;
  }
}

bool RISCVConstantPoolValue::equals(const RISCVConstantPoolValue *A) const {
  if (isGlobalValue() && A->isGlobalValue())
    return GV == A->GV;
  if (isExtSymbol() && A->isExtSymbol())
    return S == A->S;

  return false;
}
