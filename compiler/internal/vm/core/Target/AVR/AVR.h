//===-- AVR.h - Top-level interface for AVR representation ------*- C++ -*-===//
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
// This file contains the entry points for global functions defined in the LLVM
// AVR back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_AVR_H
#define LLVM_AVR_H

#include "vm/core/CodeGen/SelectionDAGNodes.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {

class AVRTargetMachine;
class FunctionPass;
class PassRegistry;

Pass *createAVRShiftExpandPass();
FunctionPass *createAVRISelDag(AVRTargetMachine &TM, CodeGenOptLevel OptLevel);
FunctionPass *createAVRExpandPseudoPass();
FunctionPass *createAVRFrameAnalyzerPass();
FunctionPass *createAVRBranchSelectionPass();

void initializeAVRAsmPrinterPass(PassRegistry &);
void initializeAVRDAGToDAGISelLegacyPass(PassRegistry &);
void initializeAVRExpandPseudoPass(PassRegistry &);
void initializeAVRShiftExpandPass(PassRegistry &);

/// Contains the AVR backend.
namespace AVR {

/// An integer that identifies all of the supported AVR address spaces.
enum AddressSpace {
  DataMemory,
  ProgramMemory,
  ProgramMemory1,
  ProgramMemory2,
  ProgramMemory3,
  ProgramMemory4,
  ProgramMemory5,
  NumAddrSpaces,
};

/// Checks if a given type is a pointer to program memory.
template <typename T> bool isProgramMemoryAddress(T *V) {
  auto *PT = cast<PointerType>(V->getType());
  assert(PT != nullptr && "unexpected MemSDNode");
  return PT->getAddressSpace() == ProgramMemory ||
         PT->getAddressSpace() == ProgramMemory1 ||
         PT->getAddressSpace() == ProgramMemory2 ||
         PT->getAddressSpace() == ProgramMemory3 ||
         PT->getAddressSpace() == ProgramMemory4 ||
         PT->getAddressSpace() == ProgramMemory5;
}

template <typename T> AddressSpace getAddressSpace(T *V) {
  auto *PT = cast<PointerType>(V->getType());
  assert(PT != nullptr && "unexpected MemSDNode");
  unsigned AS = PT->getAddressSpace();
  if (AS < NumAddrSpaces)
    return static_cast<AddressSpace>(AS);
  return NumAddrSpaces;
}

inline bool isProgramMemoryAccess(MemSDNode const *N) {
  auto *V = N->getMemOperand()->getValue();
  if (V != nullptr && isProgramMemoryAddress(V))
    return true;
  return false;
}

// Get the index of the program memory bank.
//  -1: not program memory
//   0: ordinary program memory
// 1~5: extended program memory
inline int getProgramMemoryBank(MemSDNode const *N) {
  auto *V = N->getMemOperand()->getValue();
  if (V == nullptr || !isProgramMemoryAddress(V))
    return -1;
  AddressSpace AS = getAddressSpace(V);
  assert(ProgramMemory <= AS && AS <= ProgramMemory5);
  return static_cast<int>(AS - ProgramMemory);
}

} // end of namespace AVR

} // end namespace vm::core

#endif // LLVM_AVR_H
